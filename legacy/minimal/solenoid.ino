// Minimal code to test the solenoid actuator to open and close the bin.

#include <Arduino.h>

const int solenoidPin = 37; //This is the output pin on the Arduino

void setup(){
  pinMode(solenoidPin, OUTPUT); //Sets that pin as an output
}

void loop(){
  digitalWrite(solenoidPin, HIGH);
	delay(1000);
  digitalWrite(solenoidPin, LOW);
  delay(1000);
}
