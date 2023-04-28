// demonstration of a light level sensing data logger to demonstrate cadalogger boards
// Michael Morrissey and Rebecca Nagel


#include "cadalogger.h"

cadalogger sunshine(CADALOGGERMINI);
// cadalogger sunshine(CADALOGGERMAXI);  // change to this for through-hole board

#define light_power_pin 22     // to pin-power light-sensing circuit
#define light_analog_pin A0    // analog pin to read light level
#define light_pin_digital 12   // digital alias of analog pin

File file;                               // cadalogger loads SdFat

const int cycles_per_read = 3;     // 10 second intervals between reads
const int reads_per_write = 2;     // number of reads to accumulate between each write to SD card

int reads = 0;                     // number of accumulated reads
int lightValue[reads_per_write];   // store values of accumulated reads

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
  sunshine.initialise();
//  Serial.begin(9600);    // uncomment for debugging

  pinMode(light_power_pin,OUTPUT);
  digitalWrite(light_power_pin,LOW);
}

void loop() {

  // power
  digitalWrite(light_power_pin,HIGH);                 // power to light-sensing circuit
  pinMode(light_pin_digital,INPUT);                   // set pin to input 
  delay(10);                                          // let voltage stabilise
  lightValue[reads] = analogRead(light_analog_pin);   // read value from simple light circuit, discard first read
  lightValue[reads] = analogRead(light_analog_pin);
//  Serial.println(lightValue); Serial.flush();       // for debugging/seeing data
 
  pinMode(light_pin_digital,OUTPUT);           // set pin A0 (digital pin 12) to ground to avoid power leaks
  digitalWrite(light_pin_digital,LOW);
  digitalWrite(light_power_pin,LOW);           // cut power to light-sensing circuit

  reads++;                                     // increment reads
  
  if(reads >= reads_per_write){                // write data if sufficient reads have acumulated
    reads = 0;
    sunshine.power_up_sd();
    file = sunshine.SD.open("sunshine.csv", FILE_WRITE);
    sunshine.update_time();
    write_time();
    for(int w=0; w<reads_per_write; w++){
      file.println(lightValue[w]);
    }
    file.flush();
    file.close();
    sunshine.power_down_sd();
  }
  for(int i = 0; i<cycles_per_read; i++){      // sleep for the appropriate number of cycles
    sunshine.go_to_sleep_until_RTC_wake();
    sunshine.flash(2);                         // let the world know the logger is working
  }
}
