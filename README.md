HC-SR04 Ultrasonic Ranging Module Distance Calculator
=================

This is a simple user space program to read distance from multiple HC-SR04 ultrasonic ranging module using GPIO ports on Rasberry Pi.

Ultrasonic ranging module datasheet: (http://www.micropik.com/PDF/HCSR04.pdf)

There are two versions implemented in this repository. The `getdist2` is the recommended one.

getdist
-------
Aim of this version was to measure more distances using multiple sensors in one program loop to obtain all distances as soon as possible.
However, this can lead to decreased accuracy. Therefore `getdist2` is recommended.


getdist2
--------
This version performs serialized measurements using multiple sensors. 

Because this is run in userspace (and maybe also because of something else) occationally we can get very high numbers. To cope with this problem and also to improve accuracy, each distance was measured five times and median was selected as resulting value. Because measurements is relatively fast (tens of ms), changes between measurements can be ignored.

### Building & setup
Standard makefile is attached. To compile run:

```
make getdist2
```

Program is interfacing GPIO porst using `/sys/class/gpio/`. GPOI ports must be initialized and their direction must be set correctly. An example of setup script is shown in `setup_gpios.sh`.

### Usage
Number of sensors is variable. For each sensor `TRIGGER_GPIO ECHO_GPIO` argument pair should be specified. Additionally the last argument can be `debug`, which cause the program to run in debug mode (all five measurements will be displayed)

**General usage:**

```
./getdist2 [TRIGGER_GPIO] [ECHO_GPIO]... [debug]
```

**Example:**

```
./getdist2 9 11 14 15 8 7 12 16 debug
```

In the example above 4 sensors are measured with the following (trigger,echo) GPIO pin pairs: `(9,11) (14,15) (8,7) (12,16)`.


Author
------

Team TechNoLogic <teamtechnologic@googlegroups.com>

License (MIT)
-------------

Copyright (c) 2014 Team TechNoLogic

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
