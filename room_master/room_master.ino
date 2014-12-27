#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEf };
byte server[] = { 10, 0, 1, 100 };
byte ip[]     = { 10, 0, 1, 80 };

int bLed = 2;
int rLed = 3;

char message_buff[1000];

char Str0[] = "b1h";                           // Client ID string
char Str1[] = "i/b1/t";                        // Temperature Publish string
char Str2[] = "i/b1/h";                        // Humidity Publish string
char Str3[] = "i/b1/s";                        // Sonar Publish string
char Str4[] = "i/b1/m";                        // Motion Publish string
char Str5[] = "i/b1/l";                        // Light Publish string

void callback(char* topic, byte* payload, unsigned int length);

String toString(byte* payload, unsigned int length);

EthernetClient ethClient;

PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  pinMode(bLed,OUTPUT);
  pinMode(rLed,OUTPUT);
  Serial.begin(9600);
  Ethernet.begin(mac);
  client.connect(Str0);
  client.subscribe(Str1);
  //client.subscribe(Str2);
  client.subscribe(Str3);
  //client.subscribe(Str4);
  //client.subscribe(Str5);
}

void loop()
{
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length)
{
  int i = 0;
  //Serial.println("Message arrived:  topic: " + String(topic));
  //Serial.println("Length: " + String(length,DEC));
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  if(String(topic) == Str1) {
    digitalWrite(bLed, HIGH);
  }
  else(digitalWrite(bLed, LOW));
  if(String(topic) == Str3) {
    digitalWrite(rLed, HIGH);
  }
  else(digitalWrite(rLed, LOW));
  Serial.print(String(topic));
  Serial.print(" - ");
  Serial.println(msgString);
}
