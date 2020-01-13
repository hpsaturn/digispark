/**
 * Digispark NeoPixel 12bits (WS2812)
 * 
 * Framework: PlatformIO - Arduino 
 * Digispark board: http://bit.ly/2uju1nf
 * WS2812 board   : http://bit.ly/2sFNXAx
 * 
 * Author: @hpsaturn 2019-2020 
 * 
 * Revision:
 * 0000 : Code based from Github user Smartynov http://bit.ly/2sOX1TA
 * 0001 : Added button functions
 * 
 * ***********************************************************************************/


#include <Adafruit_NeoPixel.h>
#include <Button2.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define BODS 7                   //BOD Sleep bit in MCUCR
#define BODSE 2                  //BOD Sleep enable bit in MCUCR
uint8_t mcucr1, mcucr2;

#define RNDPIN      5      // for random generator

#define FOCUS       20
#define DELAY       1000
#define WRAP        1     // wrap color wave

int pin = 0;                             // Ring Led input
int numPixels = 12; 
int brightness = 20;
int pixelFormat = NEO_GRB + NEO_KHZ800;
Adafruit_NeoPixel *strip;

int animduration = 13000; // loop time 13000 ~= 10s 
int animticks = 0;
uint32_t color = 0;

#define BUTTON_A_PIN  2   // multi mode button
Button2 *buttonA;
bool onSuspend;

void sleep(void);

void loadRandomColor() {
  color = strip->Color(random(brightness), random(brightness), random(brightness));
  eeprom_write_dword(0,color);
}

void animRingLoop() {
  for (int i=0; i<numPixels; i++) {
    strip->setPixelColor(i,color);
  }
}

void colorWipe(int wait) {
  for(int i=0; i<numPixels; i++) { // For each pixel in strip...
    strip->setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip->show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void OnClickHandler(Button2& btn) {
  if(!onSuspend) {
    loadRandomColor();
    colorWipe(30);     // turn on/off suspend
    animticks=0;
  }
  else onSuspend=false;             // fix button delay after suspend
}

void OnLongClickHandler(Button2& btn) {
  sleep();
}

void OnDoubleClickHandler(Button2& btn) {
  brightness = brightness + 10;
  if (brightness > 60) brightness = 10;
  strip->setBrightness(brightness);
  strip->show();
  animticks=0;
}

void setup() {
  // initialize pseudo-random number generator with some random value
  randomSeed(analogRead(RNDPIN));

  // initialize LED strip
  strip = new Adafruit_NeoPixel(numPixels, pin, pixelFormat);
  strip->begin();
  strip->show();
  color=eeprom_read_dword(0);
  // initialize Button
  buttonA = new Button2(BUTTON_A_PIN);
  buttonA->setClickHandler(OnClickHandler);
  buttonA->setLongClickHandler(OnLongClickHandler);
  buttonA->setDoubleClickHandler(OnDoubleClickHandler);
}

void clear() {
  strip->clear();
  strip->show();
}

void sleep() {
  clear();               // clear ring
  onSuspend = true;      // prevent button delay issue
  animticks = 0;         // reset anim time counter

  GIMSK |= _BV(PCIE);    // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT2);  // Use PB2 as interrupt pin
  ACSR |= _BV(ACD);      // Disable the analog comparator
  ADCSRA &= ~_BV(ADEN);  // Disable ADC

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();        // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                 // Enable interrupts
  mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  mcucr2 = mcucr1 & ~_BV(BODSE);
  MCUCR = mcucr1;
  MCUCR = mcucr2;
  sleep_cpu();           // sleep
  cli();                 // Disable interrupts
  PCMSK &= ~_BV(PCINT2); // Turn off PB2 as interrupt pin
  sleep_disable();       // Clear SE bit
  sei();                 // Enable interrupts
}

ISR(PCINT0_vect) { }     // This is called when the interrupt occurs, but I don't need to do anything in it

void loop() {
  buttonA->loop();
  if(onSuspend){
    colorWipe(30);
    onSuspend=false;
  }
  if (animticks++>animduration) {
    sleep();
  }
  strip->show();
}
