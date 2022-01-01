#include <SPI.h>
#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define uS_TO_S_FACTOR 1000000ULL   /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) 1h */

//dht
#define DHT22_PIN 13
#define DHTTYPE DHT22
DHT dht(DHT22_PIN, DHTTYPE);

//hdc1080
#include "ClosedCube_HDC1080.h"
ClosedCube_HDC1080 hdc1080;

//wifi constants
const char *SSID = "OBE028947";
const char *PWD = "melin369";

//mqtt constants
const char* mqttServer = "192.168.1.105";
WiFiClient espClient;
PubSubClient client(espClient);

float dht_h;
float dht_t;
float hdc_h;
float hdc_t;

void setup()   {                
   Serial.begin(9600); 
   Wire.begin();
   dht.begin();
   initWiFi();
   Serial.print("RRSI: ");
   Serial.println(WiFi.RSSI());
   hdc1080.begin(0x40);
   //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() {
  dht_h = dht.readHumidity();
  dht_t = dht.readTemperature();
  hdc_h = hdc1080.readHumidity();
  hdc_t = hdc1080.readTemperature();
  //delay(2000);
  Serial.print("DHT humidity");
  Serial.println(dht_h);
  Serial.print("DHT temperature");
  Serial.println(dht_t);
  Serial.print("HDC humidity");
  Serial.println(hdc_h);
  Serial.print("HDC temperature");
  Serial.println(hdc_t);
  //check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) { 
    Serial.println("Reconnecting to WiFi...");
    WiFi.reconnect();
  }
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
   //sending mqtt message
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    initMQTT();
    JSONencoder["dht_temp"] = dht_t;
    JSONencoder["dht_hum"] = dht_h;
    JSONencoder["hdc_temp"] = hdc_t;
    JSONencoder["hdc_hum"] = hdc_h;
    char JSONmessageBuffer[100];
    JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    //Serial.println("Sending message to MQTT topic..");
    //Serial.println(JSONmessageBuffer);
    if (client.publish("outside", JSONmessageBuffer) == true) {
      //Serial.println("Success sending message");
    } else {
      //Serial.println("Error sending message");
    }
    client.loop();
    //esp_deep_sleep_start();
    delay(60000);
}


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);
  //Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print('.');
    delay(1000);
  }
  //Serial.println(WiFi.localIP());
}

void initMQTT() {
  client.setServer(mqttServer, 1883);
  while (!client.connected()) {
    //Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client")) {
 
      //Serial.println("connected");
 
    } else {
 
      //Serial.print("failed with state ");
      //Serial.print(client.state());
      delay(2000);
 
    }
  }
}
