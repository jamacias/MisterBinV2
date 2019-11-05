#include <Arduino.h>
#include "WiFiEsp.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define MQTT_SOCKET_TIMEOUT 5;
#define MQTT_KEEPALIVE 5;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

const char ssid[] = "Phoenix";            // your network SSID (name)
const char pass[] = "jamacias";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

const char server[] = "52.143.156.181";
const char* mqtt_server = "52.143.156.181";

const char bin_id[] = "b57bdd8e-2bae-4e40-bce6-c6c28f978a8c";

const unsigned long HTTP_TIMEOUT = 1000;
long lastReconnectAttempt = 0;
long lastTime = 0;

#define RST_PIN 5
#define SS_PIN  53

const int TriggerPin = 40;
const int EchoPin = 41;
const int servoPin = 4;               //connection pin of the servo
Servo servoMotor;

const int frontDoorEndstopPin = 11;
const int topDoorEndstop1Pin = 12;
const int topDoorEndstop2Pin = 13;
const int solenoidPin = 37; // MOSFET Gate pin
const int redPin = 8;
const int yellowPin = 9;
const int greenPin = 10;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

byte id[4];

// Initialize the Ethernet client object
WiFiEspClient espClient;
PubSubClient client(espClient);

bool skipResponseHeaders() {
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line
  espClient.setTimeout(HTTP_TIMEOUT);
  bool ok = espClient.find(endOfHeaders);
  if (!ok){ Serial.println("No response or invalid response!"); }
  return ok;
}

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

bool GET_data(const char server[], const char trash_id[]){
  bool inside;

  // if (espClient.connect(server, 3001)) {
  if (espClient.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    // "GET /bin/trash/5e299355-67e5-4705-8c11-e1f6f8e8519a HTTP/1.1"
    char message[200];
    sprintf(message, "GET /bin/trash/%s HTTP/1.1", trash_id);
    Serial.println(message);
    espClient.println(message);
    // espClient.println("GET /bin/trash/" + id + " HTTP/1.1");
    // espClient.println("Host: 79.31.104.186");
    sprintf(message, "Host: %s", server);
    Serial.println(message);
    espClient.println(message);
    // espClient.println("Host: " + server);
    espClient.println("Connection: close");
    espClient.println();
    Serial.println("GET connection closed...");
  }

	// delay(100);
  // if there are incoming bytes available
  // from the server, read them and print them

  inside = 1;
  if(espClient.connected()){
    while (espClient.available() == 0 && status == WL_CONNECTED);
    while (!skipResponseHeaders() && status == WL_CONNECTED);  // Wait until actual content comes in

    while(espClient.available() && status == WL_CONNECTED){
      String line = espClient.readStringUntil('\n');

      // Ignore data that is not likely to be JSON formatted, so must contain a '{'
      if (line.indexOf('{',0) >= 0){

        const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;

        const char* json = line.c_str();
        DynamicJsonDocument jsonBuffer(bufferSize);
        deserializeJson(jsonBuffer, json);
        Serial.println(json);
        inside = jsonBuffer["inside"];

        jsonBuffer.clear();
      }
    }
  }

  espClient.flush();
  espClient.stop();
  return inside;
}

void POST_data(const char server[], const char bin_id[], const char trash_id[]){
  String content;

  const size_t bufferSize = JSON_OBJECT_SIZE(2);
  DynamicJsonDocument jsonBuffer(bufferSize);
  jsonBuffer["bin_id"] = bin_id;
  jsonBuffer["trash_id"] = trash_id;
  serializeJson(jsonBuffer, content);

  Serial.println(content);

  if (espClient.connect(server, 80)){
    Serial.println("Connected to server");
    // Make the HTTP request
    espClient.println("POST /bin/trash HTTP/1.1");
    espClient.println("Connection: Keep-Alive");
    espClient.println("Keep-Alive: timeout=2, max=1");
    char message[200];
    sprintf(message, "Host: %s", server);
    Serial.println(message);
    espClient.println(message);
    espClient.println("Accept: */*");
    espClient.println("Content-Length: " + String(content.length()));
    espClient.println("Content-Type: application/json");
    espClient.println();
    espClient.println(content);
    Serial.println("POST connection keep-alive, but sent...");
  }

  delay(100);

  jsonBuffer.clear();
  espClient.flush();
  espClient.stop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // If a message arrives open the door by triggering the solenoid.
  digitalWrite(solenoidPin, HIGH);
  int frontDoorIsClosed;
  do{
    frontDoorIsClosed = digitalRead(frontDoorEndstopPin); // 0 when pressed.
  } while(!frontDoorIsClosed);
  digitalWrite(solenoidPin, LOW);
}

