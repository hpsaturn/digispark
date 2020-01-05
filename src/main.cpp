#include <Arduino.h>

int blinkPin = 1;

void blinkLoop(){
  digitalWrite(blinkPin, HIGH);
  delay(100);
  digitalWrite(blinkPin, LOW);
  delay(100);
}

void setup() {
  pinMode(blinkPin, OUTPUT);
}

void loop(){
  blinkLoop();
}
