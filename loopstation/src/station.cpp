#include <arduino.h>
#include "../headers/station.h"
#include <ArduinoJson.h>

/**
    Parses the JSON that is received from a GET request.

    @param char pointer to the array containing the GET response.
    @return a bool containing the value of the JSON parsed.
*/
bool Station::parseJSON(char* get_response){
  bool inside;
  String line = String(get_response);
  // Ignore data that is not likely to be JSON formatted, so must contain a '{'
  if (line.indexOf('{',0) >= 0){
    const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
    const char* json = line.c_str();
    DynamicJsonDocument jsonBuffer(bufferSize);
    deserializeJson(jsonBuffer, json);
    inside = jsonBuffer["inside"];
    jsonBuffer.clear();
  }
  return inside;
}

/**
    Make the JSON payload for the POST request.

    @param bin_id The identifier of the bin.
           cup_id The identifier of the cup read by RFID.
           json The pointer to the variable that will have the payload.
    @return void.
*/
void Station::makeJSON(const char* bin_id, const char* cup_id, char* json){
  String content;

  Serial.println(cup_id);

  const size_t bufferSize = JSON_OBJECT_SIZE(2);
  DynamicJsonDocument jsonBuffer(bufferSize);
  jsonBuffer["bin_id"] = bin_id;
  jsonBuffer["cup_id"] = cup_id;
  serializeJson(jsonBuffer, content);
  content.toCharArray(json, 100);
  jsonBuffer.clear();
}

/**
    Setups the WiFi

    @param None.
    @return void.
*/
void Station::setupWifi(){
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
    Setups the RFID reader.

    @param None.
    @return void.
*/
void Station::setupRfidReader(){
  SPI.begin();			// Init SPI bus
  tagReader.PCD_Init();		// Init MFRC522
  tagReader.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

/**
    Read the cups RFID tag.

    @param cup_id The pointer to the variable that stores the cup identifier.
    @return void.
*/
void Station::readRfidTag(char* cup_id){
  byte id[4];

  // Look for new tags, and select one if present
  if (!tagReader.PICC_IsNewCardPresent() || !tagReader.PICC_ReadCardSerial()){
    Serial.println("Nothing to read...");
    delay(50);

    // If no tag is present, there is nothing to do.
    return;
  }
  else{
    Serial.println("There's a tag!");
    // Now a card is selected. The UID and SAK is in mfrc522.uid.
    // Dump UID
    Serial.print(F("Card UID:"));
    for (byte i = 0; i < tagReader.uid.size; i++) {
      Serial.print(tagReader.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(tagReader.uid.uidByte[i], HEX);
    	id[i] = tagReader.uid.uidByte[i];
    }

    char* cupPtr = &cup_id[0];
    for (byte i = 0; i < 4; i++){
      snprintf(cupPtr, 3, "%02x", id[i]); //convert a byte to character string, and save 2 characters (+null) to charArr;
      cupPtr += 2; //increment the pointer by two characters in charArr so that next time the null from the previous go is overwritten.
    }

    Serial.println();
    Serial.println(cup_id);
  }
}

/**
    Setups the proximity sensor.

    @param None.
    @return void.
*/
void Station::setupProximitySensor(){
  pinMode(PROXIMITY_PIN, INPUT);
}

/**
    Reads the proximity sensor to decide if it is safe to move the motor.

    @param None.
    @return True if it is safe to move the motor.
*/
bool Station::safe(){
  if(digitalRead(PROXIMITY_PIN) == HIGH){
    return 1;
  }
  else{
    return 0;
  }
}
