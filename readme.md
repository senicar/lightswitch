LightSwitch
===========

> **Disclaimer**
>
> This code is a result of a learning process, so it is far from perfect.
>
> Proceed with caution.

This projects is based on Arduino platform. However the code is written for the Teensy2.0++ since it has certain functions Arduino UNO does not. Most notably, number of TIMERS.

TLC5940 needs two timers and IR needs one, IR cannot be used with Arduino Uno since it has only one timer. Use teensy 2.0++, arduino mega or other arduino compatible board that has three timers. If IR is not needed all reference to it should be removed and the code could be used on Arduino UNO.

Beacuse of slightly different setup, a proper [TIMER must be set](https://github.com/shirriff/Arduino-IRremote/blob/master/IRremoteInt.h) in the IRremote library, since by default it uses the same TIMER2 as TLC5940


## Touch Screen ##

The main control of the lights is via touch panel. See [this bildr tutorial](http://bildr.org/2011/06/ds-touch-screen-arduino/) on how to set it up.


## Dependencies ##

* [Arduino-IRremote](https://github.com/shirriff/Arduino-IRremote)

* [tlc5940arduino](https://code.google.com/p/tlc5940arduino/) and how to [wire it with Teensy](https://www.pjrc.com/teensy/td_libs_Tlc5940.html)

## Circuit Diagram ##

![Circuit Diagram](/fritzig/circuit.jpg?raw=true "Circuit Diagram")

