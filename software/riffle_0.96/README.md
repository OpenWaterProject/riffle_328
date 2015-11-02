# Riffle v0.96 Software

Here is the software for the Riffle board version 0.96, which is the production board for the summer of 2015.


## Requirements

### Power Up

The Riffle's brain needs to be on to interact with it. (The "power" led just means there's power to the board, not necessarily that the brain is on. Just slide the switch toward the edge of the board to turn it on.

### Arduino Libraries

The Riffle's Real Time Clock needs a particular flavor of the RTC libarary to be in your Arduino's "Library" folder, which comes via [mizraith's Github page](https://github.com/mizraith/RTClib). Here's what to do:

  - [Download this zip file](https://github.com/mizraith/RTClib/archive/master.zip)
  - Unzip it
  - Remove the "-master" from the folder name so it's "RTClib" instead of "RTClib-master"
  - Move that whole folder into your Arduino "libraries" folder. ([Here is more information](https://learn.sparkfun.com/tutorials/installing-an-arduino-library) on doing that, if you need it.)
  - Restart your Arduino software (also known as the Ardunio IDE).

### USB Driver

The Riffle is using a special USB chip, and may require a special driver so that it is recognized by your computer.

#### Mac OS

For Macs, you'll definitely need a driver. Download and install it from here: [http://raysfiles.com/drivers/ch341ser_mac.zip](http://raysfiles.com/drivers/ch341ser_mac.zip)

#### Ubuntu

If you're using Ubuntu, you don't need anything special.

## Arduino Setup

- Lie to your Arduino software and tell it that Riffle is an "Arduino Uno" in the `Tools > Board` menu.

### Mac OS

- Under `Tools > Serial Port` pick the thing that looks most like: `/dev/tty.wchusbserialXXXX`



