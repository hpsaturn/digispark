# Digispark Projects

<a href="https://github.com/hpsaturn/digispark/blob/master/images/multi_dice_touch.gif" target="_blank"><img src="https://raw.githubusercontent.com/hpsaturn/digispark/master/images/multi_dice_touch.gif" align="right" width="240" ></a>

Some demo projects using [ATTINY85 Digispark](https://www.aliexpress.com/item/32897732904.html) interfaced with a [12 Bits RGB LED Ring WS2812](https://www.aliexpress.com/item/32666384944.html)

Current demos:

- [X] Blink LED 
- [X] Neopixel demo
- [X] EEPROM color setting
- [X] Neopixel demo
- [X] Neopixel button handler (click, long click, doubleclick)
- [X] Touchbutton demo
- [X] Multi dice with touch button (master)

# Compiling and installing

First, please install [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** code and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system. 

Note: with `platformIO` you don't need the Arduino IDE and install libraries, this will do it for you.

## Select project tag

First clone the project
``` bash
git clone https://github.com/hpsaturn/digispark.git && cd digispark
```

List projects: 
``` bash
git tag -n9
```

Select and compiling:
``` bash
git checkout multi_dice && pio run --target upload
```

Note: you need connect your digispark after each compiling for upload the new firmware or reset it. More info [here](http://digistump.com/wiki/digispark/tutorials/connectingpro)