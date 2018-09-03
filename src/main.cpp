#include <Arduino.h>
#include "DigiKeyboard.h"
#include <avr/boot.h>
#include <TinyWireM.h>
#include "TinyRTClib.h"

RTC_Millis RTC;
int blinkPin = 1;

byte read_factory_calibration(void) {
  byte SIGRD = 5; // for some reason this isn't defined...
  byte value = boot_signature_byte_get(1);
  return value;
}

boolean is_clock_calibrated(void) {
  return read_factory_calibration() != OSCCAL;
}

void blinkLoop(){
  digitalWrite(blinkPin, HIGH);
  delay(100);
  digitalWrite(blinkPin, LOW);
  delay(100);
}

void clockLoop(){
  DateTime now = RTC.now();
  char output [30];
  sprintf(output,"echo %04d%02d%02d.%02d%02d%02d",now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second());
  DigiKeyboard.println(output);
  delay(1000);
}

void setup() {
  pinMode(blinkPin, OUTPUT);
  digitalWrite(blinkPin, HIGH);
  DigiKeyboard.delay(500);
  digitalWrite(blinkPin, LOW);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(100);
  DigiKeyboard.print("echo is_clock_calibrated => ");
  DigiKeyboard.delay(100);
  char output [10];
  sprintf(output,"%s",is_clock_calibrated() ? "true":"false");
  DigiKeyboard.println(output);
  DigiKeyboard.delay(100);
  RTC.begin(DateTime(__DATE__, __TIME__));
  DigiKeyboard.delay(100);
  digitalWrite(blinkPin, HIGH);
}

void loop(){
  clockLoop();
  blinkLoop();
}
