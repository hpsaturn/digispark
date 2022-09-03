/**
 * Dice implementation on Digispark with NeoPixel of 12bits (WS2812)
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
 * 0002 : Testing supend methods for low power consumption
 * 0003 : Changed primitives for Adafruit new implementation
 * 0004 : Added eeprom methods for color and brightness
 * 0005 : Added dice primitives, number textures and animations
 * 0006 : Added dice type selector feature (dice type: 2,3,4,5 and 6)
 * 0007 : Added debug macros over blink led
 * 0008 : Added touch button option from ADC measures
 * 0009 : Fixed Adafruit_NeoPixel library issues on last version
 * ***********************************************************************************/

#include <Adafruit_NeoPixel.h>
#include <Button2.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "debug.h"


#define NUMPIXELS 12
#define PIN 0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define BUTTON_A_PIN  2   // Multi mode button pin
Button2 *buttonA;
int debounce       = 1;   // Total debounce value
int debounce_count = 2;   // init with overreached value
bool intro;               // Animation intro flag
int ref0;                 // reference for ADCButton

#define BODS 7            // BOD Sleep bit in MCUCR
#define BODSE 2           // BOD Sleep enable bit in MCUCR
uint8_t mcucr1, mcucr2;   // sleep mcu vars
int sleep_time  = 1500;   // sleep time 10000 ~= 8s
int sleep_count = 0;      // sleep counter
bool onSuspend = true;    // init with reached condition

#define RNDPIN      2     // for random generator, P4 is analog input 2

uint8_t address_brightness = 0;
uint8_t address_dice_type = 30;
uint8_t dice_type = 6;
uint8_t brightness = 25;  // Loaded from eeprom and changet it via key

uint16_t dice[6][12] = {                   // Dice texture numbers
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},   // number 1
  { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1},   // number 2
  { 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},   // number 3
  { 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},   // number 4
  { 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0},   // number 5
  { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}    // number 6
};


uint32_t getRandomColor() {
  return strip.Color(random(brightness), random(brightness), random(brightness));
}

void loadColor(uint32_t color, int wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void loadImage(uint32_t *image, int wait){  // Load complete image on the ring
  for(uint16_t i=0; i<strip.numPixels(); i++) {    
    strip.setPixelColor(i,image[i]);
    strip.show();
    delay(wait);
  }
}

void loadNumber(uint32_t cbg, uint32_t cnm, int num, int wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    if(dice[num][i]==1)                 // select dice mumber and on/off pixel texture
      strip.setPixelColor(i,cnm);   // set number pixel color
    else
      strip.setPixelColor(i,cbg);   // set background pixel color
    strip.show();
    delay(wait);
  }
}

void launchDice() {
  loadColor(strip.Color(0,0,0),20);                         // animating clear
  loadColor(strip.Color(0,0,255),20);                       // paint background
  loadNumber(strip.Color(0,0,255),strip.Color(255,0,0),random(dice_type),20); // load dice number
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void OnClickHandler(Button2& btn) {      // onclick handler
  if(!onSuspend) {                       // debounce after sleep
    launchDice();                        // launch dice with normal click
    sleep_count=0;                        // reset sleep counter
  }
}

void OnLongClickHandler(Button2& btn) {           
  if (!onSuspend && debounce_count>debounce) {    // weird anti debounce issue fix
    if(++dice_type>6)dice_type=2;                 // dice types: 2 to 6
    loadColor(0,10);                   
    uint32_t cbg = strip.Color(0,0,255);          // background color
    uint32_t cnm = strip.Color(0,255,0);          // number color
    loadColor(cbg,10);
    loadNumber(cbg,cnm,dice_type-1,30);           // load dice number
    eeprom_write_byte(&address_dice_type,dice_type);    // save in eeprom
    sleep_count = 0;
  } else if (!onSuspend) {
    debounce_count++;
    sleep_count = 0;
  }
}

void OnDoubleClickHandler(Button2& btn) {         // Dice type changer
  if(!onSuspend) {                                // avoid onsleep event
    brightness = brightness + 10;                 // increment brightness for ring image
    if (brightness > 30) brightness = 5;         // max value
    strip.setBrightness(brightness);
    strip.show();
    eeprom_write_byte(&address_brightness,brightness); // save in eeprom
    sleep_count = 0;
  }
}

void sleep() {
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
  ADCSRA |= _BV(ADEN);   // Enable ADC
  sei();                 // Enable interrupts
 
  debounce_count = 0;    // for long click issue
  sleep_count = 0;        // reset anim time counter
  onSuspend = true;      // prevent button delay issue
}

int ADCread(byte ADCChannel, int samples) {
  long _value = 0;
  for(int _counter = 0; _counter < samples; _counter ++) {
    _value += analogRead(ADCChannel);
  }
  return _value / samples;
}

ISR(PCINT0_vect) { }     // This is called when the interrupt occurs, but I don't need to do anything in it

void setup() {
  debugSetup();
  // initialize pseudo-random number generator with some random value
  int seed = analogRead(RNDPIN);
  // debugBlink(seed);
  randomSeed(seed);
  // initialize brightness
  uint8_t old_brg=eeprom_read_byte(&address_brightness);
  if(old_brg>0 && old_brg != brightness)brightness=old_brg;
  // initialize dice type
  uint8_t old_type=eeprom_read_byte(&address_dice_type);
  if(old_type>1 && old_type<7 && old_type!=dice_type)dice_type=old_type;
  // initialize LED strip
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();
  // initialize Button
  buttonA = new Button2(BUTTON_A_PIN);
  buttonA->setClickHandler(OnClickHandler);
  buttonA->setLongClickHandler(OnLongClickHandler);
  buttonA->setDoubleClickHandler(OnDoubleClickHandler);
}

void loop() {

  buttonA->loop();                 // check multi event button
  
  if(!intro){                      // only show intro one time or with double click
    debugBlink(3);
    rainbow(3);                    // fast intro animation
    intro=true;                    // animation intro only after reset
  }
  if(onSuspend){                   // after sleep starting with dice
    launchDice();                  // launch 1-6 dice
    onSuspend=false;               // reset suspend flag
  }
  int value0 = ADCread(RNDPIN,50); // reading analog avarage value (50 samples)
  if(value0>60){                   // calculating if touch button was pressed
    debugBlink(1);                 // only in debuging one blink
    launchDice();                  // launch dice
    sleep_count=0;                  // reset sleep timer
  }
  if(sleep_count++>sleep_time) {     // testing if sleep time was reached
    debugBlink(3);                 // only in debuging 3 blinks
    loadColor(0,60);               // sleep animation
    sleep();                       // go to low power consumption
  }
}
