// cadalogger.h
// Michael Morrissey and Rebecca Nagel

#ifndef LOGGER_H
#define LOGGER_H


#include "Arduino.h" 
//#include "build_defs.h"   // for setting date/time from computer system time


#define CADALOGGERMINI 0
#define CADALOGGERMAXI 1

#include <SPI.h>
#include <Wire.h>
//#include <SD.h>   // n.b. Sandisk 32Gb cards do not sleep with megacorex's SD
#include <SdFat.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

class cadalogger{
  public:
    cadalogger(byte boardVersion);
    void initialise();
    void initialise(int heartbeat);
    void flash(byte flashes);
    void rest(int cycles);
    void update_time();
    void power_down_sd();
    void power_up_sd();
    void go_to_sleep_until_RTC_wake();
    void enable_watchdog();
    void disable_watchdog();
    void feed_watchdog();
    void print_time();
    void write_time_to_rtc();
    double rtc_temp();
    byte time[6];
    SdFat SD;
  private:
    static void _RTC_interrupt();
    void _write_time_to_RV3032();  // Michael's board RTC functions
    void _update_time_RV3032();
    void _prepare_rtc_RV3032();
    double _rtc_temp_RV3032();
    void _write_time_to_DS3231();  // Rebecca's board RTC functions
    void _update_time_DS3231();
    void _prepare_rtc_DS3231();
    double _rtc_temp_DS3231();
    byte _board_version;
    byte _DecToBcd(byte val);
    byte _BcdToDec(byte val);
    void _prepare_rtc();
    byte _uSD_SCL_pin;
    byte _uSD_MISO_pin;
    byte _uSD_MOSI_pin;
    byte _uSD_SS_pin;
    byte _uSD_power_pin;
    byte _LED_pin;
    byte _RTC_int_pin;
    int _heartbeat_interval_s;   // Rebecca's board will probably need to just do minutes and seconds?
};

#endif
