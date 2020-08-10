#include<SoftwareSerial.h>
#define relay_pin 5
#include <LoRa.h>
#include<string.h>
#define sensorTrigPin    3
#define sensorEchoPin    4
 int pulseWidth = 0;  
//int distance;

//SoftwareSerial Serial1(3,4);    //(Rx,Tx)

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin
char str;
String outgoing;              // outgoing message
int prev_count = 0;
int next_count = 0;
int gateway_count = 0;
byte msgCount = 0;                 // count of outgoing messages
byte localAddress = 0xCC;          // address of Node 2
byte destination_prev = 0xBB;      // address of Node 3
byte destination_next = 0xFF;      // address of Node 1
byte destination_gateway = 0xDD;   // address of Gateway 
int timer_2  = 0;
int timer_1 = 0;
int timer = 0;
int car_present = 0;
int count = 0;
int equal_count = 0;
int r = 0;
int dist;
int strength;
int check;
int uart[9];
int i;
boolean x = 0;
int  prev_count_present,prev_count_previous,update_count;
const int HEADER = 0x59;

void setup()
{
Serial.begin(115200);
pinMode(sensorTrigPin, OUTPUT);
pinMode(sensorEchoPin, INPUT);
//Serial1.begin(115200);
//attachInterrupt(digitalPinToInterrupt(2),onReceive,FALLING);
while (!Serial);
Serial.println("LIDAR Sensor and LoRa Duplex");
pinMode(relay_pin,OUTPUT);
TCCR0A = (1<WGM01);
OCR0A = 0xF9;
TIMSK0|=(1<<OCIE0A);
TCCR0B|=(1<<CS01);
TCCR0B|= (1<<CS00);
sei();

digitalWrite(relay_pin,HIGH);
  while (!LoRa.begin(866E6)) 
  {
    Serial.println("Starting LoRa failed!");
  }
   Serial.println("LoRa Duplex");
   delay(100);
   
}
int distance()
{
     digitalWrite(sensorTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(sensorTrigPin, LOW);

    pulseWidth = pulseIn(sensorEchoPin, HIGH);
    dist= pulseWidth*0.034/2;
    delay(20);
         return dist;
}

void count_check()
{
  if(prev_count == count&&equal_count==0)
  {
      equal_count = 1;
      timer_2 = 0;
      car_present = 0;
      Serial.println("lowada ra...");
  }
  
  else if(prev_count != count){
    equal_count = 0;
  
  }
}
void loop()
{
          count_check();
            if(prev_count > count && (prev_count_present != prev_count_previous||update_count==1) )
            {
                
               digitalWrite(relay_pin,LOW);
               update_count = 0;
                Serial.print("prev_count= ");
                Serial.println(prev_count);
                Serial.print("\n");
                timer_1=0;
                car_present = 1;
            }

            if(prev_count_present != prev_count_previous){
            prev_count_previous = prev_count_present;
           
            }
          
           
           if(timer_2>=2 && equal_count==1)
           {
           digitalWrite(relay_pin,HIGH);
           equal_count = 0;
           timer_2 = 0;
           timer_1 = 0;
           Serial.println("Lights OFF - 10 secs done with equal count!! ");
           }
         
           
           if(timer_1>= 10 && car_present == 1)
           {
            
           digitalWrite(relay_pin,HIGH);
           timer_1 = 0;
           car_present = 0;
           Serial.println("Lights OFF - 60 secs done wihout any detection!! ");
           }
      
          
          if(distance()<20)
          {
                  count++;
                  Serial.println("Car is crossing the sensor");
                  Serial.print("count = ");
                  Serial.println(count);
                 
                  while(distance()<20)
                  {
                       onReceive(LoRa.parsePacket());
                      
                     
                  }
                  Serial.println("Car has crossed the sensor");
                  Lora_send_count();
                  update_count = 1;
          }
           
           onReceive(LoRa.parsePacket());
           prev_count_present=prev_count;
           
}
ISR(TIMER0_COMPA_vect)
{
timer++;
if(timer>=1000)
          {
            timer_2 ++;
            timer_1 ++;
            timer = 0;
          }
}
void Lora_send_count()
{
  outgoing = String(count);
  Serial.print("Sending : ");
  Serial.println(outgoing);
  LoRa.beginPacket();
  LoRa.write(destination_prev);         // add destination address previous
  LoRa.write(destination_next);         // add destination address next
  LoRa.write(destination_gateway);      // add destination address gateway  
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        //add payload size
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket(); 
  msgCount++;
  return;
}
void onReceive(int packetSize) {
  if (packetSize == 0) return;                  // if there's no packet, return
  byte recipient_prev    =    LoRa.read();      // recipient address
  byte recipient_next    =    LoRa.read();
  byte recipient_gateway =    LoRa.read();
  byte sender            =    LoRa.read();         // sender address
  byte incomingMsgId     =    LoRa.read();        // incoming msg ID
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

  if (recipient_prev != localAddress && recipient_next != localAddress && recipient_gateway != localAddress){
    Serial.println("This message is not for me.");
    return;                             
  }
  byte recipient;
  if(recipient_prev == localAddress)
  {
  recipient = localAddress;
  next_count = inc_count;
  }
  else if(recipient_next == localAddress)
  {
    recipient = localAddress;
    prev_count = inc_count;
  }
  else if(recipient_gateway == localAddress)
  {
  recipient = localAddress;
  gateway_count = inc_count;
  }
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x"       + String(recipient, HEX));
  Serial.println("Message ID: "      + String(incomingMsgId));
  Serial.println("Message length: "  + String(incomingLength));
  Serial.println("Message: "         + incoming);
  Serial.println("RSSI: "            + String(LoRa.packetRssi()));
  Serial.println("Snr: "            +  String(LoRa.packetSnr()));
  Serial.println();
  return;
}
