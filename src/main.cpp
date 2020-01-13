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
 * ***********************************************************************************/

#include <Adafruit_NeoPixel.h>
#include <Button2.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define BODS 7                   //BOD Sleep bit in MCUCR
#define BODSE 2                  //BOD Sleep enable bit in MCUCR
uint8_t mcucr1, mcucr2;

#define RNDPIN      4      // for random generator

#define FOCUS       20
#define DELAY       1000
#define WRAP        1     // wrap color wave
Adafruit_NeoPixel *strip;
int pin = 0;                             // Ring Led input
int numPixels = 12; 
int brightness = 30;
int pixelFormat = NEO_GRB + NEO_KHZ800;

#define BUTTON_A_PIN  2  // multi mode button pin
Button2 *buttonA;
int debounce       = 1;  // total debounce value
int debounce_count = 2;  // init with overreached value
bool intro;              // animation intro flag

int sleeptime  = 10000; // sleep time 10000 ~= 8s 
int sleepcount = 0;     // sleep counter
bool onSuspend = true;  // init with reached condition

uint32_t dice[6][12] = {                   // dice texture numbers
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},   // dice number 1
  { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1},   // dice number 2
  { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},   // dice number 3
  { 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},   // dice number 4
  { 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0},   // dice number 5
  { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}    // dice number 6
};

uint32_t loadRandomColor() {
  return strip->Color(random(brightness), random(brightness), random(brightness));
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<numPixels; i++) {    // For each pixel in strip...
    strip->setPixelColor(i, color);   // Set pixel's color (in RAM)
    strip->show();                    // Update strip to match
    delay(wait);                      // Pause for a moment
  }
}

void loadImage(uint32_t *image, int wait){  // Load complete image on the ring
  for(int i=0; i<numPixels; i++) {    
    strip->setPixelColor(i,image[i]);
    strip->show();
    delay(wait);
  }
}

void loadNumber(uint32_t cbg, uint32_t cnm, int num, int wait) {
  for(int i=0; i<numPixels; i++) {
    if(dice[num][i])                 // select dice mumber and on/off pixel texture
      strip->setPixelColor(i,cnm);   // set number pixel color
    else
      strip->setPixelColor(i,cbg);   // set background pixel color
    strip->show();
    delay(wait);
  }
}

void launchDice() {
  uint32_t cbg = strip->Color(0,0,255); // background color
  uint32_t cnm = strip->Color(255,0,0); // number color
  colorWipe(0,10);                      // animating clead
  delay(100);                           // change time
  colorWipe(cbg,20);                    // paint background
  loadNumber(cbg,cnm,random(6),30);     // load dice number
}

void rainbow(int wait) {                // Adafruit rainbow (see library for explanation)
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<numPixels; i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / numPixels);
      strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixelHue)));
    }
    strip->show();
    delay(wait);
  }
}

void clear() {
  strip->clear();
  strip->show();
}

void OnClickHandler(Button2& btn) {      // onclick handler
  if(!onSuspend) {                       // debounce after sleep
    launchDice();                        // launch dice with normal click
    sleepcount=0;                        // reset sleep counter
  }
}

void OnLongClickHandler(Button2& btn) {           
  if (!onSuspend && debounce_count>debounce) {  // weird anti debounce issue fix
    brightness = brightness + 10;               // increment brightness for ring image
    if (brightness > 30) brightness = 10;       // max value
    strip->setBrightness(brightness);
    strip->show();
    eeprom_write_byte(10,brightness);
    sleepcount = 0;
  } else if (!onSuspend) {
    debounce_count++;
    sleepcount = 0;
  }
}

void OnDoubleClickHandler(Button2& btn) {
  if(!onSuspend)intro=false;
}

void sleep() {
  clear();               // clear ring

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
 
  debounce_count = 0;
  sleepcount = 0;        // reset anim time counter
  onSuspend = true;      // prevent button delay issue
}

ISR(PCINT0_vect) { }     // This is called when the interrupt occurs, but I don't need to do anything in it

void setup() {
  // initialize pseudo-random number generator with some random value
  pinMode(RNDPIN, INPUT);
  randomSeed(analogRead(RNDPIN));
  // initialize brightness
  uint8_t old_brg=eeprom_read_byte(10);
  if(old_brg>0 && old_brg != brightness)brightness=old_brg;
  // initialize LED strip
  strip = new Adafruit_NeoPixel(numPixels, pin, pixelFormat);
  strip->begin();
  strip->setBrightness(brightness);
  strip->show();
  // initialize Button
  buttonA = new Button2(BUTTON_A_PIN);
  buttonA->setClickHandler(OnClickHandler);
  buttonA->setLongClickHandler(OnLongClickHandler);
  buttonA->setDoubleClickHandler(OnDoubleClickHandler);
}

void loop() {
  buttonA->loop();
  if(!intro){            // only show intro one time or with double click
    rainbow(3);
    intro=true;
  }
  if(onSuspend){         // after sleep starting with dice
    launchDice();
    onSuspend=false;
  }
  if (sleepcount++>sleeptime) {
    colorWipe(0,60);     // sleep animation
    sleep();             // go to low power consumption
  }
  strip->show();
}
