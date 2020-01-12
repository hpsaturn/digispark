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

#define RNDPIN      5      // for random generator

#define FOCUS       20
#define DELAY       1000
#define WRAP        1     // wrap color wave

int pin = 0;                             // Ring Led input
int numPixels = 12; 
int brightness = 20;
int pixelFormat = NEO_GRB + NEO_KHZ800;
Adafruit_NeoPixel *strip;

uint32_t current_color = 0;

#define BUTTON_A_PIN  2   // multi mode button
Button2 *buttonA;

int sleepCounter;
bool onSuspend;
bool onTask;
bool intro=false;

void loadRandomColor() {
  current_color = strip->Color(random(brightness), random(brightness), random(brightness));
  eeprom_write_dword(0,current_color);
}

void OnClickHandler(Button2& btn) {
  if (!onSuspend && !onTask) onTask = true;     // enable task
  else {
    onSuspend = false;
    delay(100);
  }
}

void OnLongClickHandler(Button2& btn) {
  loadRandomColor();
  if (!onTask) onTask = true;
}

void OnDoubleClickHandler(Button2& btn) {
  brightness = brightness + 10;
  if (brightness > 60) brightness = 10;
  strip->setBrightness(brightness);
  if (!onTask) onTask = true;
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip->Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < numPixels; i++) {  // For each pixel in strip
    strip->setPixelColor(i, color);      // Set pixel's color (in RAM)
    strip->show();                       // Update strip->to match
    delay(wait);                         // Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip->Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait)
{
    for (int a = 0; a < 10; a++)
    { // Repeat 10 times...
        for (int b = 0; b < 3; b++)
        {                  //  'b' counts from 0 to 2...
            strip->clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in steps of 3...
            for (int c = b; c < numPixels; c += 3)
            {
                strip->setPixelColor(c, color); // Set pixel 'c' to value 'color'
            }
            strip->show(); // Update strip with new contents
            delay(wait);  // Pause for a moment
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow()
{
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
    {
        for (int i = 0; i < numPixels; i++)
        {   // For each pixel in strip.
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (numPixels steps):
            int pixelHue = firstPixelHue + (i * 65536L / numPixels);
            // strip->ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip->gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixelHue)));
        }
        strip->show(); // Update strip->with new contents
    }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait)
{
    int firstPixelHue = 0; // First pixel starts at red (hue 0)
    for (int a = 0; a < 30; a++)
    { // Repeat 30 times...
        for (int b = 0; b < 3; b++)
        {                  //  'b' counts from 0 to 2...
            strip->clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in increments of 3...
            for (int c = b; c < numPixels; c += 3)
            {
                // hue of pixel 'c' is offset by an amount to make one full
                // revolution of the color wheel (range 65536) along the length
                // of the strip (strip->numPixels() steps):
                int hue = firstPixelHue + c * 65536L / numPixels;
                uint32_t color = strip->gamma32(strip->ColorHSV(hue)); // hue -> RGB
                strip->setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
            }
            strip->show();                // Update strip->with new contents
            delay(wait);                 // Pause for a moment
            firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
        }
    }
}

void clear() {
  strip->clear();
  strip->show();
}

void sleep() {
  clear();               // clear ring
  onSuspend = true;      // prevent button delay issue

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

void setup() {
  // initialize pseudo-random number generator with some random value
  randomSeed(analogRead(RNDPIN));

  // initialize LED strip
  strip = new Adafruit_NeoPixel(numPixels, pin, pixelFormat);
  strip->begin();
  strip->setBrightness(20);
  strip->show();
  current_color=eeprom_read_dword(0);
  // initialize Button
  buttonA = new Button2(BUTTON_A_PIN);
  buttonA->setClickHandler(OnClickHandler);
  buttonA->setLongClickHandler(OnLongClickHandler);
  buttonA->setDoubleClickHandler(OnDoubleClickHandler);
  
  ADCSRA &= ~_BV(ADEN); // Disable ADC, it uses ~320uA
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop() {
  buttonA->loop();
  if (!intro) {
    rainbow();
    intro = true;
    colorWipe(0,40);
    sleep();
  }
  if (onTask) {
    colorWipe(current_color,30);
    sleepCounter=0;
    onTask = false;
  }
  if (sleepCounter++>9000){
    sleepCounter=0;
    sleep();
  }
  if (sleepCounter==2000) {
    theaterChase(current_color,15);
    colorWipe(0,5);
  }
  strip->show();
}
