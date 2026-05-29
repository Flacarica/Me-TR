#include <BLEDevice.h> 
#include <BLEServer.h> 
#include <BLEUtils.h> 
#include <BLE2902.h> 
#include <LoRa.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h> // BMP180 compatible
#include <stdio.h> // printf
#include "driver/pcnt.h"


// BLUETOOTH VARIABLES
BLEServer *pServer = NULL; 
BLECharacteristic * pTxCharacteristic; 
bool deviceConnected = false; 
bool oldDeviceConnected = false; 

String txValue;

// SENSORS VALUES
uint8_t SF=12;
uint8_t CR=5;
uint8_t TxPOW=20;
uint8_t PacketLength = 7;
uint8_t counter = 0;
uint16_t WaterLevel = 0;
uint32_t UpdatePeriod = 3000;

Adafruit_BMP085 bmp; // pressure and temperature
float temperature = 0;
float pressure = 0;         // -900
float humidity_air = 0;
float humidity_soil = 0;
float UV_index = 0;
float air_dir = 0;          //4b
float air_speed = 0;        // rotations per second
float rain_gauge = 0;       


// PCNT (pulse counter) for wind speed
#define PCNT_UNIT       PCNT_UNIT_0
#define PCNT_H_LIM      32000       // near int16 max, avoid overflow
static int16_t pcnt_overflow_count = 0;
static portMUX_TYPE pcnt_mux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR pcnt_overflow_isr(void *arg) {
    portENTER_CRITICAL_ISR(&pcnt_mux);
    pcnt_overflow_count++;
    pcnt_counter_clear(PCNT_UNIT);  // reset counter and clears the event/interrupt flag
    portEXIT_CRITICAL_ISR(&pcnt_mux);
}


// BLUETOOTH UUID`s
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


// HARDWARE PINS
#define ss 5
#define rst 14
#define dio0 2
#define RainSen 4
#define UV_EN 16  
#define UV_OUT 15
#define SDA_PIN 21
#define SCL_PIN 22
#define HUM_SOIL 34
#define HUM_AIR 33
#define WIND_SPD 17
#define AIR_DIR1 12
#define AIR_DIRP2 13



// testing variables
int testing = 0;
int test_packetSize = 1;
uint8_t packet[68];


// Returns total pulse count (overflows + current counter register)
int32_t pcnt_get_total_count() {
    int16_t count = 0;
    pcnt_get_counter_value(PCNT_UNIT, &count);
    int32_t total;
    portENTER_CRITICAL(&pcnt_mux);
    total = (int32_t)pcnt_overflow_count * PCNT_H_LIM + count;
    portEXIT_CRITICAL(&pcnt_mux);
    return total;
}

// Returns rotations/second measured over the last UpdatePeriod ms
// Call once per loop iteration; tracks its own timing internally
float pcnt_get_rps() {
    static int32_t last_count = 0;
    static uint32_t last_time = 0;

    uint32_t now = millis();
    int32_t  current_count = pcnt_get_total_count();

    float dt = (now - last_time) / 1000.0f;  // seconds since last call
    float rps = 0.0f;
    if (dt > 0.0f) rps = (current_count - last_count) / dt;

    last_count = current_count;
    last_time  = now;
    return rps;
}


