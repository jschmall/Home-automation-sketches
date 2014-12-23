// Libraries included
#include "DHT.h"              // Temp/Humidity Sensor Library
#include <UIPEthernet.h>      // Library for ethernet breakout board
// ENC SO -> Arduino pin 12
// ENC SI -> Arduino pin 11
// ENC SCK -> Arduino pin 13
// ENC CS -> Arduino pin 10
// ENC VCC -> Arduino 3V3 pin
// ENC GND -> Arduino Gnd pin
//#include <Ethernet.h>
//#include <SPI.h>
#include <PubSubClient.h>     // MQTT Library
#include <MemoryFree.h>

// Definitions
#define DHTPIN 2            // DHT Data Pin
#define DHTTYPE DHT22       // DHT Type (11,22,21)
#define trigPin 4           // Sonar Trigger Pin.  Sound out
#define echoPin 5           // Sonar Echo Pin.  Sound in

// Declarations
long unsigned int lowIn;         
long unsigned int pause = 5000;       // The amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime;
long unsigned previousMillis = 0;              // Defining previousMillis for millis() loop
long unsigned previousMillis2 = 0;             // Defining previousMillis2 for millis() loop
long unsigned previousMillis3 = 0;             // Defining previousMillis3 for millis() loop
long unsigned interval = 3000;                 // Time to wait before next DHT read
long unsigned interval2 = 1000;                // Time to wait before next loop
byte pirPin = 3;                               // PIR Data Pin
byte LDR_Pin = A5;                             // Photocell Analog Pin
char Str0[] = "b1c";                           // Client ID string
char Str1[] = "i/b1/t";                        // Temperature Publish string
char Str2[] = "i/b1/h";                        // Humidity Publish string
char Str3[] = "i/b1/s";                        // Sonar Publish string
char Str4[] = "i/b1/m";                        // Motion Publish string
char Str5[] = "i/b1/l";                        // Light Publish string
char Str6[] = "i/b1/mem";                      // Memory monitoring

// Variables
byte mac[]    = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };      // MAC address for Ethernet Shield
byte server[] = { 10, 0, 1, 100 };                           // Static IP for MQTT server. Change as needed.
byte ip[]     = { 10, 0, 1, 75 };                            // Static IP for Ethernet Shield. Change as needed.

DHT dht(DHTPIN, DHTTYPE);                                    // Define DHT values

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
  client.connect(Str0);    // Connect to MQTT broker as "bedroom1Client"
  //Serial.begin(9600);
}

// Loop function

void loop()
{
  atmosphere();        // Run the atmosphere function to get the condition of the room
  presence();          // Run the presence funtion to get status of occupancy
  memory();            // Run the memory function
  client.loop();       // Keep the MQTT connection alive
}

// Atmosphere Function

// Callback Function for PubSubClient
void callback(char* topic, byte* payload, unsigned int length)
{
}

void atmosphere()
{
  unsigned long currentMillis = millis();                     // Define currentMillis as millis() function
  if(currentMillis - previousMillis > interval) {             // If currentMillis minus previousMillis is greater than the intetval value
    previousMillis = currentMillis;                           // then previousMillis is equal to currentMillis
    float f = dht.readTemperature(true);                      // Read the temperature value in Farenheit and store as a floating value
    float h = dht.readHumidity();                             // Read the humidiry value and store as a floating value
    char sT[5];                                               // Create char array variable to hold the temp value with 10 positions
    dtostrf(f, 3, 1, sT);                                     // Convert floating temp value to a string with 3 values, 2 before the decimal and 1 after
    char sH[5];                                               // Create char array variable to hold the humidity value with 10 positions
    dtostrf(h, 3, 1, sH);                                     // Convert floating humidity value to a string with 3 valuse, 2 before the decimal and 1 after
    client.publish(Str1, sT);                                 // Publish the temp string to the MQTT broker under "inside/bedroom1/temp"
    client.publish(Str2, sH);                                 // Publish the humidity string to the MQTT broker under "inside/bedroom1/humidity"
  }
}

// Presence Function

void presence()
{

  unsigned long currentMillis2 = millis();                      // Define currentMillis as millis() function
  if(currentMillis2 - previousMillis2 > interval2) {            // If currentMillis minus previousMillis is greater than the intetval value
    previousMillis2 = currentMillis2;                           // then previousMillis is equal to currentMillis

    // Sonar Module
    long duration, distance;                                      // Long variables for duration and distance
    digitalWrite(trigPin, LOW);                                   // Write Trigger Pin low
    delayMicroseconds(2);                                         // Wait 2 microseconds
    digitalWrite(trigPin, HIGH);                                  // Write Trigger Pin high
    delayMicroseconds(10);                                        // Wait 10 microseconds
    digitalWrite(trigPin, LOW);                                   // Write Trigger Pin low
    duration = pulseIn(echoPin, HIGH);                            // Define duration variable
    distance = (duration/2) / 2.9;                                // Define distance variable and calculate distance in mm
    //Serial.print("Sonar - ");
    //Serial.println(distance);
    if (distance < 1100) {                                        // Check distance, if less than given value
      client.publish(Str3, "1");                                  // publish to "inside/bedroom1/sonar" as "Occupied"
    }
    else if (distance >= 1100) {                                  // Otherwise, if distance is greater than or equal to given value
      client.publish(Str3, "0");                                  // publish to "inside/bedroom1/sonar" as "Not Occupied"
    }

    // Photocell Module
    int LDRReading = analogRead(LDR_Pin);              
    char l[4];                                        
    String lS;
    lS=String(LDRReading);
    lS.toCharArray(l,4);
    client.publish(Str5, l);       
    //Serial.print("Light - ");
    //Serial.println(analogRead(LDR_Pin));

    // PIR Module
    // ***************** NEED TO COMMENT ********************* //
    if(digitalRead(pirPin) == HIGH){ 
      if(lockLow){                                              
        lockLow = false;                                       
        client.publish(Str4, "1");  
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
        client.publish(Str4, "0");     
      }
    }
  }
}

void memory()
{
  unsigned long currentMillis3 = millis();                      // Define currentMillis as millis() function
  if(currentMillis3 - previousMillis3 > interval2) {            // If currentMillis minus previousMillis is greater than the intetval value
    previousMillis3 = currentMillis3;
  int fMem = freeMemory();
  char m[5];                                        
  String mem;
  mem=String(fMem);
  mem.toCharArray(m,5);
  client.publish(Str6, m);
  }
}
