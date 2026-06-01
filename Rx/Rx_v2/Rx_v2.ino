#include <LoRa.h>
#include <SPI.h>
 
// LORA MODULE PINS
#define ss 5
#define rst 14
#define dio0 2

int testing = 0;
int test_packetSize = 1;
int test_packetSize;
String LoRaData;

void setup() 
{
  // start and wait for serial monitor
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("LoRa Receiver");
 
  // start LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);    
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }

  // setup LoRa module
  LoRa.setSyncWord(0xA5);
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(12);
  LoRa.setCodingRate4(5);
  Serial.println("LoRa Initializing OK!");
}
 

void loop() 
{

    // try to parse lora data packet
    test_packetSize = LoRa.parsePacket();    
    if (test_packetSize) 
    {
        while (LoRa.available()){
        LoRaData = LoRa.readString();
        Serial.printf("\nrecived packets: %d   RSSI: %d   DATA: %s", test_packetSize, LoRa.packetRssi(), LoRaData);
        }
    }


    // send commands via serial monitor
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim(); 

        int spaceIndex = input.indexOf(' '); 

        if (spaceIndex > 0) {
            String textPart = input.substring(0, spaceIndex);      // Extract string
            int numberPart = input.substring(spaceIndex + 1).toInt(); // Extract and convert to integer
            if(textPart == "P") test_packetSize = numberPart;
            if(textPart == "SF") LoRa.setSpreadingFactor(numberPart); // 7-12, pentru SF 6 trebuie specificata marimea pachetului
            if(textPart == "CR") LoRa.setCodingRate4(numberPart); // 5-8
            
        }
    }
}




