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

#define RNDPIN      4      // for random generator

#define FOCUS       20
#define DELAY       1000
#define WRAP        1     // wrap color wave
Adafruit_NeoPixel *strip;
int pin = 0;                             // Ring Led input
int numPixels = 12; 
int brightness = 20;
int pixelFormat = NEO_GRB + NEO_KHZ800;

#define BUTTON_A_PIN  2   // multi mode button
Button2 *buttonA;
int debounce_longclick = 1;
int debounce_count = 2;
bool intro;

void sleep(void);

int sleeptime  = 10000; // loop time 10000 ~= 10s 
int sleepcount = 0;
bool onSuspend = true;


uint32_t four[12] = {   // intro image
  0xFF0000FF,
  0xFF00FF00,
  0xFFFF0000,
  0xFF0000FF,
  0xFF00FFFF,
  0xFFFFFFFF,
  0xFF00FFFF,
  0xFF000FFF,
  0xFF00FFFF,
  0xFF0FFFFF,
  0xFFFFFFFF,
  0xFF0FFFFF
};

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
    strip->setPixelColor(i, color);   //  Set pixel's color (in RAM)
    strip->show();                    //  Update strip to match
    delay(wait);                      //  Pause for a moment
  }
}

void loadImage(uint32_t *image){
  for(int i=0; i<numPixels; i++) {
    strip->setPixelColor(i,image[i]);         //  Set pixel's color (in RAM)
    strip->show();
  }
}

void loadNumber(uint32_t cbg, uint32_t cnm, int num, int wait) {
  for(int i=0; i<numPixels; i++) {
    if(dice[num][i])
      strip->setPixelColor(i,cnm);
    else
      strip->setPixelColor(i,cbg);
    strip->show();
    delay(wait);
  }
}

void launchDice() {
  uint32_t cbg = strip->Color(0,0,255);
  uint32_t cnm = strip->Color(255,0,0);
  colorWipe(0,10);
  delay(100);
  colorWipe(cbg,20);
  loadNumber(cbg,cnm,random(6),30);
}

void OnClickHandler(Button2& btn) {
  if(!onSuspend) {
    launchDice();
    sleepcount=0;
  }
}

void OnLongClickHandler(Button2& btn) {
  if (!onSuspend && debounce_count>debounce_longclick) {
    brightness = brightness + 10;
    if (brightness > 30) brightness = 10;
    strip->setBrightness(brightness);
    strip->show();
    sleepcount = 0;
  } else if (!onSuspend) {
    debounce_count++;
    sleepcount = 0;
  }
}

void OnDoubleClickHandler(Button2& btn) {
  if(!onSuspend)intro=false;
}

void rainbow(int wait) {
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<numPixels; i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / numPixels);
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixelHue)));
    }
    strip->show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void setup() {
  // initialize pseudo-random number generator with some random value
  pinMode(RNDPIN, INPUT);
  randomSeed(analogRead(RNDPIN));
  // initialize LED strip
  strip = new Adafruit_NeoPixel(numPixels, pin, pixelFormat);
  strip->begin();
  strip->setBrightness(20);
  strip->show();
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

void loop() {
  buttonA->loop();
  if(!intro){
    rainbow(3);
    intro=true;
  }
  if(onSuspend){
    launchDice();
    onSuspend=false;
  }
  if (sleepcount++>sleeptime) {
    colorWipe(0,60);
    sleep();
  }
  strip->show();
}
