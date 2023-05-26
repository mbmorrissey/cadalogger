
#include "cadalogger.h"

// cadalogger env(CADALOGGERMINI); // surfacemount board
cadalogger env(CADALOGGERMAXI);    // through-hole board

File file;                            // cadalogger loads SdFat

// Thermometer ------------------------------------------------------------------------------
#define tempHum 0x44                  // I2C address of the SEN temperature and humidity sensor

// MOISTure sensor --------------------------------------------------------------------------
#define chirp 0x20                    // I2C address of the CHIRP moisture sensor

// PHOTOtransistor --------------------------------------------------------------------------
#define PHOTO_pin 24                  // phototransistor connected to ATmega on this pin
#define PHOTO_pwr_pin 25

// function to compactify writing time/date to SD card
void write_time() {
  file.print(env.time[0]); file.print(",");
  file.print(env.time[1]); file.print(",");
  file.print(env.time[2]); file.print(",");
  file.print(env.time[3]); file.print(",");
  file.print(env.time[4]); file.print(",");
  file.print(env.time[5]); file.print(",");
}

void setup() {
  env.initialise();
  Serial.begin(9600);

  // turn on pins needed
  pinMode(PHOTO_pin, INPUT);               // photoresistor
  pinMode(PHOTO_pwr_pin, OUTPUT);
  digitalWrite(PHOTO_pwr_pin, LOW);

  // set up temperature and humidity sensor
  Wire.beginTransmission(tempHum);
  Wire.write(0x00);                        // reset device
  Wire.endTransmission();

  // set up moisture sensor
  Wire.beginTransmission(chirp);
  Wire.write(0x06);                        // reset device
  Wire.endTransmission();

  Wire.beginTransmission(chirp);           // sleep device
  Wire.write(0x08);
  Wire.endTransmission();
}

void loop() {
  env.flash(2);

  // Get time ------------------------------------------------------------------------------
  env.update_time();
  Serial.print(env.time[0]); Serial.print("\t");
  Serial.print(env.time[1]); Serial.print("\t");
  Serial.print(env.time[2]); Serial.print("\t");
  Serial.print(env.time[3]); Serial.print("\t");
  Serial.print(env.time[4]); Serial.print("\t");
  Serial.println(env.time[5]);

  // THERMOMETER ----------------------------------------------------------------------------
  // max measurement duration is 6 ms at medium repeatability and 15 ms at high repeatability
  Wire.beginTransmission(tempHum);
  Wire.write(0x24);                // clock streching disabled (pg 10)
  Wire.write(0x0B);                // medium repeatability
  Wire.endTransmission();

  delay(20);

  byte buffer[6];

  Wire.requestFrom(tempHum, 6);
  for (int i = 0; i < 6; i++) buffer[i] = Wire.read();
  Wire.endTransmission();

  uint16_t rawTemperature = (buffer[0] << 8) + buffer[1];
  uint16_t rawHumidity = (buffer[3] << 8) + buffer[1];

  // conversions on pg 13
  float SEN0385Temp = rawTemperature * (175.0 / 65535) - 45;
  float SEN0385Hum = rawHumidity * (100.0 / 65535);

  // MOISTure sensor ------------------------------------------------------------------------
  Wire.beginTransmission(chirp);
  Wire.write(0x00);                // capacitance
  Wire.endTransmission();

  delay(20);

  Wire.requestFrom(chirp, 2);
  unsigned int CHIRPcap = Wire.read() << 8;
  CHIRPcap = CHIRPcap | Wire.read();
  Wire.beginTransmission(chirp);   // put device to sleep
  Wire.write(0x08);
  Wire.endTransmission();

  // PHOTOTRANSISTOR ------------------------------------------------------------------------
  digitalWrite(PHOTO_pwr_pin, HIGH);
  delay(100);

  int sensorLight = analogRead(PHOTO_pin);             // read analog value of phototransmitter
  float voltage = sensorLight * (3.3 / 1023.0);        // analog-to-digital converter: convert 0-1023 reading to 0-3.3v voltage

  delay(100);
  digitalWrite(PHOTO_pwr_pin, LOW);

  // print to serial monitor ----------------------------------------------------------------
  Serial.print("CHIRP Capacitance = ");
  Serial.println(CHIRPcap);

  Serial.print("SEN Temperature = ");
  Serial.print(SEN0385Temp);
  Serial.println("°C");
  Serial.print("SEN Humidity = ");
  Serial.print(SEN0385Hum);
  Serial.println(" %");

  Serial.print("light raw = "); Serial.print(sensorLight);
  Serial.print(" | light V = "); Serial.println(voltage);

  Serial.println();

  delay(10);

  // print to microSD card ------------------------------------------------------------------
  // 1. wake up SD card
  env.power_up_sd();

  // 2. open the file
  file = env.SD.open("env.csv", FILE_WRITE);

  // 3a. if file is avilable, write to it
  if (file) {
    env.update_time();
    write_time();

    file.print(CHIRPcap);
    file.print(",");

    file.print(SEN0385Temp);
    file.print(",");
    file.print(SEN0385Hum);
    file.print(",");

    file.print(sensorLight);
    file.print(",");
    file.println(voltage);

    file.flush();
    file.close();
  }

  // 3b. if the file doesn't open, give an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  // 4. shut down sd card
  env.power_down_sd();


  // put board to sleep ----------------------------------------------------------------------
  env.go_to_sleep_until_RTC_wake();

}
