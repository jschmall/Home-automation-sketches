// Libraries included
#include "DHT.h"            // Temp/Humidity Sensor Library
#include <SPI.h>            // SPI LIbrary
#include <Ethernet.h>       // Ethernet Shield Library
#include <PubSubClient.h>   // MQTT Library

// Definitions
#define DHTPIN 2            // DHT Data Pin
#define DHTTYPE DHT11       // DHT Type (11,22,21)
#define trigPin 4           // Sonar Trigger Pin.  Sound out
#define echoPin 5           // Sonar Echo Pin.  Sound in

// Integers
int calibrationTime = 30;   // PIR Calibration time
int pirPin = 3;             // PIR Data Pin
int LDR_Pin = A0;            // Sonar Analog Pin

// Variables
byte mac[]    = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };      // MAC address for Ethernet Shield
byte server[] = { 10, 0, 1, 100 };                           // Static IP for MQTT server. Change as needed.
byte ip[]     = { 10, 0, 1, 75 };                            // Static IP for Ethernet Shield. Change as needed.

DHT dht(DHTPIN, DHTTYPE);                                    // Define DHT values

EthernetClient ethClient;                                    //Ethernet Object for MQTT
PubSubClient client(server, 1883, callback, ethClient);      // MQTT Client Instance clientname( Server,Port,Callback,Client)

// Setup Function

void setup() 
{
  pinMode(trigPin, OUTPUT);            // Set Pins to appropriate types
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);
  Ethernet.begin(mac, ip);             // Initialize Ethernet connection
  dht.begin();                         // Initialize DHT sensor
  client.connect("bedroom1Client");    // Connect to MQTT broker as "bedroom1Client"
  delay(calibrationTime);              // Delay for PIR calibration
}

// Loop function

void loop()
{
  atmosphere();        // Run the atmosphere function to get the condition of the room
  presence();          // Run the presence funtion to get status of occupancy
  delay(500);          // Delay loop to account for slow sensors
}

// Callback Function for PubSubClient
void callback(char* topic, byte* payload, unsigned int length) {
}

// Atmosphere Function

void atmosphere()
{
  float f = dht.readTemperature(true);                      // Read the temperature value in Farenheit and store as a floating value
  float h = dht.readHumidity();                             // Read the humidiry value and store as a floating value
  char stringTF[10];                                        // Create char array variable to hold the temp value with 10 positions
  dtostrf(f, 3, 1, stringTF);                               // Convert floating temp value to a string with 3 values, 2 before the decimal and 1 after
  char stringH[10];                                         // Create char array variable to hold the humidity value with 10 positions
  dtostrf(h, 3, 1, stringH);                                // Convert floating humidity value to a string with 3 valuse, 2 before the decimal and 1 after
  client.publish("inside/bedroom1/temp", stringTF);         // Publish the temp string to the MQTT broker under "inside/bedroom1/temp"
  client.publish("inside/bedroom1/humidity", stringH);      // Publish the humidity string to the MQTT broker under "inside/bedroom1/humidity"
}

// Presence Function

void presence()
{
  
  // Sonar Module
  
  long duration, distance;                                      // Long variables for duration and distance
  digitalWrite(trigPin, LOW);                                   // Write Trigger Pin low
  delayMicroseconds(2);                                         // Wait 2 microseconds
  digitalWrite(trigPin, HIGH);                                  // Write Trigger Pin high
  delayMicroseconds(10);                                        // Wait 10 microseconds
  digitalWrite(trigPin, LOW);                                   // Write Trigger Pin low
  duration = pulseIn(echoPin, HIGH);                            // Define duration variable
  distance = (duration/2) / 2.9;                                // Define distance variable and calculate distance in mm
  if (distance < 1100) {                                        // Check distance, if less than given value
    client.publish("inside/bedroom1/sonar", "Occupied");        // publish to "inside/bedroom1/sonar" as "Occupied"
  }
  else if (distance >= 1100) {                                  // Otherwise, if distance is greater than or equal to given value
    client.publish("inside/bedroom1/sonar", "Not Occupied");    // publish to "inside/bedroom1/sonar" as "Not Occupied"
  }
  
  // Photocell Module
  
  int LDRReading = analogRead(LDR_Pin);                         // Define LDRReading integer as reading LDR_Pin
  String stringL;                                               // Create a string
  stringL += LDRReading;                                        // Give the string the value of int LDRReading
  char l[1024];                                                 // Create char l with 1024 positions
  stringL.toCharArray(l, 1024);                                 // Convert the string value to a char array
  client.publish("inside/bedroom1/light", l);                   // Publish to "inside/bedroom1/light" with value of l
  
  // PIR Module
  
  if(digitalRead(pirPin) == HIGH){                              // If PIR Pin is high
     client.publish("inside/bedroom1/motion", "Motion");        // then publish to "inside/bedroom1/motion" as "Motion"
       }

   else {                                                       // Otherwise
   client.publish("inside/bedroom1/motion", "No Motion");       // publish to "inside/bedroom1/motion" as "No Motion"
   }
}
