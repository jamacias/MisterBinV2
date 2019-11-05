#include <Arduino.h>

const int TriggerPin = 40;
const int EchoPin = 41;

// Raw output, unfiltered with average
// int calculate_distance(int TriggerPin, int EchoPin){
//     long duration, distanceCm;
//
//     digitalWrite(TriggerPin, LOW);  //Setting to LOW for 4us for cleaner reading
//     delayMicroseconds(4);
//     digitalWrite(TriggerPin, HIGH);  //Triggering for 10us
//     delayMicroseconds(10);
//     digitalWrite(TriggerPin, LOW);
//
//     duration = pulseIn(EchoPin, HIGH);  //Measuring the time between pulses in microseconds
//
//     distanceCm = duration * 10 / 292 / 2;   //Converting to cm
//
//     return distanceCm;
// }

float calculate_distance(int TriggerPin, int EchoPin){
    int num = 10;
    long duration, distanceCm;
		float value = 0;

    for(int i = 0; i < num; i++){
        digitalWrite(TriggerPin, LOW);  //Setting to LOW for 4us for cleaner reading
        delayMicroseconds(4);
        digitalWrite(TriggerPin, HIGH);  //Triggering for 10us
        delayMicroseconds(10);
        digitalWrite(TriggerPin, LOW);

        duration = pulseIn(EchoPin, HIGH);  //Measuring the time between pulses in microseconds

        distanceCm = duration * 10 / 292 / 2;   //Converting to cm
				// Serial.println(distanceCm);
        value += distanceCm;
				// Serial.print(value);
    }

		// Serial.println(value);
    float average = value / num;

    return average;
}

void setup() {
    Serial.begin(115200);
    pinMode(TriggerPin, OUTPUT);
    pinMode(EchoPin, INPUT);
}

void loop(){
    float distance = calculate_distance(TriggerPin, EchoPin);
		distance = abs(1 - distance/27)*100;
		Serial.println(distance);

    delay(500);
}
