/* 
  Video: https://www.youtube.com/watch?v=oCMOYS71NIU 
  Based on Neil Kolban example for IDF.
  Ported to Arduino ESP32 by Evandro Copercini 
*/ 

#include <BLEDevice.h> 
#include <BLEServer.h> 
#include <BLEUtils.h> 
#include <BLE2902.h> 
#include <LoRa.h>
#include <SPI.h>

BLEServer *pServer = NULL; 
BLECharacteristic * pTxCharacteristic; 
bool deviceConnected = false; 
bool oldDeviceConnected = false; 

// FIXED: Cleanly using Arduino Strings for both to eliminate array size errors
String txValue;
String txPack;

uint8_t SF=12;     //  LoRa spread factor parameter
uint8_t CR=8;      //  LoRa coding rate parameter
uint8_t TxPOW=20;  //  LoRa transmitted power parameter
uint8_t counter = 0;
uint16_t WaterLevel = 0;

// UART service UUIDs
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
#define GND_LED 32

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
    pinMode(GND_LED, OUTPUT);
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
    digitalWrite(GND_LED, LOW);
    delay(1000);

    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, HIGH);
    digitalWrite(B_LED, LOW);
    digitalWrite(GND_LED, LOW);
    delay(1000);

    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, HIGH);
    digitalWrite(GND_LED, LOW);
    delay(1000);
} 

void loop() { 
    // FIXED: Read the physical analog sensor data ONCE at the top of the loop
    WaterLevel = analogRead(RainSen);

    if (deviceConnected) { 
        txValue = "Water level: ";
        // FIXED: Replaced invalid multi-parameter call with stable String argument
        pTxCharacteristic->setValue(txValue); 
        pTxCharacteristic->notify(); 
        
        // Convert the sensor map value to a String to transmit over BLE
        uint16_t mappedLevel = map(WaterLevel, 0, 4095, 0, 255);
        txPack = String(mappedLevel);
        
        // Send actual numeric reading string over BLE
        pTxCharacteristic->setValue(txPack);
        pTxCharacteristic->notify();
    } 
    
    // Always send the raw unmapped sensor data out via LoRa packet
    LoRa.beginPacket();
    LoRa.println(WaterLevel);
    LoRa.endPacket();

    // Disconnecting 
    if (!deviceConnected && oldDeviceConnected) { 
        delay(500); 
        pServer->getAdvertising()->start(); 
        Serial.println("Start advertising"); 
        oldDeviceConnected = deviceConnected; 
    } 
    
    // Connecting 
    if (deviceConnected && !oldDeviceConnected) { 
        delay(500); 
        pTxCharacteristic->setValue("TX shell online\n"); 
        pTxCharacteristic->notify();
        oldDeviceConnected = deviceConnected; 
    } 
    
    delay(3000);
}