/*
                 Arduino Nano V3
                     _______
  RX             D0 -|  N  |- 
  TX             D1 -|  A  |-             GND
  RESET             -|  N  |-             RESET
  GND               -|  O  |-   	      5V
  receive        D2 -|     |- A7
                 D3 -|  V  |- A6      
  transmit       D4 -|  3  |- A5          SCL
                 D5 -|     |- A4          SDA
                 D6 -|     |- A3      
                 D7 -|     |- A2      
                 D8 -|     |- A1      
  DHT11          D9 -|     |- A0      			 
                D10 -|     |-             AREF
  MOSI          D11 -|     |-             3.3V			
  MISO          D12 -|_____|- D13         LED/SCK
v 0.1    Read DHT11 and send

 * Generic Sender code : Send a value (counter) over RF 433.92 mhz
 * Fréquence : 433.92 mhz
 * Protocole : homepi 
 * Licence : CC -by -sa
 * Auteur : Yves Grange
 * Version : 0.1
 * Lase update : 10/10/2014
 * https://github.com/Yves911/generic_433_sender
 * 
 * Based on: Valentin CARRUESCO aka idleman and Manuel Esteban aka Yaug (http://manuel-esteban.com) work  
 */

// Includes
#include <dht.h> // http://playground.arduino.cc/Main/DHTLib#.UyMXevldWCQ

// Define vars
#define DHT11_PIN 9
#define senderPin 4

long codeKit = 1000;  // Your unique ID for your Arduino node
int Bytes[30]; 
int BytesData[30]; 

// Start includes
dht DHT;

void itob(unsigned long integer, int length)
{  
 for (int i=0; i<length; i++){
   if ((integer / power2(length-1-i))==1){
     integer-=power2(length-1-i);
     Bytes[i]=1;
   }
   else Bytes[i]=0;
 }
}

void itobCounter(unsigned long integer, int length)
{  
 for (int i=0; i<length; i++){
   if ((integer / power2(length-1-i))==1){
     integer-=power2(length-1-i);
     BytesData[i]=1;
   }
   else BytesData[i]=0;
 }
}

unsigned long power2(int power){    //gives 2 to the (power)
 unsigned long integer=1;          
 for (int i=0; i<power; i++){      
   integer*=2;
 }
 return integer;
}

/**
 * Crée notre signal sous forme binaire
**/
void buildSignal()
{
  Serial.println(codeKit);
  // Converti les codes respectifs pour le signal en binaire
  itob(codeKit, 14);
  for(int j=0;j < 14; j++){
   Serial.print(Bytes[j]);
  }
  Serial.println();
}

// Convert 0 in 01 and 1 in 10 (Manchester conversion)
void sendPair(bool b) {
 if(b)
 {
   sendBit(true);
   sendBit(false);
 }
 else
 {
   sendBit(false);
   sendBit(true);
 }
}

//Envois d'une pulsation (passage de l'etat haut a l'etat bas)
//1 = 310µs haut puis 1340µs bas
//0 = 310µs haut puis 310µs bas
void sendBit(bool b) {
 if (b) {
   digitalWrite(senderPin, HIGH);
   delayMicroseconds(650);   //506 orinally, but tweaked.
   digitalWrite(senderPin, LOW);
   delayMicroseconds(2024);  //1225 orinally, but tweaked.
 }
 else {
   digitalWrite(senderPin, HIGH); 
   delayMicroseconds(650);   //506 orinally, but tweaked.
   digitalWrite(senderPin, LOW);
   delayMicroseconds(4301);   //305 orinally, but tweaked.
 }
}

/** 
 * Transmit data
 * @param boolean  positive : if the value you send is a positive or negative one
 * @param long Counter : the value you want to send
 **/
void transmit(boolean positive, unsigned long Counter, int BytesType[], int repeats)
{
 int ii;
 for(ii=0; ii<repeats;ii++)
 {
  int i;
  itobCounter(Counter, 30);

  // Send the unique ID of your Arduino node
  for(i=0; i<14;i++)
 {
  sendPair(Bytes[i]);
 }

  // Send protocol type
 for(int j = 0; j<4; j++)
 {
  sendPair(BytesType[j]);
 }

 // Send the flag to mark the value as positive or negative
 sendPair(positive);

 // Send value (ie your counter)
 for(int j = 0; j<30; j++)
 {
   sendPair(BytesData[j]);
 }

 // Send the flag "End of the transmission"
 digitalWrite(senderPin, HIGH);
 delayMicroseconds(650);     
 digitalWrite(senderPin, LOW);
 delayMicroseconds(8602);
 }
}





void transmitOLD(boolean positive, unsigned long Counter, int BytesType[])
{
 int i;
 itobCounter(Counter, 30);

 // Send the unique ID of your Arduino node
 for(i=0; i<14;i++)
 {
   sendPair(Bytes[i]);
 }

 // Send protocol type
 for(int j = 0; j<4; j++)
 {
   sendPair(BytesType[j]);
 }

 // Send the flag to mark the value as positive or negative
 sendPair(positive);

 // Send value (ie your counter)
 for(int j = 0; j<30; j++)
 {
   sendPair(BytesData[j]);
 }

 // Send the flag "End of the transmission"
 digitalWrite(senderPin, HIGH);
 delayMicroseconds(650);     
 digitalWrite(senderPin, LOW);
 delayMicroseconds(8602);

}






 void setup()
{
  pinMode(senderPin, OUTPUT);
  buildSignal();
}

void loop()
{
 // Read DHT11 and transmit value as sensor 1
 int chk = DHT.read11(DHT11_PIN);
 switch (chk)
 {
  case DHTLIB_OK:
   float humfloat = DHT.humidity;
   int CounterValue = humfloat * 10;
   int BytesType[] = {0,0,0,1}; // type 1
   transmit(true, CounterValue, BytesType, 6); // transmit
   break;
 }
 delay(60000); // delay 60s
} 

