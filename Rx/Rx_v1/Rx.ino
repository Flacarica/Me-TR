/*#include <LoRa.h>
#include <SPI.h>
 
#define ss 5
#define rst 14
#define dio0 2
 
void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
 
  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
 
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(12);
  LoRa.setCodingRate4(8);
  Serial.println("LoRa Initializing OK!");
}
 
void loop() 
{
  int packetSize = LoRa.parsePacket();    // try to parse packet
  if (packetSize) 
  {
    
    Serial.print("Received packet '");
 
    while (LoRa.available())              // read packet
    {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }
    Serial.print("' with RSSI ");         // print RSSI of packet
    Serial.println(LoRa.packetRssi());
  }
}*/

#include <LoRa.h>
#include <SPI.h>
 
// LORA MODULE PINS
#define ss 5
#define rst 14
#define dio0 2
 
int test_packetSize = 1;

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
  int packetSize = LoRa.parsePacket();    
  if (packetSize) // try to parse lora data packet
  {
    Serial.print("Received packet ");
 
    while (LoRa.available())              // read packet
    {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }
    Serial.print(" with RSSI ");         // print RSSI of packet
    Serial.println(packetSize);
  }


    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim(); 

        if(input == "CR5") LoRa.setCodingRate4(5);
        if(input == "CR6") LoRa.setCodingRate4(6);
        if(input == "CR7") LoRa.setCodingRate4(7);
        if(input == "CR8") LoRa.setCodingRate4(8);

        if(input == "SF6") LoRa.setSpreadingFactor(6);
        if(input == "SF7") LoRa.setSpreadingFactor(7);
        if(input == "SF8") LoRa.setSpreadingFactor(8);
        if(input == "SF9") LoRa.setSpreadingFactor(9);
        if(input == "SF10") LoRa.setSpreadingFactor(10);
        if(input == "SF11") LoRa.setSpreadingFactor(11);
        if(input == "SF12") LoRa.setSpreadingFactor(12);

        int spaceIndex = input.indexOf(' '); 

        if (spaceIndex > 0) {
            String textPart = input.substring(0, spaceIndex);      // Extract string
            int numberPart = input.substring(spaceIndex + 1).toInt(); // Extract and convert to integer
            if(textPart == "P"){
                test_packetSize = numberPart;
            }
        }
  


    }


}


// temp, presure, hum_air, hum_soil, UV, air_dir, air_speed, rain_gauge, 
// pin 16 en si 17 out??? UV , analog gyml8511
// time?



