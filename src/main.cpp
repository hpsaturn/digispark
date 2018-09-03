#include "DigiKeyboard.h"
#include "TinyRTClib.h"

RTC_Millis RTC;
int blinkPin = 1;

void blinkLoop(){
  digitalWrite(blinkPin, HIGH);
  delay(100);
  digitalWrite(blinkPin, LOW);
  delay(100);
}

void clockLoop(){
  DateTime now = RTC.now();
  char output [20];
  sprintf(output,"echo %04d%02d%02d.%02d%02d%02d",now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second());
  DigiKeyboard.println(output);
  delay(1000);
}

void setup() {
  RTC.begin(DateTime(__DATE__, __TIME__));
  pinMode(blinkPin, OUTPUT);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(100);
  digitalWrite(blinkPin, HIGH);
}

void loop(){
  clockLoop();
  blinkLoop();
}
