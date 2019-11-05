/*
  Minimal code to test the LEDs.
*/
#include <Arduino.h>

const int redPin = 8;
const int yellowPin = 9;
const int greenPin = 10;

void setup() {
  Serial.begin(115200);
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop() {
  digitalWrite(redPin, HIGH);
  delay(100);
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  delay(100);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, HIGH);
  delay(100);
  digitalWrite(greenPin, LOW);

  delay(500);
}