boolean reconnect() {
  if (client.connect("Bin1")){
    Serial.println("connected");
    client.subscribe("bin_topic");
  }
  Serial.println(client.connected());
  return client.connected();
}

// Raw output, unfiltered with average
int calculate_distance(int TriggerPin, int EchoPin){
    long duration, distanceCm;

    digitalWrite(TriggerPin, LOW);  //Setting to LOW for 4us for cleaner reading
    delayMicroseconds(4);
    digitalWrite(TriggerPin, HIGH);  //Triggering for 10us
    delayMicroseconds(10);
    digitalWrite(TriggerPin, LOW);

    duration = pulseIn(EchoPin, HIGH);  //Measuring the time between pulses in microseconds

    distanceCm = duration * 10 / 292 / 2;   //Converting to cm

    return distanceCm;
}

//ServoRoutine
void throw_cup(){
  delay(500);
  servoMotor.write(80);
  delay(600);
  servoMotor.write(170);
  delay(500);
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

  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  Serial.println();

  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);

  pinMode(servoPin, OUTPUT);
  servoMotor.attach(servoPin);
  servoMotor.write(170); // Homing the servo
  delay(500);

  // Initialize the endstops with pull-up resistors
  pinMode(frontDoorEndstopPin, INPUT_PULLUP);
  pinMode(topDoorEndstop1Pin, INPUT_PULLUP);
  pinMode(topDoorEndstop2Pin, INPUT_PULLUP);

  pinMode(solenoidPin, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  Serial.println();
  Serial.println("Starting connection to server...");


  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(){
  char trash_id[2*4 + 1];

  if (!client.connected()){
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, HIGH);
    delay(100);
    digitalWrite(yellowPin, LOW);
    delay(100);
    Serial.println("Reconnecting...");
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
    digitalWrite(greenPin, HIGH);
    // Client connected
    delay(100);
    client.loop();

    // Look for new cards, and select one if present
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()){
      delay(50);

      int distance = calculate_distance(TriggerPin, EchoPin);
      Serial.println(distance);
      delay(500);

      return;
    }

    // Now a card is selected. The UID and SAK is in mfrc522.uid.
    // Dump UID
    Serial.print(F("Card UID:"));
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    	id[i] = mfrc522.uid.uidByte[i];
    }

    char* trashPtr = &trash_id[0];
    for (byte i = 0; i < 4; i++){
      snprintf(trashPtr, 3, "%02x", id[i]); //convert a byte to character string, and save 2 characters (+null) to charArr;
      trashPtr += 2; //increment the pointer by two characters in charArr so that next time the null from the previous go is overwritten.
    }

    Serial.println();
    Serial.println(trash_id);

    client.disconnect(); // Disconnect MQTT
    bool inside = GET_data(server, trash_id);
    Serial.print("inside: ");
    Serial.println(inside);
    Serial.println(mfrc522.PICC_ReadCardSerial());
    if(inside){
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
      delay(2000);
    }
    if(!inside){
      client.disconnect(); // Disconnect MQTT
      Serial.println("Trash not inside. POST.");
      int topDoorIsClosed1 = digitalRead(topDoorEndstop1Pin); // 0 when pressed.
      int topDoorIsClosed2 = digitalRead(topDoorEndstop2Pin); // 0 when pressed.
      Serial.println(topDoorIsClosed1);
      Serial.println(topDoorIsClosed2);
      while(!topDoorIsClosed1 && !topDoorIsClosed2){
        digitalWrite(greenPin, LOW);
        digitalWrite(yellowPin, HIGH);
        delay(100);
        digitalWrite(yellowPin, LOW);
        delay(100);
        topDoorIsClosed1 = digitalRead(topDoorEndstop1Pin); // 0 when pressed.
        topDoorIsClosed2 = digitalRead(topDoorEndstop2Pin); // 0 when pressed.
      }
      Serial.println("Throwing cup...");
      throw_cup();
      POST_data(server, bin_id, trash_id);
      digitalWrite(greenPin, HIGH);
    }
    digitalWrite(redPin, LOW);
  }
}
