//
//    FILE: DS18B20_simple.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: equivalent of DallasTemperature library Simple
//     URL: https://github.com/RobTillaart/DS18B20_RT

//#include <OneWire.h>
//#include <DS18B20.h>
#include "cadalogger.h"

cadalogger templogger(CADALOGGERMINI);

//#define ds18b20_power_pin 4
//#define ONE_WIRE_BUS              19
//OneWire oneWire(ONE_WIRE_BUS);
//DS18B20 sensor(&oneWire);

File file;    // file object to record events
char df[80];  // data file name (to include date)


void write_time(){
  file.print(templogger.time[0]); file.print(",");
  file.print(templogger.time[1]); file.print(",");
  file.print(templogger.time[2]); file.print(",");
  file.print(templogger.time[3]); file.print(",");
  file.print(templogger.time[4]); file.print(",");
  file.print(templogger.time[5]);
}

void setup(void){
  templogger.initialise(10);  // initialise and set up watchdog
  Serial.begin(9600);
//  pinMode(ds18b20_power_pin,OUTPUT);
//  digitalWrite(ds18b20_power_pin,HIGH);
//  sensor.begin();
}

void loop(void){
//  digitalWrite(ds18b20_power_pin,HIGH);
//  digitalWrite(ONE_WIRE_BUS,HIGH);
//  delay(10);
//  sensor.begin();
//  sensor.requestTemperatures();
//  while (!sensor.isConversionComplete());  // wait until sensor is ready
//  float temp = sensor.getTempC();
//  digitalWrite(ds18b20_power_pin,LOW);
//  digitalWrite(ONE_WIRE_BUS,LOW);
//  Serial.println(temp);

  templogger.power_up_sd();
  file = templogger.SD.open("templog.csv", FILE_WRITE);
  templogger.update_time();
  write_time();
//  file.print(",");
//  file.println(temp);
  file.println("");
  file.flush();
  file.close();
  templogger.power_down_sd();

  templogger.go_to_sleep_until_RTC_wake();
  templogger.flash(2);
}