// demonstration of a light level sensing data logger to demonstrate cadalogger boards
// Michael Morrissey and Rebecca Nagel


#include "cadalogger.h"

cadalogger wind(CADALOGGERMINI);
// cadalogger sunshine(CADALOGGERMAXI);  // change to this for through-hole board



#define MESAGE_DISCARD 3          // how many messages to discard when a bout of reading starts
#define MAX_MESSAGE 50            // maximum message length in characters

#define power_pin_5V  18          // (aka A6) enable pin on 5V boost converter 
#define power_pin_3V  23          // (aka A9, not broken out) low-side mosfet switch on pin MAX3232 module
#define on_switch 19              // (aka A7) switch to determine if user wants to logger to be running
#define batt_volt_pin A0

File file;                        // cadalogger loads SdFat
char df[80];                      // current data file name

byte discarded_messages = 0;      // keep track of how many messages have been discarded
byte last_hour;                   // needed to tell if new data file needs to be started
char buffer[MAX_MESSAGE];         // contains current message
float batt_v;                     // battery voltage


byte logger_state;  // 1 = collecting data (continuous or intermittent), 0 = idle (e.g., for uSD to be removed)

// function to compactify writing time/date to SD card
void write_time(){
  file.print(wind.time[0]); file.print(",");
  file.print(wind.time[1]); file.print(",");
  file.print(wind.time[2]); file.print(",");
  file.print(wind.time[3]); file.print(",");
  file.print(wind.time[4]); file.print(",");
  file.print(wind.time[5]);
}

void setup() {
  wind.initialise(5);    // 5 second hearbeat so watchdog can run with resets through sleep cycles
  Serial.begin(9600);    // uncomment for debugging

  byte discarded_messages = 0;
  while (discarded_messages<MESAGE_DISCARD){
    while (Serial.available() > 0) {
      delay(5);
      char inch = Serial.read();
    }
    discarded_messages++;
  }
  
  wind.update_time();
  last_hour = wind.time[2];
  sprintf(df, "wind_dat_%d_%d_%d.csv", wind.time[2], wind.time[3], wind.time[4]);

  pinMode(on_switch, INPUT);
  logger_state = digitalRead(on_switch);

  if(logger_state==1){
    wind.power_up_sd();
    file.open(df,FILE_WRITE);
    // turn on 5V circuit

    // turn on 3.3V circuit

  }else{
    // turn off 5V circuit

    // turn off 3.3V circuit

  }

  pinMode(batt_volt_pin,INPUT);
}

void loop() {

  // if logger is running but switch has been turned off
  if(logger_state==1 & digitalRead(on_switch)==0){
    file.close();
    wind.power_down_sd();
    logger_state = 0;
    // turn off 5V circuit

    // turn off 3.3V circuit

  }

  // if logger is not running but switch has been turned on
  if(logger_state==0 & digitalRead(on_switch)==1){
    wind.power_up_sd();
    wind.update_time();
    // turn on 5V circuit

    // turn on 3.3V circuit

    last_hour = wind.time[2];
    sprintf(df, "wind_dat_%d_%d_%d.csv", wind.time[2], wind.time[3], wind.time[4]);
    file.open(df,FILE_WRITE);
    logger_state = 1;   

    byte discarded_messages = 0;
      while (discarded_messages<MESAGE_DISCARD){
        while (Serial.available() > 0) {
          delay(5);
          char inch = Serial.read();
        }
      discarded_messages++;
    }
  }


  if(logger_state==1){ // if running (either in continuous or intermittent low power mode)

    int sensorValue = analogRead(batt_volt_pin);
    batt_v = sensorValue * 2.0 * 3.3 / 1023.0;
    if(batt_v > bat_threshold_v){
      logger_power_mode = 1;
    }else{
      logger_power_mode = 0;
    }

    byte message = 0;
    if(Serial.available()>0){
      wind.update_time();
      delay(30);
      byte index = 0;
      while(Serial.available()>0){
        char inch = Serial.read();
        if (index < MAX_MESSAGE - 1) {
          buffer[index] = inch;
          index++;
        }
        delay(3);           // let the message fully come in
      }
      wind.flash(1);      // heartbeat
      message++;
    }

    // if the hour has changed, close previous hour's data file and start a new one
    if(wind.time[2] != last_hour){
      last_hour = wind.time[2];
      file.close();
      sprintf(df, "wind_dat_%d_%d_%d.csv", wind.time[2], wind.time[3], wind.time[4]);
      file.open(df,FILE_WRITE);
    }

    if(message > 0){
      wind.feed_watchdog();
      if(logger_power_mode == 1) message = 0;  // if power is abundant, don't let variable 'message'
                                               // ever get anywhere near the threshold (i.e.,
                                               // 'power_save_message_num') for going to sleep
      write_time_batV_message();
    }

  }else{ // end if for logger operation
    wind.feed_watchdog();
  }

  if(message > power_save_message_num){  // if power is getting low, 'message' is allowed to increment
                                         // to a point where we go to sleep for a winle between bursts
                                         // of recording data
    message = 0;

    cut_power();
    sleep_with_led_heartbeat();
    restore_power()
  }

}

void cut_power(){
    // shut down power in 5V circuit

    // shut down power to MAX3232 module

}

void restore_power(){
    // restore power in 5V circuit

    // restore power to MAX3232 module

}

void sleep_with_led_heartbeat(){
  for(int i = 0 i < power_save_sleep_cycles; i++){
    wind.feed_watchdog(); 
    wind.go_to_sleep_until_RTC_wake();
    wind.flash(2);
  }
}

void write_time_batV_message(){
  write_time();
  file.write(",");
  file.print(batt_v);
  file.write(",");
  for (int i = 0; i < MAX_MESSAGE; i++) { // write buffer to file and erase as we go
    file.write(buffer[i]); 
    buffer[i] = 0;
  }
}