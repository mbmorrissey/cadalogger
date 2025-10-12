#include "cadalogger.h"
#include "SHT31.h"

cadalogger mth(1);
SHT31 sht;

#define SHT31_ADDRESS   0x44
#define moistureSensPin A0
#define moistureGndPin A1

File file;    // file object to record events
char df[80];  // data file name (to include date)

void setup()
{
  mth.initialise();
  
  pinMode(moistureSensPin,INPUT);
  pinMode(moistureGndPin,OUTPUT);
  digitalWrite(moistureGndPin,HIGH);

  Wire.begin();
  sht.begin();
}


void loop()
{
  sht.read();         

  digitalWrite(moistureGndPin,LOW);
  delay(100);
  int adc = analogRead(moistureSensPin);

  digitalWrite(moistureGndPin,HIGH);
  
  mth.update_time();
  mth.power_up_sd();
  sprintf(df, "mthlog_%d_%d.csv", mth.time[3], mth.time[4]);
  file = mth.SD.open(df, FILE_WRITE);
  write_time();
  file.print(",");
  file.print(adc);
  file.print(",");
  file.print(sht.getTemperature());
  file.print(",");
  file.print(sht.getHumidity());
  file.print(",");
  file.println(mth.supply_voltage());
  file.flush();
  file.close();
  mth.power_down_sd();

  mth.wakeOnSecondMultiple(60);
}

// compactly writes six fields of date and time to data file
void write_time() {
  file.print(mth.time[0]);
  file.print(",");
  file.print(mth.time[1]);
  file.print(",");
  file.print(mth.time[2]);
  file.print(",");
  file.print(mth.time[3]);
  file.print(",");
  file.print(mth.time[4]);
  file.print(",");
  file.print(mth.time[5]);
}
