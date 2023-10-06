// demonstration of setting time on cadalogger board
// Michael Morrissey and Rebecca Nagel


#include "cadalogger.h"
#include "build_defs.h"

File file;

cadalogger timelogger(CADALOGGERMINI);
// cadalogger timelogger(CADALOGGERMAXI);  // change to this for through-hole board

long last_msg = 0;

// function to compactify writing time/date to serial
void print_time(){
  Serial.print(timelogger.time[5]); Serial.print("/");
  Serial.print(timelogger.time[4]); Serial.print("/");
  Serial.print(timelogger.time[3]); Serial.print(" ");
  Serial.print(timelogger.time[2]); Serial.print(":");
  Serial.print(timelogger.time[1]); Serial.print(":");
  Serial.print(timelogger.time[0]);
}

void setup() {
  delay(100);
  timelogger.initialise();

  pinMode(19, INPUT_PULLUP);   // that's A7 on 32 pin 4808

  if(digitalRead(19)==0){
    timelogger.time[0] = ((int)BUILD_SEC_CH0 - '0')*10+((int)BUILD_SEC_CH1 - '0');
    timelogger.time[1] = ((int)BUILD_MIN_CH0 - '0')*10+((int)BUILD_MIN_CH1 - '0');
    timelogger.time[2] = ((int)BUILD_HOUR_CH0 - '0')*10+((int)BUILD_HOUR_CH1 - '0');
    timelogger.time[3] = ((int)BUILD_DAY_CH0 - '0')*10+((int)BUILD_DAY_CH1 - '0');
    timelogger.time[4] = ((int)BUILD_MONTH_CH0 - '0')*10+((int)BUILD_MONTH_CH1 - '0');
    timelogger.time[5] = ((int)BUILD_YEAR_CH2 - '0')*10+((int)BUILD_YEAR_CH3 - '0');
    timelogger.write_time_to_rtc();
  }


  
  Serial.begin(9600);
  Serial.println("1: decrease seconds, 2: increase seconds (press enter)");
}

void loop() {
  
  // keep up an output of the current time according to the RTC
  if(millis() > last_msg+1000){
    timelogger.update_time();
    print_time(); Serial.println();
    Serial.flush();
    last_msg = millis();
  }

  // increment seconds or minutes, taking care not to make values
  // greater than 59 or less than zero.  Seconds increment by two
  // because a second or so is likely to pass in any event whn
  // incrementing forward
  if(Serial.available()){
    char rec = Serial.read();
    timelogger.update_time();
    switch(rec){
      case '1':
        if(timelogger.time[0]>0){
          timelogger.time[0]=timelogger.time[0]-1;
        }
        break;
      case '2':
        if(timelogger.time[0]<57){
          timelogger.time[0]+=2;
        }
        break;
      case '3':
        if(timelogger.time[1]>0){
           timelogger.time[1]=timelogger.time[1]-1;
        }
        break;
      case '4':
        if(timelogger.time[1]<58){
          timelogger.time[1]++;
        }
        break;
    }
    timelogger.write_time_to_rtc();
  }
  
}
