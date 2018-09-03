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
  char output [15];
  DateTime now = RTC.now();
  sprintf(output,"echo %04d%02d%02d ",now.year(),now.month(),now.day());
  DigiKeyboard.print(output);
  sprintf(output,"%02d%02d",now.hour(),now.minute());
  DigiKeyboard.print(output);
  sprintf(output,"%02d",RTC.now().second());
  DigiKeyboard.println(output);
  delay(1000);
}

void setup() {
  RTC.begin(DateTime(__DATE__, __TIME__));
  pinMode(blinkPin, OUTPUT);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(1000);
}

void loop(){
  clockLoop();
  blinkLoop();
}
