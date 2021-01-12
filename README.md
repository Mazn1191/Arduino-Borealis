# Arduino-Borealis
An arduino project to simulate the visual effect of aurora borealis on an LED strip.

The apperance and "behaviour" of the effect can be easily manipulated using the defined configs at the top.

It utilizes the FastLED library by Daniel Garcia (https://github.com/FastLED/FastLED) for interfacing the LED strip. The library is not included but can be downloaded directly inside the Arduino IDE.

## Hardware
I ran this code on an Arduino Uno in conjunction with an indiviually adressable WS2812B LED Strip. A 100µF capacitor was used in parallel with the power supply.
I used a 5V 4A 20W power supply. This obviously varies by the number of LEDs used.

For further details on the wiring please refer to this article:
https://howtomechatronics.com/tutorials/arduino/how-to-control-ws2812b-individually-addressable-leds-using-arduino/

## WLED
If you're interested in using LED Strips as part of your home automation you might have heard of WLED.
The Arduino Borealis effect (with slight changes) is now  integrated as an effect in the [WLED project](https://github.com/Aircoookie/WLED).
The current release (version 0.11.1) does not yet contain this effect, but it's already there if you compile from code.
