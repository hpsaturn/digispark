#include "DigiKeyboard.h"
#include "TinyRTClib.h"

RTC_Millis RTC;
int blinkPin = 1;

void blinkLoop(){
  digitalWrite(blinkPin, HIGH);
  delay(100);
  digitalWrite(blinkPin, LOW);
}

void clockLoop(){
  char output [7];
  DateTime now = RTC.now();
  sprintf(output,"%02d%02d%02d ",now.hour(),now.minute(),now.second());
  DigiKeyboard.print(output);
  delay(1000);
}

void setup() {
  RTC.begin(DateTime(__DATE__, __TIME__));
  pinMode(blinkPin, OUTPUT);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(100);
}

void loop(){
  clockLoop();
  blinkLoop();
}
