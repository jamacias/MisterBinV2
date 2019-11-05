/*
  Minimal code to test the endstops.
*/
#include <Arduino.h>

const int frontDoorEndstopPin = 11;
const int topDoorEndstop1Pin = 12;
const int topDoorEndstop2Pin = 13;

void setup() {
  Serial.begin(115200);
  pinMode(frontDoorEndstopPin, INPUT_PULLUP);
  pinMode(topDoorEndstop1Pin, INPUT_PULLUP);
  pinMode(topDoorEndstop2Pin, INPUT_PULLUP);
}

void loop() {
  int frontDoorIsClosed = digitalRead(frontDoorEndstopPin); // 0 when pressed.
  int topDoorIsClosed1 = digitalRead(topDoorEndstop1Pin); // 0 when pressed.
  int topDoorIsClosed2 = digitalRead(topDoorEndstop2Pin); // 0 when pressed.
  Serial.print(frontDoorIsClosed);
  Serial.print(" ");
  Serial.print(topDoorIsClosed1);
  Serial.print(" ");
  Serial.println(topDoorIsClosed2);

  delay(100);
}
