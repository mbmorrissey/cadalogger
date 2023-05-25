/*

  Rebecca Nagel
  Last modified: 24 May 2023

  cataloger
  board components:
  - designed PCB, 12 May 2023
  - ATMEGA4809 (ATMEGA4809-PF)
  - microSD™ Surface Mount (MEM2075)
  - Linear Voltage Regulator (MCP1700-3302E/TO)
  - Voltage Feedback Amplifier (OPA357AIDBVT)
  - Real Time Clock (RTC) IC Clock/Calendar (DS3231M+)
  - Power Switch (TPS2021P)

  sensors:
  - SHT31 Temperature & Humidity sensor
  - CHIRP Capacitive Soil Moisture & Light sensor
  - TEPT5700 Photoresistor, 570 nm
*/




/*
  Initial SETUP ------------------------------------------------------------------------------
*/

// DS3231 RTC -------------------------------------------------------------------------------
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#define rtc 0x68                        // I2C address of the DS3231
#define RTC_int_pin 28                  // RTC connected to ATmega on this pin

int int_detect = 0;

// get time of build from __TIME__ function, which is in format HH:MM:SS
#define BUILD_HOUR (((__TIME__)[0] - '0') * 10 + (__TIME__)[1] - '0')
#define BUILD_MINUTE ((__TIME__[3] - '0') * 10 + (__TIME__)[4] - '0')
#define BUILD_SECOND  ((__TIME__[6] - '0') * 10 + (__TIME__)[7] - '0')

// microSD card reader ----------------------------------------------------------------------
#include <SPI.h>
#include "SdFat.h"
SdFat SD;

#define SCK_pin 10
#define MISO_pin 9
#define MOSI_pin 8
#define SS_pin 11                        // SS pin on ATmega that connects to CMD pin on microSD card reader
#define SD_power_pin 30                  // power switch EN pin to MCU

// LED --------------------------------------------------------------------------------------
#define LED_pin 31                     // LED connected to ATmega on this pin

// Thermometer ------------------------------------------------------------------------------
#include "SHT31.h"

#define SHT31_ADDRESS   0x44
uint32_t start;
uint32_t stop;
SHT31 sht;

// PHOTOtransistor --------------------------------------------------------------------------
#define PHOTO_pin 24                  // phototransistor connected to ATmega on this pin
#define PHOTO_pwr_pin 25

// MOISTure sensor --------------------------------------------------------------------------
#include "I2CSoilMoistureSensor.h"

I2CSoilMoistureSensor chirp;



/*
  FUNCTIONS ----------------------------------------------------------------------------------
*/

// convert BCD to decimal
byte bcdTOdec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

// convert decimal to BCD
byte decTObcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}

// interrupt flag
static void RTC_interrupt() {
  //  int_detect=1;
}

// functions to cut power to SD card
// see: https://thecavepearlproject.org/2017/05/21/switching-off-sd-cards-for-low-power-data-logging/
void wake_up_sd() {
  pinMode(SS_pin, OUTPUT);
  digitalWrite(SS_pin, HIGH);      // pull up the CS pin on the SD card

  pinMode(MOSI_pin, OUTPUT);
  digitalWrite(MOSI_pin, HIGH);    // pull up the MOSI pin on the SD card

  pinMode(MISO_pin, INPUT_PULLUP); // pull up the MISO pin on the SD card

  pinMode(SCK_pin, OUTPUT);
  digitalWrite(SCK_pin, LOW);      // pull down the SCK pin on the SD card

  digitalWrite(SD_power_pin, LOW); // uSD on

  delay(10);

  SPI.swap(1);
  SPI.begin();                     // initialize microSD card
  SD.begin(SS_pin);
}


void shut_down_sd() {
  delay(1000);                      // delay for 1 sec after the file has been opened & written to

  SPI.end();                        // before shutting down the SPI interface
  SPI.swap(0);

  pinMode(SCK_pin, INPUT);          // and disable pins on SD card
  pinMode(MISO_pin, INPUT);
  pinMode(MOSI_pin, INPUT);
  pinMode(SS_pin, INPUT);

  digitalWrite(SD_power_pin, HIGH); // power off uSD
}

// functions to set time on RTC
void setTime(byte second, byte minute, byte hour, byte dayOfMonth, byte month, byte year) {
  Wire.beginTransmission(rtc);
  Wire.write(0x00); // moves register pointer to 00h
  Wire.write(decTObcd(second));     // set seconds
  Wire.write(decTObcd(minute));     // set minutes
  Wire.write(decTObcd(hour));       // set hours
  Wire.endTransmission();
  Wire.beginTransmission(rtc);
  Wire.write(0x04); // moves register pointer to 04h
  Wire.write(decTObcd(dayOfMonth)); // set date
  Wire.write(decTObcd(month));      // set month
  Wire.write(decTObcd(year));       // set year
  Wire.endTransmission();
}



/*
  SETUP --------------------------------------------------------------------------------------
*/

