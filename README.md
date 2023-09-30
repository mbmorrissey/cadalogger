# cadalogger
arduino-inspired boards to support data logging in ecology and environmental science

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/X45r74pawpw/0.jpg)](https://www.youtube.com/watch?v=X45r74pawpw)

## Preamble

This project aims to generate easy-to-use open-source hardware and software for data logging for ecology and environmental science field studies.  The goal is to produce hardware and software for arduino-like (and arduino-compatible) boards that bring together key elements needed for most data loggers, in arrangemets that support low-power field deployments.

## Motivation

[...exmaplain the hurdles to getting an arduino (or similar) or RPi-based system from the bench to the field...]

## Hardware

[...overview, photos, brief description of compact vs easy-to-assemble verions, pinouts]


## Programming

### MegacoreX

All examples here us an open source arduino core for the family of MCUs that cadalogger boards use, MegacoreX by MCUDude.  Roughly, the purpose arduino core is to translate standard arduino functions into the specific bits of underlying code necessary for a given microcontroller to understand.

MCUDude provides comprehensive information on using MegacoreX.  Here we provide an overview to get started with cadalogger boards.

In the Arduion IDE, go to  'Arduino IDE' -> 'Preferences...'.  Then in the 'Additional board manager URLs' filed, add 
https://mcudude.github.io/MegaCoreX/package_MCUdude_MegaCoreX_index.json .  If you already have, or subsequently add, other URLs, they can just be separated with commas.

An option of 'MegaCoreX' should then under 'Tools' -> 'Board'.  Within the 'MegaCoreX' options, select ATmega4808 if using the cadalogger lamb version, or 'ATmega4809' if using the cadalogger ram version.  After selecting the appropriate MCU option, go back to 'Tools' -> 'Pinout', and ensure that '32 pin standard' is selected if using the lamb version or '40 pin standard' if using the ram version.

If your board has a bootloader, you can skip to 'Cadalogger library'.

### Programming

The ATMega480X MCUs used in cadalogger boards are programmed over UPDI.  The boards have three pins for programming: ground, 3.3V and UPDI.

[include images of both boards, highlighting programming headers]

An UPDI programmer is required.  SerialUPDI is the simplest and least expensive option.  See here for more

### Uploading a bootloader

A new build of a cadalogger board will need an bootloader to be burned.  This allows the microcontroller to receive further programming instructions over serial.

Using a SerialUPDI adapter, see above, you will want to set the following



### Cadalogger library

[instructions on how to install the library from zip]



### Blink: the 'Hello world!' of microcontrollers

[instructions on how to get blink working]



## Examples

Here we list two simple examples for getting started.  More varied examples are detailed here.

*sunshine.ino* A simple data logger example demonstrating several power-saving tricks including:

- deep sleep
- depowering the SD card
- powering sensor circuits with a MCU pin
- saving data from multiple data reads between bulk writes to the SD card





