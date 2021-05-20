# kmclib
Library of functions needed in almost all linux C/C++ programs

# Documentation
https://projects.dj0abr.de/doku.php?id=de:rpictlbrd:ctlbrd_functions

# hamcontrol
Hardware support for this board:
https://projects.dj0abr.de/doku.php?id=de:rpictlbrd:ctlbrd_overview

## Development Status
working with Raspi OS
workig with DietPi

## Overview
this library contains a skeleton with many useful and easy to use functions
which are interesting for many ham radio linux projects.

I HATE (!) software which is complicated to use, has a lot of functions hard to understand, need many tricks and nasty fiddling.

Therefore I have created a set of very easy to use modules for many job in many programs:

* serial interfaces, local and USB/serial, solves the problem of changing ttyUSB0/1... numbers
* GPS handler including sun position calculation
* Stepper motor controller
* linear motor controller
* Rotary encoder
* UDP: receiving and sending messages
* drawing to the Raspberry PI 3,5" Waveshare TFT-Touch display
* bargraphs for measurement value display
* touch buttons
* web socket for data transfer to web browsers
* Thread safe FIFOs
* Timers
* more ... see documentation

If you use my RPI control board or not, these functions will make RPI software development much easier.
If using a different hardware, just deactivate the I2C initialisation and don't use GPIO or ADC functions. All other stuff will just work.

## build the library
make clean

make

sudo make install

## build an application with kmclib
see the sample: main_sample with it's makefile

make -f Makefile_sample clean

make -f Makefile_sample

./sample (just prints a welcome message and runs in an endless loop. Press Ctrl-C to exit)

## typical applications we had in mind when creating this library

* automatic short wave tuner
* solar panel controller driving a panel according the sun position
* remote control of antenna rotors by a web browser
* weather station controller
* controller for Oil and Wood Heatings
and many more....