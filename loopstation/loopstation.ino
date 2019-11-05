/**
    loopstation.ino
    Purpose: Main code of the LoopStation.

    @author Javier Macías
    @version 0.1 5/11/19
*/

#include "headers/constants.h"
#include "headers/station.h"
#include "headers/request.h"

// const char* cup_id = "d9cff5f3";
const char* bin_id = "b57bdd8e-2bae-4e40-bce6-c6c28f978a8c";
char get_response[20];
char json[100];

Station station;

void setup()
{
  station.setupWifi();
  station.setupRfidReader();
  station.setupProximitySensor();
}

void loop()
{
  char cup_id[2*4 + 1] = "00000000"; // Cup tags are always 8 numbers + null terminator long.
  delay(5000);

  station.readRfidTag(&cup_id[0]);

  station.request.GET(host, cup_id, &get_response[0]);
  Serial.println(get_response);
  bool inside = station.parseJSON(&get_response[0]);
  Serial.println(inside);

  delay(500); // Wait for the user to retire the hand.

  Serial.println(station.safe());

  if(!inside && station.safe()){
    Serial.println("Activate servo");
    Serial.println("POST");
    Serial.println(cup_id);
    station.makeJSON(bin_id, cup_id, &json[0]);
    Serial.println(json);
    station.request.POST(host, &json[0]);
  }
  else{
    Serial.println("Let the user know and don't actuate servo");
  }
}
