
#include <LoRa.h>
#include <SPI.h>
 
#define ss 5
#define rst 14
#define dio0 2

#define RainSen 4
 
int counter = 0;
int WaterLevel = 0;

 
void setup() 
{
  Serial.begin(115200); 
  while (!Serial);
  Serial.println("LoRa Sender");

  pinMode(RainSen, INPUT);
 
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
  WaterLevel = map(analogRead(RainSen), 0, 4095, 0, 255);

  Serial.print("Sending packet: ");
  Serial.println(WaterLevel);
 
  LoRa.beginPacket();   //Send LoRa packet to receiver
  LoRa.print(WaterLevel);
  LoRa.endPacket();
 
  delay(3000);
}