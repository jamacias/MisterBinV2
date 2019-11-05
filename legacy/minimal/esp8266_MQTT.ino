/*
  Minimal code to test the ESP8266 MQTT connections.
  Make sure to use the right server and ids.
*/
#include <Arduino.h>
#include "WiFiEsp.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define MQTT_KEEPALIVE 10;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "Phoenix";            // your network SSID (name)
char pass[] = "jamacias";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "52.143.156.181";
const char* mqtt_server = "52.143.156.181";
// const char* mqtt_server = "broker.mqtt-dashboard.com";

const char bin_id[] = "b57bdd8e-2bae-4e40-bce6-c6c28f978a8c";

const unsigned long HTTP_TIMEOUT = 1000;
long lastReconnectAttempt = 0;

// Initialize the Ethernet client object
WiFiEspClient espClient;
PubSubClient client(espClient);

void printWifiStatus(){
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// void reconnect() {
//   // Loop until we're reconnected
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     // Attempt to connect
//     if (client.connect("Bin1")) {
//       Serial.println("connected");
//       client.subscribe("bin_topic");
//       // client.publish("outTopic", "hello world");
//       // client.subscribe("inTopic");
//       // client.subscribe("outTopic");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
// }

boolean reconnect() {
  if (client.connect("Bin1")){
    Serial.println("connected");
    client.subscribe("bin_topic");
  }
  return client.connected();
}

void setup(){
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(){
  if (!client.connected()){
    long now = millis();
    if (now - lastReconnectAttempt > 5000){
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()){
        lastReconnectAttempt = 0;
      }
    }
  }
  else{
    // Client connected
    delay(100);
    client.loop();
    client.publish("outTopic", "hello world");
  }
}
