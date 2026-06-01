#include <BLEDevice.h> 
#include <BLEServer.h> 
#include <BLEUtils.h> 
#include <BLE2902.h> 
#include <LoRa.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <stdio.h>
#include "driver/pcnt.h"


BLEServer *pServer = NULL; 
BLECharacteristic * pTxCharacteristic; 
bool deviceConnected = false; 
bool oldDeviceConnected = false; 

String txValue;

uint8_t SF=12;
uint8_t CR=5;
uint8_t TxPOW=20;
uint8_t PacketLength = 8;
uint8_t counter = 0;
uint16_t WaterLevel = 0;
uint32_t UpdatePeriod = 3000;

Adafruit_BMP085 bmp;
float temperature = 0;
float pressure = 0;
float humidity_air = 0;
float humidity_soil = 0;
float UV_index = 0;
float air_dir = 0;
float air_speed = 0;
float rain_gauge = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define ss 5
#define rst 14
#define dio0 2
#define RainSen 4
#define UV_EN 16  
#define UV_OUT 15
#define R_LED 33
#define G_LED 25
#define B_LED 26
#define SDA_PIN 21
#define SCL_PIN 22
#define HUM_SOIL 14
#define HUM_AIR 27
#define AIR_SPD 17
#define AIR_DIR1 12
#define AIR_DIRP2 13

#define PCNT_UNIT       PCNT_UNIT_0
#define PCNT_CHANNEL    PCNT_CHANNEL_0
#define PCNT_FILTER_NS  1000

int testing = 0;
int test_packetSize = 1;
uint8_t packet[68];

portMUX_TYPE sensorMux = portMUX_INITIALIZER_UNLOCKED;

volatile float shared_air_speed = 0;
volatile float shared_air_dir   = 0;

TaskHandle_t Task_WindHandle   = NULL;
TaskHandle_t Task_MainHandle   = NULL;


