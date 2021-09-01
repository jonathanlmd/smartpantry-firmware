#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

#define HTTP_DEBUG

const int RX02 = 13;
const int TX02 = 15;
String HOST = "smartpantryuepg.herokuapp.com";
String PATH_DEF = "/pantry/add-item/";
String HOST_NAME = "smartpantryuepg.herokuapp.com/pantry/add-item/";
String USER_ID = "612b0e8787a4d2750f4a3268";
String FINGERPRINT = "39 A9 C4 FE B1 7E 23 9E 2F 4D BB AC E8 D7 A3 4F CE 43 0E 7B";

const char* mqttServer = "mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttUser = "otfxknod";
const char* mqttPassword = "nSuUc1dDLygF";


WiFiManager wifiManager;
WiFiClient wifiClient;
SoftwareSerial serialESP8266(RX02, TX02);
PubSubClient clientMqtt(wifiClient);

void setUpWiFi()
{
  //WiFiManager
  //WiFi.mode(WIFI_AP_STA); // explicitly set mode, esp defaults to STA+AP
  WiFi.setHostname("smartpantry.io");
  //Local intialization. Once its business is done, there is no need to keep it around
  //reset saved settings
  // wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  wifiManager.autoConnect("SMART-PANTRY");

  Serial.println("connected...");
}

void setup()
{
  Serial.begin(115200);
  serialESP8266.begin(115200);
  Serial.println("Starting...");
  setUpWiFi();
  clientMqtt.setServer(mqttServer, mqttPort);
   clientMqtt.setCallback(callback);
  while (!clientMqtt.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (clientMqtt.connect("SmartPantryClient")) {
 
      Serial.println("connected");
      clientMqtt.subscribe("pantry.add-item");
      clientMqtt.publish("pantry.add-item", "Hello");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(clientMqtt.state());
      delay(2000);
 
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
}

void loop()
{

  String barcode = "";

  while (serialESP8266.available())
  {
    barcode = serialESP8266.readString();
    serialESP8266.write("Data received.");
  }

  if (barcode.length() > 0 && WiFi.status() == WL_CONNECTED)
  {
    char topic[] = "pantry.add";
    String message = USER_ID + "/" + barcode;

    clientMqtt.publish(topic, message.c_str() );
   
    String PATH = HOST_NAME + barcode.c_str() + "/" + USER_ID;
    Serial.println(PATH);
    
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(FINGERPRINT.c_str());
    HTTPClient https;
    Serial.println("Begin https req.");
    https.begin(*client, HOST.c_str(), 80, PATH.c_str(), true);
    https.addHeader(Content_Type, "application/json");
    https.addHeader("User-Agent", "ESP8266/NodeMCU 0.9");
    int status = https.GET();
    Serial.print("Status: ");
    Serial.println(status);
    https.end();
    serialESP8266.write("Data sent.");
    Serial.println("Data Sent.");
  }

  delay(1000);
}