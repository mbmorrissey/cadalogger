// demonstration of a light level sensing data logger to demonstrate cadalogger boards
// Michael Morrissey and Rebecca Nagel


#include "cadalogger.h"

cadalogger sunshine(CADALOGGERMINI);
// cadalogger sunshine(CADALOGGERMAXI);  // change to this for through-hole board

#define light_power_pin 22     // to pin-power light-sensing circuit
#define light_analog_pin A0    // analog pin to read light level
#define light_pin_digital 12   // digital alias of analog pin

File file;                               // cadalogger loads SdFat

#define cycles_per_read = 12;     // 5 second intervals between reads

int lightValue;   // store values of accumulated reads

// function to compactify writing time/date to SD card
void write_time(){
  file.print(sunshine.time[0]); file.print("\t");
  file.print(sunshine.time[1]); file.print("\t");
  file.print(sunshine.time[2]); file.print("\t");
  file.print(sunshine.time[3]); file.print("\t");
  file.print(sunshine.time[4]); file.print("\t");
  file.print(sunshine.time[5]);
}

void setup() {
  // set up the logger with the RTC firing an interrupt ever 5 seconds
  sunshine.initialise();
  // logger will restart if the watchdog timer isn't reset every 8 and a bit seconds
  beamlogger.enable_watchdog();

  // de-power the light sensing circuit
  pinMode(light_power_pin,OUTPUT);
  digitalWrite(light_power_pin,LOW);
}

void loop() {

  // power on and set appropriate pin to input
  digitalWrite(light_power_pin,HIGH);                 // power to light-sensing circuit
  pinMode(light_pin_digital,INPUT);                   // set pin to input 
  delay(10);                                          // let voltage stabilise

  // take the reading
  lightValue = analogRead(light_analog_pin);   // read value from simple light circuit, discard first read
  lightValue = analogRead(light_analog_pin);

  // turn off power and set pin states to avoid power leaks
  pinMode(light_pin_digital,OUTPUT);           // set pin A0 (digital pin 12) to ground to avoid power leaks
  digitalWrite(light_pin_digital,LOW);
  digitalWrite(light_power_pin,LOW);           // cut power to light-sensing circuit

  // wake uSD card, write data, and shut things down again
  sunshine.power_up_sd();
  file = sunshine.SD.open("sunshine.csv", FILE_WRITE);
  sunshine.update_time();
  write_time();
  file.println(lightValue);
  file.flush();
  file.close();
  sunshine.power_down_sd();

  // sleep until it is time to take another light level reading
  sunshine.wakeOnSecondMultiple(60);

}