void pcnt_init_airspeed() {
    pcnt_config_t cfg = {};
    cfg.pulse_gpio_num = AIR_SPD;
    cfg.ctrl_gpio_num  = PCNT_PIN_NOT_USED;
    cfg.channel        = PCNT_CHANNEL;
    cfg.unit           = PCNT_UNIT;
    cfg.pos_mode       = PCNT_COUNT_INC;
    cfg.neg_mode       = PCNT_COUNT_DIS;
    cfg.lctrl_mode     = PCNT_MODE_KEEP;
    cfg.hctrl_mode     = PCNT_MODE_KEEP;
    cfg.counter_h_lim  = 32767;
    cfg.counter_l_lim  = 0;

    pcnt_unit_config(&cfg);

    pcnt_set_filter_value(PCNT_UNIT, (uint16_t)(PCNT_FILTER_NS / 12.5));
    pcnt_filter_enable(PCNT_UNIT);

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
            else if(rxString.indexOf("CR") != -1){ 
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


// --- Core 0 Task: wind sensors ---
void Task_Wind(void *pvParameters) {
    pcnt_init_airspeed();
    Serial.println("PCNT air speed counter initialized!");

    for (;;) {
        int16_t pcnt_count = 0;
        pcnt_get_counter_value(PCNT_UNIT, &pcnt_count);
        pcnt_counter_clear(PCNT_UNIT);

        float local_speed = (float)pcnt_count / (UpdatePeriod / 1000.0f);
        float local_dir   = analogRead(AIR_DIR1);

        portENTER_CRITICAL(&sensorMux);
        shared_air_speed = local_speed;
        shared_air_dir   = local_dir;
        portEXIT_CRITICAL(&sensorMux);

        vTaskDelay(pdMS_TO_TICKS(UpdatePeriod));
    }
}


// --- Core 1 Task: all other sensors, BLE, LoRa ---
void Task_Main(void *pvParameters) {
    Serial.begin(115200); 
    Serial.println("MeteoTX started...");

    Wire.begin(SDA_PIN, SCL_PIN);
    if (!bmp.begin()) {
        Serial.println("BMP180 not found! Check wiring.");
        while (1);
    }
    Serial.println("BMP180 initialized OK!");

    pinMode(RainSen, INPUT);
    pinMode(R_LED, OUTPUT);
    pinMode(G_LED, OUTPUT);
    pinMode(B_LED, OUTPUT);
    pinMode(UV_EN, OUTPUT);
    pinMode(UV_OUT, INPUT);
    digitalWrite(UV_EN, HIGH);

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

    LoRa.setPins(ss, rst, dio0);

    while (!LoRa.begin(433E6)){
        Serial.println(".");
        delay(500);
    }

    LoRa.setSyncWord(0xA5);
    LoRa.setTxPower(TxPOW);
    LoRa.setSpreadingFactor(SF);
    LoRa.setCodingRate4(CR);
    Serial.println("LoRa Initializing OK!");

    digitalWrite(R_LED, HIGH); digitalWrite(G_LED, LOW); digitalWrite(B_LED, LOW);
    delay(250);
    digitalWrite(R_LED, LOW); digitalWrite(G_LED, HIGH); digitalWrite(B_LED, LOW);
    delay(250);
    digitalWrite(R_LED, LOW); digitalWrite(G_LED, LOW); digitalWrite(B_LED, HIGH);
    delay(250);
    digitalWrite(R_LED, LOW); digitalWrite(G_LED, LOW); digitalWrite(B_LED, LOW);

    if(testing) for(int i=0;i<64;i++) packet[i] = 128 + i;

    for (;;) {
        if (!deviceConnected){ digitalWrite(R_LED, LOW); digitalWrite(G_LED, HIGH); digitalWrite(B_LED, LOW); }
        else { digitalWrite(R_LED, LOW); digitalWrite(G_LED, LOW); digitalWrite(B_LED, HIGH); }

        portENTER_CRITICAL(&sensorMux);
        air_speed = shared_air_speed;
        air_dir   = shared_air_dir;
        portEXIT_CRITICAL(&sensorMux);

        temperature  = bmp.readTemperature();
        pressure     = bmp.readPressure() / 100.0;
        humidity_air = analogRead(HUM_AIR); 
        humidity_soil= analogRead(HUM_SOIL);
        UV_index     = analogRead(UV_OUT); 
        WaterLevel   = analogRead(RainSen);

        packet[0] = (uint8_t) constrain((int)temperature, 0, 255);
        packet[1] = (uint8_t) constrain((int)pressure - 900, 0, 255);
        packet[2] = (uint8_t) map(humidity_air,  0, 4095, 0, 255);
        packet[3] = (uint8_t) map(humidity_soil, 0, 4095, 0, 255);
        packet[4] = (uint8_t) map(UV_index,      0, 4095, 0, 255);
        packet[5] = (uint8_t) map((long)air_dir, 0, 4095, 0, 255);
        packet[6] = (uint8_t) constrain((int)air_speed, 0, 255);
        packet[7] = (uint8_t) WaterLevel;//map(WaterLevel,    0, 4095, 0, 255);

        Serial.printf("\nSent packet: temperature - %dC, pressure - %dhPa, air humidity - %dRh, soil humidity - %dRh, UV - %d, air direction - %d, air speed - %.1f counts/s, water level - %d",
                            packet[0], (int)pressure, packet[2], packet[3], packet[4], packet[5], air_speed, packet[7]);

        if(testing){
            Serial.print("\nSent packet: ");
            for(int i=0;i<PacketLength;i++){
                Serial.print(packet[i]);
                Serial.print(", ");
            }
        }

        if (deviceConnected) {
            digitalWrite(R_LED, LOW); digitalWrite(G_LED, LOW); digitalWrite(B_LED, HIGH);
            pTxCharacteristic->setValue(packet, PacketLength);
            pTxCharacteristic->notify();
        } 
        else { digitalWrite(R_LED, LOW); digitalWrite(G_LED, HIGH); digitalWrite(B_LED, LOW); }

        LoRa.beginPacket();
        LoRa.write(packet, PacketLength);
        LoRa.endPacket();

        if (deviceConnected && !oldDeviceConnected) { 
            delay(500); 
            pTxCharacteristic->setValue("TX shell online\n"); 
            pTxCharacteristic->notify();
            oldDeviceConnected = deviceConnected; 
        } 
        if (!deviceConnected && oldDeviceConnected) { 
            delay(500); 
            pServer->getAdvertising()->start(); 
            Serial.println("Start advertising"); 
            oldDeviceConnected = deviceConnected; 
        } 

        vTaskDelay(pdMS_TO_TICKS(UpdatePeriod));
    }
}


void setup() {
    xTaskCreatePinnedToCore(
        Task_Wind,
        "Task_Wind",
        4096,
        NULL,
        1,
        &Task_WindHandle,
        0
    );

    xTaskCreatePinnedToCore(
        Task_Main,
        "Task_Main",
        8192,
        NULL,
        1,
        &Task_MainHandle,
        1
    );
}

void loop() {
    // intentionally empty — logic runs in FreeRTOS tasks
}
