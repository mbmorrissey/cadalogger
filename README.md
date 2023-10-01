# cadalogger
arduino-inspired boards to support data logging in ecology and environmental science



## Preamble

This project aims to generate easy-to-use open-source hardware and software for data logging for ecology and environmental science field studies.  The goal is to produce hardware and software for arduino-like (and arduino-compatible) boards that bring together key elements needed for most data loggers, in arrangemets that support low-power field deployments.

Here is a video introducing the cadalogger project.  We're hoping to develop better documentation, including videos with better production quality, over the next few years!

[![A quick intro to cadalogger](https://img.youtube.com/vi/7qkBjCcAluY/0.jpg)](https://www.youtube.com/watch?v=7qkBjCcAluY)


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


## Functions

    cadalogger(byte boardVersion);
    void initialise();
    void initialise(int heartbeat);
    void flash(byte flashes);
    void rest(int cycles);
    void update_time();
    void power_down_sd();
    void power_down_sd(bool wait);
    void power_up_sd();
    void go_to_sleep_until_RTC_wake();
    void enable_watchdog();
    void disable_watchdog();
    void feed_watchdog();
    void print_time();
    void write_time_to_rtc();
    double rtc_temp();
    
cadalogger(byte boardVersion); 

- constructor
- boardVersion=CADALOGGERMINI for small surface mount board
- boardVersion=CADALOGGERMAXI for hand-assembled largely through hole version

void initialise();
void initialise(int heartbeat);

- initialise with or without heartbeat
- sets pin states to minimise power loss, sets up RTC
- heartbeat is interval in seconds for a periodic RTC interrupt, and is used to allow sleep modes with regular wake-up
- arbitrary heartbeat intervals are allowed on if boardVersion==CADALOGGERMINI, for boardVersion==CADALOGGERMINI, 5s is currently required



## Examples

See the /examples/ director for code for each example.  Here is some further context for each.

### Blink: the 'Hello world!' of microcontrollers

[instructions on how to get blink working]



### Sunshine: the 'Hello world!' of environmental data loggers

Here we list two simple examples for getting started.  More varied examples are detailed here.

*sunshine.ino* A simple data logger example demonstrating several power-saving tricks including:

- deep sleep
- depowering the SD card
- powering sensor circuits with a MCU pin
- saving data from multiple data reads between bulk writes to the SD card


### Templogger: from 'sunshine', it isn't too far to other programs

Temperature 


### Temperature logger with ultrasonic distance (river stage and temperature)



### Temperature logger with satellite data transmission




