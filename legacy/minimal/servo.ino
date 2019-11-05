/* -- PIN CONNECTIONS FOR THE SERVO --
 *  BROWN   WIRE | MASS
 *  CENTRAL WIRE | 5V
 *  ORANGE  WIRE | DATA (HERE PIN4)
 */

 #include <Arduino.h>
 #include <Servo.h>

 int   servoPin = 4;               //connection pin of the servo
 Servo servoMotor;

 // //ServoRoutine
 void throw_cup(){
   delay(500);
   servoMotor.write(80);
   delay(600);
   servoMotor.write(170);
   delay(500);
 }

 void setup(){
   pinMode(servoPin, OUTPUT);
   servoMotor.attach(servoPin);
   servoMotor.write(170); // Homing the servo
   delay(500);
 }

 void loop(){
   throw_cup();
   delay(5000);
 }
