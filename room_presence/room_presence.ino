#include "DHT.h"
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define trigPin 4
#define echoPin 5

int calibrationTime = 30;  
int pirPin = 3;
int LDR_Pin = A0;

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte server[] = { 10, 0, 1, 100 };
byte ip[]     = { 10, 0, 1, 75 };

DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length) {
}


EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup() 
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);
  Ethernet.begin(mac, ip);
  dht.begin();
  client.connect("arduinoClient");
  delay(calibrationTime);
}

void loop() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 2.9;
  if (distance < 1100) {           // This value is dependent on position of the sonar module and will need to be changed
    client.publish("inside/bedroom1/sonar", "Occupied");
  }
  else if (distance >= 1100) {
    client.publish("inside/bedroom1/sonar", "Not Occupied");
  }
  
  int LDRReading = analogRead(LDR_Pin);
  String stringL;
  stringL += LDRReading;
  char l[1024];
  stringL.toCharArray(l, 1024);
  client.publish("inside/bedroom1/light", l);
  
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
  
  char stringTF[10];
  dtostrf(f, 3, 1, stringTF);
  char stringH[10];
  dtostrf(h, 3, 1, stringH);
  
  client.publish("inside/bedroom1/temp", stringTF);
  client.publish("inside/bedroom1/humidity", stringH);
  
  if(digitalRead(pirPin) == HIGH){  
     client.publish("inside/bedroom1/motion", "Motion");
       }

   if(digitalRead(pirPin) == LOW){
     client.publish("inside/bedroom1/motion", "No Motion");
           }
           
   delay(500);  
 }