void setup() {

  //   set all pins to pullup - reduces power drain on unused pins
  for (byte i = 0; i < 33; i++) {
    pinMode(i, INPUT_PULLUP);
  }

  // initalize serial monitor @ 9600 characters per second
  Serial.begin(9600);
  Wire.begin();

  // turn on pins needed
  pinMode(LED_pin, OUTPUT);                // LED
  digitalWrite(LED_pin, LOW);

  pinMode(SD_power_pin, OUTPUT);           // uSD card

  pinMode(PHOTO_pin, INPUT);               // photoresistor
  pinMode(PHOTO_pwr_pin, OUTPUT);
  digitalWrite(PHOTO_pwr_pin, LOW);

  // start up the real time clock
  // set an intial time on RTC :: seconds, minutes, hours, date, month, year

//  setTime(BUILD_SECOND, BUILD_MINUTE, BUILD_HOUR, 25, 05, 23);
//  Serial.print("build time was ");
//  Serial.print(BUILD_HOUR);
//  Serial.print(":");
//  Serial.print(BUILD_MINUTE);
//  Serial.print(":");
//  Serial.println(BUILD_SECOND);

  // set up RTC & interrupt sequence
  Wire.beginTransmission(rtc);
  Wire.write(0x0F);                  // address: status register
  Wire.write(0b00000000);            // set flag from alarm1 to 0
  Wire.endTransmission();

  Wire.beginTransmission(rtc);
  Wire.write(0x0E);                  // address: control register (oscillator)
  Wire.write(0b00000101);            // enable interupt (INTCN) and alarm1 (A1lE)
  Wire.endTransmission();

  Wire.beginTransmission(rtc);
  Wire.write(0x07);                  // address: start at row alarm1 seconds
  Wire.write(0b00000000);            // set alarm on 07 - A1M1
  Wire.write(0b10000000);            // set alarm on 08 - A1M2
  Wire.write(0b10000000);            // set alarm on 09 - A1M3
  Wire.write(0b10000000);            // set alarm on 0A - A1M4
  Wire.endTransmission();
  // set byte 7 on A1M1, A1M2, A1M3, A1M4 to 1 = 1 second alarm
  // set byte 7 on A1M1 to 0 & A1M2, A1M3, A1M4 to 1 = 1 minute alarm


  // set up temperature and humidity sensor
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();


  // set up moisture sensor
  chirp.begin(); // reset sensor
  delay(1000); // give some time to boot up

}



/*
  WORK ---------------------------------------------------------------------------------------
  the loop function runs over and over again forever
*/
void loop() {

  digitalWrite(LED_pin, HIGH);              // blink LED
  delay(50);
  digitalWrite(LED_pin, LOW);

  pinMode(RTC_int_pin, INPUT);              // set RTC pin as input

  // read time from RTC, i.e. time since turning logger on / uploading program
  Wire.beginTransmission(rtc);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(rtc, 7);   // read from RTC as BCD
  byte second = bcdTOdec(Wire.read());
  byte minute = bcdTOdec(Wire.read());
  byte hour = bcdTOdec(Wire.read());
  byte weekday = Wire.read();
  byte dayOfMonth = bcdTOdec(Wire.read());
  byte month = bcdTOdec(Wire.read());
  byte year = bcdTOdec(Wire.read());

  delay(10);


  // THERMOMETER ----------------------------------------------------------------------------
  sht.read();         // default = true/fast       slow = false
  float SENtemp = sht.getTemperature();
  float SENhum = sht.getHumidity();

  delay(50);


  // MOISTure sensor ------------------------------------------------------------------------
  while (chirp.isBusy()) delay(50);
  float CHIRPmoist = chirp.getCapacitance();
  float CHIRPtemp = chirp.getTemperature() / (float)10;
  float CHIRPlight = chirp.getLight(true);
  chirp.sleep();

  // PHOTOTRANSISTOR ------------------------------------------------------------------------
  digitalWrite(PHOTO_pwr_pin, HIGH);
  delay(100);

  int sensorLight = analogRead(PHOTO_pin);             // read analog value of phototransmitter
  float voltage = sensorLight * (3.3 / 1023.0);        // analog-to-digital converter: convert 0-1023 reading to 0-3.3v voltage

  delay(100);
  digitalWrite(PHOTO_pwr_pin, LOW);


  // print to serial monitor ----------------------------------------------------------------
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(second);
  Serial.print(" ");
  Serial.print(dayOfMonth);
  Serial.print("/");
  Serial.print(month);
  Serial.print("/");
  Serial.println(year);

  Serial.print("CHIRP Capacitance = ");
  Serial.println(CHIRPmoist);

  Serial.print("SEN Temperature = ");
  Serial.print(SENtemp, 1);
  Serial.println("°C");
  Serial.print("SEN Humidity = ");
  Serial.print(SENhum, 1);
  Serial.println(" %");

  Serial.print("light raw = "); Serial.print(sensorLight);
  Serial.print(" | light V = "); Serial.println(voltage);

  Serial.println();

  delay(10);

  // print to microSD card ------------------------------------------------------------------
  // 1. wake up SD card
  wake_up_sd();

  // 2. open the file
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // 3a. if file is avilable, write to it
  if (dataFile) {
    dataFile.print(hour);
    dataFile.print(":");
    dataFile.print(minute);
    dataFile.print(":");
    dataFile.print(second);
    dataFile.print(" ");
    dataFile.print(dayOfMonth);
    dataFile.print("/");
    dataFile.print(month);
    dataFile.print("/");
    dataFile.print(year);
    dataFile.print(",");

    dataFile.print(CHIRPmoist);
    dataFile.print(",");

    dataFile.print(SENtemp);
    dataFile.print(",");
    dataFile.print(SENhum);
    dataFile.print(",");

    dataFile.print(sensorLight);
    dataFile.print(",");
    dataFile.println(voltage);

    dataFile.flush();
    dataFile.close();
  }

  // 3b. if the file doesn't open, give an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  // 4. shut down sd card
  shut_down_sd();


  // put board to sleep ----------------------------------------------------------------------
  cli();                                    // disable / "clear" interrupt flag
  // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(RTC_int_pin), RTC_interrupt, FALLING);
  sei();                                    // set interrupt flag

  // sleeps here until the interrupt V falls
  sleep_cpu();
  sleep_disable();

  Wire.beginTransmission(rtc);       // reset alarm counter
  Wire.write(0x0F);                  // address: status register
  Wire.write(0b00000000);            // set flag from alarm 1 to 0
  Wire.endTransmission();

}
