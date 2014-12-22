// Libraries included
#include "DHT.h"              // Temp/Humidity Sensor Library
#include <UIPEthernet.h>    // Breakout board for ethernet
  // ENC SO -> Arduino pin 12
  // ENC SI -> Arduino pin 11
  // ENC SCK -> Arduino pin 13
  // ENC CS -> Arduino pin 10
  // ENC VCC -> Arduino 3V3 pin
  // ENC GND -> Arduino Gnd pin
//#include <Ethernet.h>
//#include <SPI.h>
#include <PubSubClient.h>     // MQTT Library

// Definitions
#define DHTPIN 2            // DHT Data Pin
#define DHTTYPE DHT22       // DHT Type (11,22,21)
#define trigPin 4           // Sonar Trigger Pin.  Sound out
#define echoPin 5           // Sonar Echo Pin.  Sound in

// Integers
long unsigned int lowIn;         
long unsigned int pause = 5000;       // The amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime;
long previousMillis = 0;              // Defining previousMillis for millis() loop
long previousMillis2 = 0;             // Defining previousMillis2 for millis() loop
long interval = 3000;                 // Time to wait before next DHT read
long interval2 = 1000;                // Time to wait before next loop
int pirPin = 3;                       // PIR Data Pin
int LDR_Pin = A5;                     // Photocell Analog Pin

// Variables
byte mac[]    = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };      // MAC address for Ethernet Shield
byte server[] = { 10, 0, 1, 100 };                           // Static IP for MQTT server. Change as needed.
byte ip[]     = { 10, 0, 1, 75 };                            // Static IP for Ethernet Shield. Change as needed.

DHT dht(DHTPIN, DHTTYPE);                                    // Define DHT values

IPAddress timeServer(10, 0, 1, 1);                           // Assigns the router IP as the Time Server

EthernetClient ethClient;                                    // Ethernet Object for MQTT
PubSubClient client(server, 1883, callback, ethClient);      // MQTT Client Instance clientname (Server,Port,Callback,Client)

// Setup Function

void setup() 
{
  pinMode(trigPin, OUTPUT);            // Set Pins to appropriate types
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);
  Ethernet.begin(mac, ip);             // Initialize Ethernet connection using DHCP
  dht.begin();                         // Initialize DHT sensor
  client.connect("bedroom1Client");    // Connect to MQTT broker as "bedroom1Client"
  Serial.begin(9600);
}

// Loop function

void loop()
{
  atmosphere();        // Run the atmosphere function to get the condition of the room
  presence();          // Run the presence funtion to get status of occupancy
  client.loop();       // Keep the MQTT connection alive
}

// Callback Function for PubSubClient

void callback(char* topic, byte* payload, unsigned int length) {
}

// Atmosphere Function

void atmosphere()
{
  unsigned long currentMillis = millis();                     // Define currentMillis as millis() function
  if(currentMillis - previousMillis > interval) {             // If currentMillis minus previousMillis is greater than the intetval value
    previousMillis = currentMillis;                           // then previousMillis is equal to currentMillis
    float f = dht.readTemperature(true);                      // Read the temperature value in Farenheit and store as a floating value
    float h = dht.readHumidity();                             // Read the humidiry value and store as a floating value
    char stringTF[10];                                        // Create char array variable to hold the temp value with 10 positions
    dtostrf(f, 3, 1, stringTF);                               // Convert floating temp value to a string with 3 values, 2 before the decimal and 1 after
    char stringH[10];                                         // Create char array variable to hold the humidity value with 10 positions
    dtostrf(h, 3, 1, stringH);                                // Convert floating humidity value to a string with 3 valuse, 2 before the decimal and 1 after
    client.publish("inside/bedroom1/temp", stringTF);         // Publish the temp string to the MQTT broker under "inside/bedroom1/temp"
    client.publish("inside/bedroom1/humidity", stringH);      // Publish the humidity string to the MQTT broker under "inside/bedroom1/humidity"
}
}

// Presence Function

void presence()
{
  
  unsigned long currentMillis2 = millis();                      // Define currentMillis as millis() function
  if(currentMillis2 - previousMillis2 > interval2) {            // If currentMillis minus previousMillis is greater than the intetval value
  previousMillis2 = currentMillis2;                             // then previousMillis is equal to currentMillis
  
  // Sonar Module
  long duration, distance;                                      // Long variables for duration and distance
  digitalWrite(trigPin, LOW);                                   // Write Trigger Pin low
  delayMicroseconds(2);                                         // Wait 2 microseconds
  digitalWrite(trigPin, HIGH);                                  // Write Trigger Pin high
  delayMicroseconds(10);                                        // Wait 10 microseconds
  digitalWrite(trigPin, LOW);                                   // Write Trigger Pin low
  duration = pulseIn(echoPin, HIGH);                            // Define duration variable
  distance = (duration/2) / 2.9;                                // Define distance variable and calculate distance in mm
  Serial.print("Sonar - ");
  Serial.println(distance);
  if (distance < 1100) {                                        // Check distance, if less than given value
    client.publish("inside/bedroom1/sonar", "Object");          // publish to "inside/bedroom1/sonar" as "Occupied"
  }
  else if (distance >= 1100) {                                  // Otherwise, if distance is greater than or equal to given value
    client.publish("inside/bedroom1/sonar", "No Object");       // publish to "inside/bedroom1/sonar" as "Not Occupied"
  }
  
  // Photocell/Time Detection Module
  int LDRReading = analogRead(LDR_Pin);              
  char l[4];                                        
  String str;
  str=String(LDRReading);
  str.toCharArray(l,4);
  client.publish("inside/bedroom1/light", l);       
  Serial.print("Light - ");
  Serial.println(analogRead(LDR_Pin));

  // PIR Module
  // ***************** NEED TO COMMENT ********************* //
  if(digitalRead(pirPin) == HIGH){ 
    if(lockLow){                                              
       lockLow = false;                                       
       client.publish("inside/bedroom1/motion", "Motion");  
    }
       takeLowTime = true;                                    
       }

   if(digitalRead(pirPin) == LOW){       
       if(takeLowTime){
        lowIn = millis();                                     
        takeLowTime = false;                                  
        }
       if(!lockLow && millis() - lowIn > pause){              
           lockLow = true;                                    
   client.publish("inside/bedroom1/motion", "No Motion");     
   }
}
  }
}