void setup_pcnt() {
    pcnt_config_t cfg = {};
    cfg.pulse_gpio_num  = WIND_SPD;
    cfg.ctrl_gpio_num   = PCNT_PIN_NOT_USED;
    cfg.channel         = PCNT_CHANNEL_0;
    cfg.unit            = PCNT_UNIT;
    cfg.pos_mode        = PCNT_COUNT_INC;   // count rising edges
    cfg.neg_mode        = PCNT_COUNT_DIS;   // ignore falling edges
    cfg.lctrl_mode      = PCNT_MODE_KEEP;
    cfg.hctrl_mode      = PCNT_MODE_KEEP;
    cfg.counter_h_lim   = PCNT_H_LIM;
    cfg.counter_l_lim   = 0;
    pcnt_unit_config(&cfg);

    pcnt_set_filter_value(PCNT_UNIT, 100); // glitch filter ~1µs @ 80 MHz
    pcnt_filter_enable(PCNT_UNIT);

    pcnt_event_enable(PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_isr_register(pcnt_overflow_isr, NULL, 0, NULL);
    pcnt_intr_enable(PCNT_UNIT);

    pcnt_counter_pause(PCNT_UNIT);
    pcnt_counter_clear(PCNT_UNIT);
    pcnt_counter_resume(PCNT_UNIT);
}




class MyServerCallbacks: public BLEServerCallbacks { 
    void onConnect(BLEServer* pServer) {deviceConnected = true;} 
    void onDisconnect(BLEServer* pServer) {deviceConnected = false;} 
}; 
class MyCallbacks: public BLECharacteristicCallbacks { 
    void onWrite(BLECharacteristic *pCharacteristic) { 
        String rxString = String(pCharacteristic->getValue().c_str());
        String rxValue;

        if (rxString.length() > 0) { 
            Serial.printf("*********\nReceived Value: %s\n", rxString);

            if(rxString.indexOf("SF") != -1){ 
                rxValue = rxString.substring(3);
                SF=rxValue.toInt(); 
                LoRa.setSpreadingFactor(SF);
            } 
            else if(rxString.indexOf("CR") != -1)
            { 
                rxValue = rxString.substring(3);
                CR=rxValue.toInt(); 
                LoRa.setCodingRate4(CR);
            } 
            else if(rxString.indexOf("TxPOW") != -1){ 
                rxValue=rxString.substring(5);
                TxPOW = rxValue.toInt(); 
                LoRa.setTxPower(TxPOW);
            }
            else if(rxString.indexOf("PacketLength") != -1){ 
                rxValue=rxString.substring(12);
                PacketLength = rxValue.toInt(); 
            }
            else if(rxString.indexOf("UpdatePeriod") != -1){ 
                rxValue=rxString.substring(12);
                UpdatePeriod = rxValue.toInt(); 
            }

            Serial.println("\n*********"); 
        } 
    } 
}; 


void setup() { 
    Serial.begin(115200); 
    Serial.println("MeteoTX started...");


    // setup pressure sensor
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!bmp.begin()) {
        Serial.println("BMP180 not found! Check wiring.");
        while (1);
    }
    Serial.println("BMP180 initialized OK!");


    // setup analog sensors
    digitalWrite(UV_EN, HIGH);


    // setup bluetooth connection
    BLEDevice::init("MeteoTX"); 
    pServer = BLEDevice::createServer(); 
    pServer->setCallbacks(new MyServerCallbacks()); 
    
    BLEService *pService = pServer->createService(SERVICE_UUID); 
    
    pTxCharacteristic = pService->createCharacteristic( 
        CHARACTERISTIC_UUID_TX, 
        BLECharacteristic::PROPERTY_NOTIFY 
    ); 
    pTxCharacteristic->addDescriptor(new BLE2902()); 
    
    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic( 
        CHARACTERISTIC_UUID_RX, 
        BLECharacteristic::PROPERTY_WRITE 
    ); 
    pRxCharacteristic->setCallbacks(new MyCallbacks()); 
    
    pService->start(); 
    pServer->getAdvertising()->start(); 
    Serial.println("BLE advertising started...");


    // setup hardware pins
    pinMode(RainSen, INPUT);
    pinMode(UV_EN, OUTPUT);
    pinMode(UV_OUT, INPUT);

    // setup PCNT for wind speed on WIND_SPD pin
    setup_pcnt();
    Serial.println("PCNT wind speed counter initialized OK!");

    LoRa.setPins(ss, rst, dio0);


    // wait for lora communication
    while (!LoRa.begin(433E6)){
        Serial.println(".");
        delay(500);
    }


    // setup LoRa module
    LoRa.setSyncWord(0xA5);
    LoRa.setTxPower(TxPOW);
    LoRa.setSpreadingFactor(SF);
    LoRa.setCodingRate4(CR);
    Serial.println("LoRa Initializing OK!");


    // prime the RPS baseline (first call establishes t0)
    pcnt_get_rps();

    // make test values if in test mode
    if(testing) for(int i=0;i<64;i++) packet[i] = 128 + i; 
} 
void loop() {

    temperature = bmp.readTemperature();
    pressure    = bmp.readPressure() / 100.0; //-900
    humidity_air = analogRead(HUM_AIR); 
    humidity_soil = analogRead(HUM_SOIL);
    UV_index = analogRead(UV_OUT); 
    air_dir = analogRead(0); 
    air_speed = pcnt_get_rps(); // rotations per second via PCNT on WIND_SPD pin
    WaterLevel = analogRead(RainSen);


    packet[0] = (uint8_t) constrain((int)temperature, 0, 255);
    packet[1] = (uint8_t) constrain((int)pressure - 900, 0, 255);
    packet[2] = (uint8_t) map(humidity_air, 0, 4095, 0, 255);
    packet[3] = (uint8_t) map(humidity_soil, 0, 4095, 0, 255);
    packet[4] = (uint8_t) map(UV_index, 0, 4095, 0, 255);
    packet[5] = (uint8_t) constrain((int)air_speed, 0, 255); // rotations/sec, max 255
    packet[6] = (uint8_t) map(WaterLevel, 0, 4095, 0, 255);


    Serial.printf("\nSent packet: temperature - %dC, pressure - %dhPa, air humidity - %dRh , soil humidity - %dRh, UV - %dmw, air speed - %.2frps, water level - %dg,", 
                        packet[0], packet[1]+900, packet[2], packet[3], packet[4], packet[5], packet[6] );
    
    if(testing){
        Serial.print("\nSent packet: ");
        for(int i=0;i<PacketLength;i++){
            Serial.print(packet[i]);
            Serial.print(", ");
        }
    }



    if (deviceConnected) {
        // send bluetooth packet
        pTxCharacteristic->setValue(packet, 3);
        pTxCharacteristic->notify();
    } 
    

    // send lora packet
    LoRa.beginPacket();
    LoRa.write(packet, PacketLength);
    LoRa.endPacket();


    // send bluetooth connection acknowlage after connecting
    if (deviceConnected && !oldDeviceConnected) { 
        delay(500); 
        pTxCharacteristic->setValue("TX shell online\n"); 
        pTxCharacteristic->notify();
        oldDeviceConnected = deviceConnected; 
    } 
    // if bluetooth connection fail after acknowlage start looking for devices
    if (!deviceConnected && oldDeviceConnected) { 
        delay(500); 
        pServer->getAdvertising()->start(); 
        Serial.println("Start advertising"); 
        oldDeviceConnected = deviceConnected; 
    } 
    
    delay(3000);
}
