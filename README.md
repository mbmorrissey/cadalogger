# cadalogger
arduino-inspired boards to support data logging in ecology and environmental science

## Preamble

This project aims to generate easy-to-use open-source hardware and software for data logging for ecology and environmental science field studies.  The goal is to produce hardware and software for arduino-like (and arduino-compatible) boards that bring together key elements needed for most data loggers, in arrangemets that support low-power field deployments.

Here is a video introducing the cadalogger project.  We're hoping to develop better documentation, including videos with better production quality, over the next few years!

[![A quick intro to cadalogger](https://img.youtube.com/vi/7qkBjCcAluY/0.jpg)](https://www.youtube.com/watch?v=7qkBjCcAluY)


## Contents
- [preamble with introductory video](#preamble)
- [programming information](#programming)
- [library and functions](#library)
- [ongoing development questions](#development)
- [examples](#examples)
- [do you want to try?](#contact)


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



## Library

The library files (cadalogger.h and cadalogger.cpp) available in the `/src` directory of this repository can either be included in a folder titled `cadalogger` in your local `/Arduino/libraries/` directory, or can be included in the same directory and any sketch (i.e., `.ino` file in the Arduino IDE) that uses them.  They can then be included in a sketch in the usual way using `#include "cadalogger.h"` or '#include <cadalogger.h>'


### Functions

    
`cadalogger(byte boardVersion)`

- constructor
- `boardVersion=CADALOGGERMINI` for small surface mount board
- `boardVersion=CADALOGGERMAXI` for hand-assembled largely through hole version

`void initialise()`
`void initialise(int heartbeat)`

- initialise with or without heartbeat
- sets pin states to minimise power loss, sets up RTC
- heartbeat is interval in seconds for a periodic RTC interrupt, and is used to allow sleep modes with regular wake-up
- arbitrary heartbeat intervals are allowed on if boardVersion==CADALOGGERMINI, for boardVersion==CADALOGGERMINI, 5s is currently required

`void flash(byte flashes)`

- flashes the onboard LED with 10ms on and 90ms off cycle
- sleeps the MCU while the LED is on and off to reduce power consumption relative to blinking with delay()

`void rest(int cycles)`

- alternative to delay() that sleeps the MCU rather than idling with full current consumption
- cycles is in units of 1024 hz cycles, so, a low-poer alternative to delay(1000) is rest(1024)

`void update_time()`

- copies time from the RTC to cadalogger::time array
- calling update_time() will normally be done immediately befor any need to access the correct time information from cadalogger::time

`void power_up_sd()`

- powers the uSD card circitry, connects SPI interface to the uSD, and initialises the card

`void power_down_sd()`
`void power_down_sd(bool wait)`

- disconnects the SPI interface from the uSD and powers down the uSD
- power_down_sd() will include a 1 second low power delay to allow the SD card to perform internal maintenance, following the SD card specification
- power_down_sd(wait=false) will skip the 1 second delay in powering down; recommended only if the delay is otherwise ensured in code; potentially useful if 1s of blocking by power_down_sd() is incompatible with a given application

`go_to_sleep_until_RTC_wake()`

- sleeps the MCU until an interrupt is generated by the RTC; requires that a hearbeat has been set using cadalogger::initialise(int heartbeat);

`void enable_watchdog()`
`void disable_watchdog()`

- enable and disable watchdog timer.  Currently only implements watchdog interval of abou 8.1s.  if feed_watchdog() is not used within any 8.1s period, a reset will occur

`void feed_watchdog()`

- reset watchdog timer (see immediately above)

void write_time_to_rtc();

- copies the time in cadalogger::time array to the RTC.  Used in setting the time.

`double rtc_temp()`

- returns the temperature in degrees C as measured by the board's RTC.
- this temperature measurement is imprecise, but useful for documenting the environmental conditions to which the logger has been exposed

### Cadalogger public variables

`cadalogger::time`

- an array containing the date/time in second, minute, hour, day, month, year

`cadalogger::SD`

- an instance of SD from library SdFat


## Development

### Hardware development

- MCU choice: the `CADALOGGERMINI` board version uses the ATMega4808 and the CADALOGGERMAXI uses the ATMega4809.  The 4808 comes in a more convenient package for surfact mount hand-assembly, while the 4809 is the only MCU in the family that comes in a surface mount package.  With a shift to automated assembly of the MINI board, we may consider changing it to using a surface mount version of the ATMega 4809 MCU.  This should cause few changes to usage, but some improvements.  For example, the built in LED pin could be made consistent between board versions, allowing the defintion of LED_BUILTIN so the standard blink sketch runns correctly, and likely other such useful minor streamlining.
- protection: the boards currently do not have reverse polarity (i.e., guards against attaching a battery backwards) or over-current protection.  The design appears to be proving to be fairly robust.  Adding these protections would have drawbacks including some combination of extra components (more cost and space), less efficient voltage regulator (i.e., higher sleeping current).  However, with a greater range of field testing experience, we think this decision should be revisited in the future.
- USB connectivity: the boards are programmed over USB via a USB to serial converter module.  Many such modules are on the market.  We expect many uses of cadalogger boards to involve deployments of multiple boards, and so a typical user will save money by not having the cost of a USB-serial converter on every board.  Such a converter could be added, but would come at a financial cost, plus a use of space and power
- storage media: cadalogger is primarily conceived for logging to SD card, but as efficiently as possible.  There could be a greater role for alternative storage media, such as FRAM (for which there is provision for optional addition to the back of the `CADALOGGERMINI` board).  SD cards are generaly robust and reliable existing arduino libraries allow them to be used with a file system that allows data to be written in familiar formats and easily shifted to a computer for import into spreadsheets, R, python, etc.  However, FRAM is probably more robust, and is likely to be necessary in very low temperature environments.  We suspect that the use of FRAM as an option is appropriate, but with more field testing and feedback from a wide range of users, we intend to revisit this decision in the future.
- alternate versions and expansion boards:  cadalogger is a logging platform, not a specific logger.  However, as different use cases arise with the repeated need for different peripherals we will consider alternate board versions for specific purposes.  More likely, facilitation of specific use cases can be accommodated with expansion boards.

### Software (cadalogger library and board definition) development

- cadalogger functions include very little error and exception handling; most functions return `void` rather than useful indicators of success or errors
- many things are hard-coded that should be flexible or more flexible.  For e.g., some aspects of wake/sleep cycles and watchdog timer
- cadalogger library functionality works in ways that make sense to @mbmorrissey and @rebebba; how much they are intuitive to others, and how to make them intuitive, will result from consultation with expertis in open source materials and from feedback from diverse users
- possible specific changes to library too numerous to consider, but for e.g., many things are hard-coded that should be flexible or more flexible, incl. some aspects of wake/sleep cycles and watchdog timer.  Another example is handling of date-time information
- decision needs to be made about accommodating fuller functionality of RTCs via the cadalogger library vs whether full functionality should be exposed via libraries; if so, need to determine if we satisfied with existing libraries
- future development will generate a formal arduino board definition in order to simplify initial set-up

### Example bank development

- we are slowly developing a set of examples.  We are seeking examples contributed from others to increase the diversity of cases and their presentation
- we would like to develop more extensive examples including code and hardware descriptions
- we would also like to build up more extensive extensive case study examples, including comparisons to existing or alternate types of installations, detailed ground-truthing and callibration, and accounting for costs very broadly, including human effort

## Examples

See the /examples/ directory for code for each example.  Here is some further context for each.

*Please note that most of these examples are not extensively field tested.  They are not intended as advice as to what designs will work well and generate valid data.  These examples are intended to be instructive about how cadaloggers might be deployed to various uses.  We still need to embark on a major effort to assess what kinds of deployments are reliable, and where problems can arise.  We also make no guarantee that data collected in any of these examples will be fit for any specific purpose.  Use-specific validation is required for any application based on these examples.*

### Blink: the 'Hello world!' of microcontrollers

The sketch `blink.ino` in the `/examples/` directory can be uploaded to a cadalogger board using the directions for programming above.  This sketch is designed to be as similar as possible to blink on any other arduino-compatible microcontroller board.  `flash.ino` is a blink equivalent that uses the cadalogger libraries functions for manipulating the on-board LED in a more energy efficient way.

### `set_time.ino`: getting the time onto the real time clock



### Sunshine: the 'Hello world!' of environmental data loggers

A common tutorial to introduce collecting information from a sensor involves reading a light level using a light dependent resistor or a photodiode.  This examples does the same, but introduces other data logging aspects, saving the light level data to a SD card with time stamps, and sleeping with very low power consumption betwen reads.

Connections:
- ...

### Templogger: from 'sunshine', it isn't too far to other programs

Here we swap out the sensor in the sunshine example for a DS18b20 waterproof temperature probe.

Connections:
- ...


### Temperature logger with ultrasonic distance (river stage and temperature)

This design adds a waterproof ultrasonic distance censor to log river water level, as well as water temperature.  The sketch is in the `/examples/` directory (direct link).  The logger wakes every ten minutes to take a series of readings from the distance sensor, of which ten are kept and averaged.  Time, distance to the water surface, and water temperature are written to the SD card.

Connections
- 3xAA battery pack + to cadalogger Vin
- 3xAA battery pack - to cadalogger GND
- 4.7k resistor from DS18b20 signal (yellow) to 3.3
- cadalogger 3.3V to DS18b20 red wire
- cadalogger XX to DS18b20 yellow wire
- cadalogger GND to DS18b20 black wire
- cadalogger 3.3V to JSN-SR04Tv3 5V
- cadalogger GND to JSN-SR04Tv3 GND
- cadalogger TX to JSN-SR04Tv3 XX
- cadalogger RX to JSN-SR04Tv3 XX

### Temperature logger with satellite data transmission

This design returns to the simple scheme of only logging temperature. However, in addition to saving the temperature record, it transmits the data over the Iridium satellite network using a RockBlock 9603 modem.  A subscription to the satellite service is required (see here).

The logger wakes every hour to record temperature, and saves 12 reads before each transmitting, thus transmitting a bulk data summary twice a day.

What becomes of the messages depends on settings in that can be arranged when you subscribe to the service.  In the video, we simply see the data being received by email.

Messages up to 50 bytes have a fixed cost, so for simplicity

The satellite modem is reasonably efficient (considering it is transmittign a message that can be detected in space!), but this design would be expected to last approximately X weeks on 3xAA batteries.  We have tested it with AAs for short periods, but suggest it would be most useful to deploy with higher capacity batteries, such as 3xD cells.

### An RS232 sensor and solar power

A major design goal for cadalogger is to make it plausible to power long-term data logging deployments off of inexpensive and easy to obtain batteries.  However, sometimes we may wish to operate sensors that require enough power that some kind of topping up may be necessary.  In this example, a WindSonic (TM) ultrasonic anemometer is operated using a cadalogger board...


## Contact

We have some, but presently limited, ability to provide prepared cadalogger boards.  We will try to help out anyone who is interested in using or testing!  If we cannot provide an assembled board, we can provide guidance on navigating the full open-source documentation provided here (e.g., see `/hardware/` drectory for schematics, gerbers, and BOM to build your own.  Please get in touch via [Michael](mailto:mbm5@st-andrews.ac.uk) or [Rebecca](mailto:rn71@st-andrews.ac.uk).



