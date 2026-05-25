#include <BLEDevice.h> 
#include <BLEServer.h> 
#include <BLEUtils.h> 
#include <BLE2902.h> 
#include <LoRa.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>

BLEServer *pServer = NULL; 
BLECharacteristic * pTxCharacteristic; 
bool deviceConnected = false; 
bool oldDeviceConnected = false; 

String txValue;

uint8_t SF=12;
uint8_t CR=8;
uint8_t TxPOW=20;
uint8_t counter = 0;
uint16_t WaterLevel = 0;

Adafruit_BMP085 bmp;
float temperature = 0;
float pressure = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define ss 5
#define rst 14
#define dio0 2
#define RainSen 4
#define R_LED 33
#define G_LED 25
#define B_LED 26
#define SDA_PIN 21
#define SCL_PIN 22

class MyServerCallbacks: public BLEServerCallbacks { 
    void onConnect(BLEServer* pServer) { 
        deviceConnected = true; 
    }; 
    void onDisconnect(BLEServer* pServer) { 
        deviceConnected = false; 
    } 
}; 

class MyCallbacks: public BLECharacteristicCallbacks { 
    void onWrite(BLECharacteristic *pCharacteristic) { 
        String rxString = String(pCharacteristic->getValue().c_str());
        String rxValue;

        if (rxString.length() > 0) { 
            Serial.println("*********"); 
            Serial.print("Received Value: "); 
            Serial.println(rxString);

            if(rxString.indexOf("SF") != -1){ 
                rxValue = rxString.substring(3);
                SF=rxValue.toInt(); 
                LoRa.setSpreadingFactor(SF);
            } else 
            
            if(rxString.indexOf("CR") != -1){ 
                rxValue = rxString.substring(3);
                CR=rxValue.toInt(); 
                LoRa.setCodingRate4(CR);
            } else

            if(rxString.indexOf("TxPOW") != -1){ 
                rxValue=rxString.substring(5);
                TxPOW = rxValue.toInt(); 
                LoRa.setTxPower(TxPOW);
            }

            Serial.println(); 
            Serial.println("*********"); 
        } 
    } 
}; 

void setup() { 
    Serial.begin(115200); 
    Serial.println("MeteoTX started...");

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!bmp.begin()) {
        Serial.println("BMP180 not found! Check wiring.");
        while (1);
    }
    Serial.println("BMP180 initialized OK!");

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
    pinMode(RainSen, INPUT);
    pinMode(R_LED, OUTPUT);
    pinMode(G_LED, OUTPUT);
    pinMode(B_LED, OUTPUT);
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

    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);
    delay(250);

    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, HIGH);
    digitalWrite(B_LED, LOW);
    delay(250);

    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, HIGH);
    delay(250);

    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);
} 

void loop() {

    if (!deviceConnected){
        digitalWrite(R_LED, LOW);
        digitalWrite(G_LED, HIGH);
        digitalWrite(B_LED, LOW);
    } else {
        digitalWrite(R_LED, LOW);
        digitalWrite(G_LED, LOW);
        digitalWrite(B_LED, HIGH);
    }

    WaterLevel = analogRead(RainSen);
    temperature = bmp.readTemperature();
    pressure    = bmp.readPressure() / 100.0;

    uint8_t packet[3];
    packet[0] = (uint8_t) map(WaterLevel, 0, 4095, 0, 255);
    packet[1] = (uint8_t) constrain((int)temperature, 0, 255);
    packet[2] = (uint8_t) constrain((int)pressure - 900, 0, 255);

    Serial.println(packet[0]);
    Serial.println(packet[1]);
    Serial.println(packet[2]);
    Serial.println("--------------------");

    if (deviceConnected) {
        
        digitalWrite(R_LED, LOW);
        digitalWrite(G_LED, LOW);
        digitalWrite(B_LED, HIGH);
        
        pTxCharacteristic->setValue(packet, 3);
        pTxCharacteristic->notify();
    } 
    
    LoRa.beginPacket();
    LoRa.write(packet, 3);
    LoRa.endPacket();

    if (!deviceConnected && oldDeviceConnected) { 
        delay(500); 
        pServer->getAdvertising()->start(); 
        Serial.println("Start advertising"); 
        oldDeviceConnected = deviceConnected; 
    } 
    
    if (deviceConnected && !oldDeviceConnected) { 
        delay(500); 
        pTxCharacteristic->setValue("TX shell online\n"); 
        pTxCharacteristic->notify();
        oldDeviceConnected = deviceConnected; 
    } 
    
    delay(3000);
}