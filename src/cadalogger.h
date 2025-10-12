// cadalogger.h
// Michael Morrissey and Rebecca Nagel
// 03 Sept 2025

#ifndef LOGGER_H
#define LOGGER_H


#include "Arduino.h" 


#define CADALOGGERMINI 0
#define CADALOGGERMICKY 1

#include <SPI.h>
#include <Wire.h>
#include <SdFat.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

class cadalogger{
  public:
    cadalogger(byte boardVersion);
    void initialise();
    void flash(byte flashes);
    void rest(int cycles);
    void update_time();
    void write_time_to_RTC();
    void setHeartBeat(int interval); 
    void sleepToRTCwake(); 
    void sleepFor(int seconds);
    byte wakeOnSecondMultiple(int s);
    byte wakeOnMinuteMultiple(int s);
    void write_time_to_rtc();
    void power_down_sd();
    void power_down_sd(bool wait);
    void power_up_sd();
    void enable_watchdog();
    void disable_watchdog();
    void feed_watchdog();
    void print_time();
    double rtc_temp();
    float supply_voltage();
    byte time[6];
    SdFat SD;
    byte LED_pin;
    
    static void RTCwake();
    
  private:
    void _sleepForMsRV3032(uint16_t ms);
    void _setHeartBeatFast(int interval);
    void _write_time_to_RV3032();  
    void _update_time_RV3032();
    void _prepare_rtc_RV3032();
    double _rtc_temp_RV3032();
    void _write_time_to_DS3231();  
    void _update_time_DS3231();
    double _rtc_temp_DS3231();
    byte _DecToBcd(byte val);
    byte _BcdToDec(byte val);
    byte _getSecond();
    byte _getMinute();
    void _sleepToDS3231(int s);        // sleeps to a second match, does not do RTC resets
    void _sleepSeconds_DS3231(byte s); // sleeps s seconds, does not do RTC resets
    void _sleepForDS3231(int seconds); // sleep seconds seconds, with 5s WDT resets
    void _sleepForRV3032(int seconds);

    byte _board_version;
    byte _uSD_SCL_pin;
    byte _uSD_MISO_pin;
    byte _uSD_MOSI_pin;
    byte _uSD_SS_pin;
    byte _uSD_power_pin;
    byte _RTC_int_pin;
};

#endif
