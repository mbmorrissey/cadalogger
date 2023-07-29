# cadalogger
arduino-inspired boards to support data logging in ecology and environmental science

## Preamble

This project aims to generate easy-to-use open-source hardware and software for data logging for ecology and environmental science field studies.  The goal is to produce hardware and software for arduino-like (and arduino-compatible) boards that bring together key elements needed for most data loggers, in arrangemets that support low-power field deployments.

## Motivation

[...exmaplain the hurdles to getting an arduino (or similar) or RPi-based system from the bench to the field...]

## Hardware

[...overview, photos, brief description of compact vs easy-to-assemble verions, pinouts]

## Programming

### Programming

The ATMega480X MCUs used in cadalogger boards are programmed over UPDI.  The boards have three pins for programming: ground, 3.3V and UPDI.

[include images of both boards, highlighting programming headers]



[...UPDI, bootloaders, USB-serial, MegaCoreX...]

## Power considerations

- idle, data writing, and sleep power consumption
- managing power to sensors
- power budget calculations
- example of power provision for different field scenarios


## Examples

*sunshine.ino* A simple data logger example demonstrating several power-saving tricks including:

- deep sleep
- depowering the SD card
- powering sensor circuits with a MCU pin
- saving data from multiple data reads between bulk writes to the SD card
