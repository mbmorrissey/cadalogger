# cadalogger
arduino-inspired boards to support data logging in ecology and environmental science

cadal = sleep in Scots Gaelic: ability to sleep very efficiently is a key feature

logger: data logging

## Preamble

This project aims to generate easy-to-use open-source hardware and software for data logging for ecology and environmental science field studies.  The goal is to produce hardware and software for arduino-like (and arduino-compatible) boards that bring together key elements needed for most data loggers, in arrangemets that support low-power field deployments.

Here is a video introducing the cadalogger project.  We're hoping to develop better documentation, including videos with better production quality, over the next few years!

[![A quick intro to cadalogger](https://img.youtube.com/vi/7qkBjCcAluY/0.jpg)](https://youtu.be/G2LDQLVE-7E)


## Contents
- [preamble with introductory video](#preamble)
- [programming information](#programming)
- [library and functions](#library)
- [ongoing development questions](#development)
- [examples](#examples)
- [do you want to try?](#contact)


## Programming

### MegacoreX

All examples here use an open source arduino core for the family of MCUs that cadalogger boards use, MegacoreX by MCUDude.  Roughly, the purpose arduino core is to translate standard arduino functions into the specific bits of underlying code necessary for a given microcontroller to understand.

MCUDude provides comprehensive information on using MegacoreX.  Here we provide an overview to get started with cadalogger boards.

In the Arduion IDE, go to  'Arduino IDE' -> 'Preferences...'.  Then in the 'Additional board manager URLs' filed, add 
https://mcudude.github.io/MegaCoreX/package_MCUdude_MegaCoreX_index.json .  If you already have, or subsequently add, other URLs, they can just be separated with commas.

An option of 'MegaCoreX' should then appear under 'Tools' -> 'Board'.  Within the 'MegaCoreX' options, select ATmega4808 if using the cadalogger lamb version, or 'ATmega4809' if using the cadalogger ram version.  After selecting the appropriate MCU option, go back to 'Tools' -> 'Pinout', and ensure that '32 pin standard' is selected if using the lamb version or '40 pin standard' if using the ram version.

If your board has a bootloader, you can skip to 'Cadalogger library'.

### Programming

The ATMega480X MCUs used in cadalogger boards are programmed over UPDI.  The boards have three pins for programming: ground, 3.3V and UPDI.

[include images of both boards, highlighting programming headers]

An UPDI programmer is required for a new board.  SerialUPDI is the simplest and least expensive option.  See below for more as well as 
[here](https://github.com/SpenceKonde/AVR-Guidance/blob/master/UPDI/jtag2updi.md).  A dedicated UPDI programmer is 
also a good option, see [here](https://github.com/MCUdude/microUPDI).  Unless we have supplied you with an example 
board, and said that we have uploaded a bootloader, you'll have to do so.

Once a bootloader is uploaded, you will need to connect a USB-serial adapter to the cadalogger's 6-pin programming header, and select the following settings in the Arduino 
IDE:

For cadalogger mini
- Tools > Board > MegaCoreX > ATMega4808
- Tools > Port > as appropriate for your USB-serial adapter
- Tools > Bootloader > Optiboot (UART0 default pins)
- Tools > Clock > Internal 4 MHz
- Tools > Pinout > 32 pin standard
- Tools > Reset pin > reset

For cadalogger maxi
- Tools > Board > MegaCoreX > ATMega4809
- Tools > Port > as appropriate for your USB-serial adapter
- Tools > Bootloader > Optiboot (UART0 default pins)
- Tools > Clock > Internal 4 MHz
- Tools > Pinout > 48 pin standard
- Tools > Reset pin > reset

Once these settings are in place, you should be able to upload using the upload button near the top left of any sketch's window in the IDE, or using Sketch > Upload, or 

### Uploading a bootloader

A new build of a cadalogger board will need an bootloader to be burned.  This allows the microcontroller to receive further programming instructions over serial.

Using a SerialUPDI adapter, see above, you will want to set up as follows

- SerialUPDI connector inserted between USB-serial converter module, and conected to GND, V and UPDI on cadalogger board

For cadalogger lamb
- Tools > Board > MegaCoreX > ATMega4808
- Tools > Port > as appropriate for your USB-serial adapter
- Tools > Bootloader > Optiboot (UART0 default pins)
- Tools > Clock > Internal 4 MHz
- Tools > Pinout > 32 pin standard
- Tools > Reset pin > reset

For cadalogger ram
- Tools > Board > MegaCoreX > ATMega4809
- Tools > Port > as appropriate for your USB-serial adapter
- Tools > Bootloader > Optiboot (UART0 default pins)
- Tools > Clock > Internal 4 MHz
- Tools > Pinout > 48 pin standard
- Tools > Reset pin > reset

Once set up, selecte Tools > Burn Bootloader.  A bootloader shoud be uploaded, and subsequent uploading of compiled sketches should be possible directly using a USB-serial 
module and the 6-pin serial header pins.

## Pinouts

Pinout for the main cadalogger board (not shown: most uses will require a 3V coin cell; a coin cell clip can be soldered to pads on the back fo the 'mini' board variant):

<img src="/images/MINI_pinout.png" width="400" height="400">

Pinout for the board variant optimised for hand assembly:

<img src="/images/MAXI_pinout.png" width="500" height="620">

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
- calling update_time() will normally be done immediately before any need to access the correct time information from cadalogger::time

`void power_up_sd()`

- powers the uSD card circuitry, connects SPI interface to the uSD, and initialises the card

`void power_down_sd()`
`void power_down_sd(bool wait)`

- disconnects the SPI interface from the uSD and powers down the uSD
- power_down_sd() will include a 1 second low power delay to allow the SD card to perform internal maintenance, following the SD card specification
- power_down_sd(wait=false) will skip the 1 second delay in powering down; recommended only if the delay is otherwise ensured in code; potentially useful if 1s of blocking by power_down_sd() is incompatible with a given application

`go_to_sleep_until_RTC_wake()`

- sleeps the MCU until an interrupt is generated by the RTC; requires that a hearbeat has been set using cadalogger::initialise(int heartbeat);

`void enable_watchdog()`
`void disable_watchdog()`

- enable and disable watchdog timer.  Currently only implements watchdog interval of about 8.1s.  If feed_watchdog() is not used within any 8.1s period, a reset will occur

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

- MCU choice: the `CADALOGGERMINI` board version uses the ATMega4808 and the CADALOGGERMAXI uses the ATMega4809.  The 4808 comes in a more convenient package for surface mount hand-assembly, while the 4809 is the only MCU in the family that comes in a surface mount package.  With a shift to automated assembly of the MINI board, we may consider changing it to using a surface mount version of the ATMega 4809 MCU.  This should cause few changes to usage, but some improvements.  For example, the built in LED pin could be made consistent between board versions, allowing the defintion of LED_BUILTIN so the standard blink sketch runs correctly, and likely other such useful minor streamlining.
- protection: the boards currently do not have reverse polarity (i.e., guards against attaching a battery backwards) or over-current protection.  The design appears to be proving to be fairly robust.  Adding these protections would have drawbacks including some combination of extra components (more cost and space), less efficient voltage regulator (i.e., higher sleeping current).  However, with a greater range of field testing experience, we think this decision should be revisited in the future.
- USB connectivity: the boards are programmed over USB via a USB to serial converter module.  Many such modules are on the market.  We expect many uses of cadalogger boards to involve deployments of multiple boards, and so a typical user will save money by not having the cost of a USB-serial converter on every board.  Such a converter could be added, but would come at a financial cost, plus a use of space and power.
- storage media: cadalogger is primarily conceived for logging to SD card, but as efficiently as possible.  There could be a greater role for alternative storage media, such as FRAM (for which there is provision for optional addition to the back of the `CADALOGGERMINI` board).  SD cards are generaly robust and reliable, and existing arduino libraries allow them to be used with a file system that allows data to be written in familiar formats and easily shifted to a computer for import into spreadsheets, R, python, etc.  However, FRAM is probably more robust, and is likely to be necessary in very low temperature environments.  We suspect that the use of FRAM as an option is appropriate, but with more field testing and feedback from a wide range of users, we intend to revisit this decision in the future.
- alternate versions and expansion boards:  cadalogger is a logging platform, not a specific logger.  However, as different use cases arise with the repeated need for different peripherals we will consider alternate board versions for specific purposes.  More likely, facilitation of specific use cases can be accommodated with expansion boards.

### Software (cadalogger library and board definition) development

- cadalogger functions include very little error and exception handling; most functions return `void` rather than useful indicators of success or errors
- many things are hard-coded that should be flexible or more flexible.  For e.g., some aspects of wake/sleep cycles and watchdog timer
- cadalogger library functionality works in ways that make sense to @mbmorrissey and @rebebba; how much they are intuitive to others, and how to make them intuitive, will result from consultation with experts in open source materials and from feedback from diverse users
- possible specific changes to library too numerous to consider, but for e.g., many things are hard-coded that should be flexible or more flexible, incl. some aspects of wake/sleep cycles and watchdog timer.  Another example is handling of date-time information
- decision needs to be made about accommodating fuller functionality of RTCs via the cadalogger library vs whether full functionality should be exposed via libraries; if so, need to determine if we are satisfied with existing libraries
- future development will generate a formal arduino board definition in order to simplify initial set-up

### Example bank development

- we are slowly developing a set of examples.  We are seeking examples contributed from others to increase the diversity of cases and their presentation
- we would like to develop more extensive examples including code and hardware descriptions
- we would also like to build up more extensive case study examples, including comparisons to existing or alternate types of installations, detailed ground-truthing and callibration, and accounting for costs very broadly, including human effort

## Examples

See the `/examples/` directory for code for each example.  Here is some further context for each.

*Please note that most of these examples are not extensively field tested.  They are not intended as advice as to what designs will work well and generate valid data.  These examples are intended to be instructive about how cadaloggers might be deployed to various uses.  We still need to embark on a major effort to assess what kinds of deployments are reliable, and where problems can arise.  We also make no guarantee that data collected in any of these examples will be fit for any specific purpose.  Use-specific validation is required for any application based on these examples.*

### Blink: the 'Hello world!' of microcontrollers

The sketch `blink.ino` in the `/examples/` directory can be uploaded to a cadalogger board using the directions for programming above.  This sketch is designed to be as similar as possible to blink on any other arduino-compatible microcontroller board.  `flash.ino` is a blink equivalent that uses the cadalogger libraries functions for manipulating the on-board LED in a more energy efficient way.

### `set_time.ino`: getting the time onto the real time clock

A critical aspect of almost any data logging system is time-keeping.  cadalogger boards include high-quality temperature-compensated real-time clocks (RTCs), and the library contains functions to use these components.  When first using a cadalogger board, it will probably be necessary to set the time on the RTC.  So long as a coin cell battery with a charge is maintained on the board, the time should be retained.  `set_time.ino` is a sketch that takes your computer's system time, and incorporates it into a sketch on compilation.  This time is then uploaded onto the RTC.

However, you don't want the time that your code was compiled to be written to the RTC every time your logger starts up!  The solution in `set_time.ino` is that the sketch only writes the time to the RTC if digital pin 19 (A7 on the cadalogger mini board, A5 on the maxi board) is connected to ground.  Otherwise, the step of writing the time of compilation to the RTC is skipped.

So, to set the time, you connect pin 19 to ground, and then compile and upload the sketch.  Open a serial console in the Arduino IDE, and the cadalogger's time will be scrolling by.  You can set the time back by entering 1, and set it forward by entering 2, in the serial console.  Depending on how long your upload has taken, you'll have to set the time forward from the time of compilation by about 10-20 seconds.

After setting the time, remove the connection from digital pin 19 to ground, and the time should stay set, even when you upload a different program for whatever logging application your cadalogger board is destined to be used in.


### Sunshine: the 'Hello world!' of environmental data loggers

A common tutorial to introduce collecting information from a sensor involves reading a light level using a light dependent resistor or a photodiode.  This example does the same, but introduces other data logging aspects, saving the light level data to a SD card with time stamps, and sleeping with very low power consumption betwen reads.

The light sensing circut can be a simple voltage divider.  An approximately 2k resistor, in series with a TEPT5700 photodiode works well.  A similar circuit with a fixed value resistor and a light-dependent resistor will work well too.

Connections:
- digital pin 22 to approx 2k resistor
- 2k resistor to anode (long lead) of photodiode
- cathode (short lead) of photodiode to ground
- analog pin A0 to connection of resistor to photodiode

This logger will record a reading from the light circuit in ADC units (a 10 bit value between 0 and 1023), along with the time (the correct time, if `set_time.ino` has been used before uploading `sunshine.ino` to the board

### Temperature logger 

A very basic temperature logger.  It reads very frequently (every ten seconds), and does a data write for every read.  This isn't very efficient relative to duty cycles that would be sensible for most real logging applications, but it made the example in the introductory video less boring!

There are lots of DS18b20 arduino examples on the internet.  This one is a little bit different becuase it de-powers the sensor.  This requires grounding both the power and data pins durign sleep.  Very low power operation, and substantial logger longevity on small battery packs, can be achieved without going quite to this extreme, but it helps to highlight the extent of low power operation achievable with cadalogger boards.

To use the DS18b20 sensor with MegaCoreX, a slight modification of the OneWire library is required.  Once OneWire is installed, navigate to your /Arduino/libraries/OneWire/utils/ directory and open the file OneWire_direct_gpio.h.  The 18th line will read `#if defined(__AVR_ATmega4809__)`  Modify it to read `#if defined(__AVR_ATmega4809__) || defined(MEGACOREX)`  Now the standard DS18b20 library will be able to work.  In future will will figure out how to integrate more seemlesly with the OneWire library.

Connections:
- digital pin 4 to DS18b20 power
- digital pin 19 to DS19b30 data
- 4.7k pullup resistor between DS18b20 data and power


### Temperature logger with ultrasonic distance (river stage and temperature)

This design uses waterproof ultrasonic distance censor to log river water level, as well as water temperature; this is the logger featured briefly in the introductory video at the top of the page.  The sketch is in the `/examples/` directory.  The logger wakes every ten minutes to take a series of readings from the distance sensor, of which ten are kept and averaged.  Time, distance to the water surface, and water temperature are written to the SD card.

Connections
- 3xAA battery pack + to cadalogger Vin
- 3xAA battery pack - to cadalogger GND
- 4.7k resistor from DS18b20 signal (yellow) to 3.3
- cadalogger 3.3V to DS18b20 red wire
- cadalogger digital 19 (aka A7) to DS18b20 yellow wire
- cadalogger GND to DS18b20 black wire
- cadalogger 3.3V to JSN-SR04Tv3 5V
- cadalogger GND to JSN-SR04Tv3 GND
- cadalogger TX to JSN-SR04Tv3 RX
- cadalogger RX to JSN-SR04Tv3 TX

### Temperature logger with satellite data transmission

This design returns to the simple scheme of only logging temperature. However, in addition to saving the temperature record, it transmits the data over the Iridium satellite network using a RockBlock 9602 modem.  A subscription to the satellite service is required (see [here](https://www.groundcontrol.com/en/)).

The logger wakes every hour to record temperature, and saves 12 reads before each transmitting, thus transmitting a bulk data summary twice a day.

What becomes of the messages depends on settings that can be arranged when you subscribe to the service.  In the video, we simply see the data being received by email.

This sketch does not transmit data very efficiently, in part because we were interested in transmitting frequently in order to get an idea of reliability.  Messages up to 50 bytes have a fixed cost, so for a longer-term deployment, we would probably pack at least 12 hourly measurements into each transmission, rather than transmitting approximately every hour. 

The satellite modem is reasonably efficient (considering it is transmittign a message that can be detected in space!), but this design would be expected to last approximately X weeks on 3xAA batteries.  We have tested it with AAs for short periods, but suggest it would be most useful to deploy with higher capacity batteries, such as 3xD cells (as in the video).

We used a [RockBlock 9602 Iridium satellite modem](https://www.groundcontrol.com/en/product/rockblock-9602-satellite-modem/) for this example.  This was a bit of a mistake, becuase the 9602 version needs a 5V power supply, while we could have used the [RoclBlock 9603 modem](https://www.groundcontrol.com/en/product/rockblock-9603/) and powered it directly of the 3xAA or 3xDD battery pack.  Consequentially, we needed an extra device: a step-up voltage regulator to provide 5V.  We happened to have an [Adafruit 5V TPS61023 boost converter](https://www.adafruit.com/product/4654) to hand, so we used it.

Connections
- 3xDD positive to cadalogger power supply pin
- 3xDD positive to Vin on 5V regulator
- 3.3V output from cadalogger to Vin of waterproof DS18b20
- cadalogger mini D6 to data (yellow) of waterproof DS18b20
- 4.7k pullup resistor from DS18b20 data to 3.3V
- TX2 on cadalogger mini to modem RX
- RX2 on cadalogger mini to modem TX
- cadalogger pin D7 (aka SS) to modem sleep control
- 5V output from regulator to model power supply
- connect all grounds (battery, cadalogger, 5V regulator, DS18b20, modem)


## Contact

We have some, but presently limited, ability to provide prepared cadalogger boards.  We will try to help out anyone who is interested in using or testing!  If we cannot provide an assembled board, we can provide guidance on navigating the full open-source documentation provided here (e.g., see `/hardware/` drectory for schematics, gerbers, and BOM to build your own.  Please get in touch via [Michael](mailto:mbm5@st-andrews.ac.uk) or [Rebecca](mailto:rn71@st-andrews.ac.uk).



