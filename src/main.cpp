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

#define RNDPIN      5      // for random generator

#define FOCUS       20
#define DELAY       1000
#define WRAP        1     // wrap color wave

int pin = 0;                             // Ring Led input
int numPixels = 12; 
int brightness = 40;
int pixelFormat = NEO_GRB + NEO_KHZ800;
Adafruit_NeoPixel *strip;

// we have 3 color spots (reg, green, blue) oscillating along the strip with different speeds
float spdr, spdg, spdb;
float offset;
int animduration = 1100; // loop time 1100 ~= 10s 
int animticks = 0;

// the real exponent function is too slow, so I created an approximation (only for x < 0)
float myexp(float x) {
  return (1.0/(1.0-(0.634-1.344*x)*x));
}

#define BUTTON_A_PIN  2   // multi mode button
Button2 *buttonA;
bool toggle;
bool onSuspend;

void OnClickHandler(Button2& btn) {
  if(!onSuspend)toggle=!toggle;     // turn on/off suspend
  else onSuspend=false;             // fix button delay after suspend
}

void setup() {
  // initialize pseudo-random number generator with some random value
  randomSeed(analogRead(RNDPIN));

  // assign random speed to each spot
  spdr = 1.0 + random(200) / 100.0;
  spdg = 1.0 + random(200) / 100.0;
  spdb = 1.0 + random(200) / 100.0;

  // set random offset so spots start in random locations
  offset = random(10000) / 100.0;

  // initialize LED strip
  strip = new Adafruit_NeoPixel(numPixels, pin, pixelFormat);
  strip->begin();
  strip->show();

  // initialize Button
  buttonA = new Button2(BUTTON_A_PIN);
  buttonA->setClickHandler(OnClickHandler);
  
  ADCSRA &= ~_BV(ADEN); // Disable ADC, it uses ~320uA
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void animRingLoop() {
// use real time to recalculate position of each color spot
  long ms = millis();
  // scale time to float value
  float m = offset + (float)ms/DELAY;
  // add some non-linearity
  m = m - 42.5*cos(m/552.0) - 6.5*cos(m/142.0);

  // recalculate position of each spot (measured on a scale of 0 to 1)
  float posr = 0.15 + 0.55*sin(m*spdr);
  float posg = 0.5 + 0.65*sin(m*spdg);
  float posb = 0.85 + 0.75*sin(m*spdb);

  // now iterate over each pixel and calculate it's color
  for (int i=0; i<numPixels; i++) {
    // pixel position on a scale from 0.0 to 1.0
    float ppos = (float)i / numPixels;
 
    // distance from this pixel to the center of each color spot
    float dr = ppos-posr;
    float dg = ppos-posg;
    float db = ppos-posb;
#if WRAP
    dr = dr - floor(dr + 0.5);
    dg = dg - floor(dg + 0.5);
    db = db - floor(db + 0.5);
#endif

    // set each color component from 0 to max BRIGHTNESS, according to Gaussian distribution
    strip->setPixelColor(i,
      constrain(brightness*myexp(-FOCUS*dr*dr),0,brightness),
      constrain(brightness*myexp(-FOCUS*dg*dg),0,brightness),
      constrain(brightness*myexp(-FOCUS*db*db),0,brightness)
    );
  }
}

void clear() {
  strip->clear();
  strip->show();
}

void sleep() {
  clear();               // clear ring
  toggle = false;        // ever restore it on sleep
  onSuspend = true;      // prevent button delay issue
  animticks = 0;         // reset anim time counter

  GIMSK |= _BV(PCIE);    // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT2);  // Use PB2 as interrupt pin
  sleep_enable();        // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                 // Enable interrupts
  sleep_cpu();           // sleep
  cli();                 // Disable interrupts
  PCMSK &= ~_BV(PCINT2); // Turn off PB2 as interrupt pin
  sleep_disable();       // Clear SE bit
  sei();                 // Enable interrupts
}

ISR(PCINT0_vect) { }     // This is called when the interrupt occurs, but I don't need to do anything in it

void loop() {
  buttonA->loop();
  if (toggle) sleep();
  else if (animticks++<animduration) animRingLoop();
  else sleep();
  strip->show();
}
