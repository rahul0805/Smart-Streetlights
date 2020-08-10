#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include<string.h>

const char *ssid =  "nihar_laptop";     // replace with your wifi ssid and wpa2 key
const char *pass =  "naruto123";
WiFiClient client;
unsigned long myChannelNumber = 1018485;
const char * myWriteAPIKey = "8QG4ZJDO8XP1MKIO";
#define ss 15
#define rst 16
#define dio0 10
byte localAddress = 0xDD; 
byte destination_prev = 0xBB;      // address of Node 3
byte destination_next = 0xAA;       // address of Node 1
byte destination_gateway = 0xDD;   // address of Gateway 
int a,b,c;
void setup() 
{ 
       LoRa.setPins(ss, rst, dio0);
       Serial.begin(115200);
       delay(10000);
       //dht.begin();
       Serial.println("LoRa Receiver");
        while (!LoRa.begin(866E6)) {
        Serial.println("Lora not connected");
         delay(500);
         }
       Serial.println("LoRa Initializing OK!");
       Serial.println("Connecting to ");
       Serial.println(ssid);
       WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) 
      {
            delay(500);
            Serial.println("Wifi not connected");
      }
      Serial.println("");
      Serial.println("WiFi connected");
      ThingSpeak.begin(client);
      ThingSpeak.setField( 1, a);
      ThingSpeak.setField( 2, b);
      ThingSpeak.setField( 3, c);
      ThingSpeak.writeField(myChannelNumber, 1,0, myWriteAPIKey);
      Serial.println("Writing default value of a to thingspeak ( this will take 15 seconds )");
      delay(15000);
}
 
void loop() 
{
                 int packetSize = LoRa.parsePacket();
   if (packetSize == 0) {Serial.println("Free to receive ");return;}                 // if there's no packet, return
  byte recipient_prev    =   LoRa.read();      // recipient address
  byte recipient_next    =   LoRa.read();
  byte recipient_gateway =    LoRa.read();
  byte sender            =    LoRa.read();         // sender address
  byte incomingMsgId     =   LoRa.read();        // incoming msg ID
  byte incomingLength    =    LoRa.read();        // incoming msg length

  String incoming;
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
 int inc_count = incoming.toInt();
  if (incomingLength != incoming.length()) 
  {   
    Serial.println("error: message length does not match length");
    return;                             
  }
  byte recipient;
  if (recipient_gateway != localAddress){
    Serial.println("This message is not for me.");
    return;                             
  }
  else
  {
    Serial.println("this message is for me");
    Serial.println(sender);
    recipient = localAddress;
    if( sender == 0xCC)
    {
      Serial.println("oh yesc");
      a = (int)0;
      b = (int)0;
      c = (int)incomingMsgId;
      Serial.println(c);
      Serial.println("Send to Thingspeak.");
      ThingSpeak.writeField(myChannelNumber, 3,c, myWriteAPIKey);
    }
    if(sender == 0xBB)
    {
      Serial.println("oh yesb");
       a = (int)0;
      b = (int)incomingMsgId;
      c = (int)0;
       Serial.println(b);
       Serial.println("Send to Thingspeak.");
      ThingSpeak.writeField(myChannelNumber, 2,b, myWriteAPIKey);
      }
      if(sender == 0xAA)
      {
        Serial.println("oh yesa");
        a = (int)incomingMsgId;
      b = (int)0;
      c = (int)0;
       Serial.println(a);
       Serial.println("Send to Thingspeak.");
      ThingSpeak.writeField(myChannelNumber, 1,a, myWriteAPIKey);
        }
  }
 
          Serial.println("Writing to thingspeak ( this will take 15 seconds )");
  
  // thingspeak needs minimum 15 sec delay between updates, i've set it to 15 seconds
  delay(15000);
}
