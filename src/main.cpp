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
 * 0006 : Added dice type selector feature
 * ***********************************************************************************/

#include <Adafruit_NeoPixel.h>
#include <Button2.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <ADCTouch.h>

Adafruit_NeoPixel *strip;
int pixelFormat = NEO_GRB + NEO_KHZ800;
int pin = 0;              // Ring Led input
int numPixels = 12;       // leds on hardware
uint8_t brightness = 30;  // After that is loaded from eeprom
                          // and changet it via key
#define BUTTON_A_PIN  2   // Multi mode button pin
Button2 *buttonA;
int debounce       = 1;   // Total debounce value
int debounce_count = 2;   // init with overreached value
bool intro;               // Animation intro flag
int ref0;                 // reference for ADCButton

#define BODS 7             //BOD Sleep bit in MCUCR
#define BODSE 2            //BOD Sleep enable bit in MCUCR
uint8_t mcucr1, mcucr2;    //sleep mcu vars
int sleeptime  = 10000;   // sleep time 10000 ~= 8s
int sleepcount = 0;       // sleep counter
bool onSuspend = true;    // init with reached condition

#define RNDPIN      2     // for random generator, P4 is analog input 2
int dicetype = 6;

uint32_t dice[6][12] = {                   // Dice texture numbers
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},   // number 1
  { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1},   // number 2
  { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},   // number 3
  { 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},   // number 4
  { 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0},   // number 5
  { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}    // number 6
};

#define EEA_BRIGTHNESS 10     // eeprom address for brightness
#define EEA_DICETYPE   20     // eeprom addres for dice type

#define DEBUG 1  // Set to 1 to enable, 0 to disable
 
#if DEBUG
#define DebugPin 1  // Digispark model A onboard LED
#define DebugBlink 75
#define DebugPause 300
#define debugDelay(ms) delay(ms)  // Change if you need a special delay function (e.g. if you use libraries that need regular refresh calls)
 
void _debugBlink(int n) {
  for ( int i = 0 ; i < n ; i++ ) {
    digitalWrite(DebugPin, HIGH);
    debugDelay(DebugBlink);
    digitalWrite(DebugPin, LOW);
    debugDelay(DebugBlink);
  }
  debugDelay(DebugPause);
}
 
void _debugSetup() {
  pinMode(DebugPin, OUTPUT);
}
 
#define debugBlink(n) _debugBlink(n)  // Do the blink when DEBUG is set
#define debugSetup() _debugSetup()
#else
#define debugBlink(n)  // Make the calls disappear when DEBUG is 0
#define debugSetup()
#endif

uint32_t getRandomColor() {
  return strip->Color(random(brightness), random(brightness), random(brightness));
}

void loadColor(uint32_t color, int wait) {
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
  uint32_t cbg = strip->Color(0,0,255);    // background color
  uint32_t cnm = strip->Color(255,0,0);    // number color
  loadColor(0,20);                         // animating clead
  loadColor(cbg,20);                       // paint background
  loadNumber(cbg,cnm,random(dicetype),20); // load dice number
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

void OnClickHandler(Button2& btn) {      // onclick handler
  if(!onSuspend) {                       // debounce after sleep
    launchDice();                        // launch dice with normal click
    sleepcount=0;                        // reset sleep counter
  }
}

void OnLongClickHandler(Button2& btn) {           
  if (!onSuspend && debounce_count>debounce) {    // weird anti debounce issue fix
    if(++dicetype>6)dicetype=2;               // dice types: 2 to 6
    loadColor(0,10);                   
    uint32_t cbg = strip->Color(0,0,255);     // background color
    uint32_t cnm = strip->Color(0,255,0);     // number color
    loadColor(cbg,10);
    loadNumber(cbg,cnm,dicetype-1,30);        // load dice number
    eeprom_write_byte(EEA_DICETYPE,dicetype); // save in eeprom
    sleepcount = 0;
  } else if (!onSuspend) {
    debounce_count++;
    sleepcount = 0;
  }
}

void OnDoubleClickHandler(Button2& btn) {     // Dice type changer
  if(!onSuspend) {                            // avoid onsleep event
    brightness = brightness + 10;                 // increment brightness for ring image
    if (brightness > 30) brightness = 10;         // max value
    strip->setBrightness(brightness);
    strip->show();
    eeprom_write_byte(EEA_BRIGTHNESS,brightness); // save in eeprom
    sleepcount = 0;
  }
}

void sleep() {

  debugBlink(3);

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
  sleepcount = 0;        // reset anim time counter
  onSuspend = true;      // prevent button delay issue
}

ISR(PCINT0_vect) { }     // This is called when the interrupt occurs, but I don't need to do anything in it

void setup() {
  debugSetup();
  debugBlink(1);
  // initialize pseudo-random number generator with some random value
  int seed = analogRead(RNDPIN);
  debugBlink(seed);
  randomSeed(seed);
  // initialize brightness
  uint8_t old_brg=eeprom_read_byte(EEA_BRIGTHNESS);
  if(old_brg>0 && old_brg != brightness)brightness=old_brg;
  // initialize dice type
  uint8_t old_type=eeprom_read_byte(EEA_DICETYPE);
  if(old_type>1 && old_type<7 && old_type!=dicetype)dicetype=old_type;
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
  // initialize Touch Button (ADCButton)
  // ref0 = ADCTouch.read(RNDPIN, 500);
}

void loop() {
  buttonA->loop();       // check multi event button
  // int value0 = ADCTouch.read(RNDPIN);
  // value0 -= ref0;        //remove offset
  // if(value0>100){
    // debugBlink(1);
    // launchDice();
  // }
  if(!intro){            // only show intro one time or with double click
    rainbow(2);          // fast intro animation
    intro=true;
  }
  if(onSuspend){         // after sleep starting with dice
    launchDice();        // launch 1-6 dice
    onSuspend=false;
  }
  if(sleepcount++>sleeptime) {
    loadColor(0,60);     // sleep animation
    sleep();             // go to low power consumption
  }
  strip->show();
}
