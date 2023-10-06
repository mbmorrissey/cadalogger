/*

SENSORS:
  JSN-SR04T-V3.0 Ultrasonic Sensor - Mode 2
  - Has function to (1) record distance x 14, (2) discard first two readings and the max and min values, (3) return average of 10 remaining values
  
  DS18B20 Temperature Sensor

*/

#include "cadalogger.h"
cadalogger distlogger(CADALOGGERMINI);

byte StartByte = 0;       // 0xFF signifies a new packet of info coming in
byte MSByte = 0;          // Most Significant Byte of distance measurement
byte LSByte = 0;          // Least Significant Byte of distance measurement
byte CheckSum = 0;        // CheckSum = 0xFF + MSByte + LSByte


#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 19                 // DS18B20 Temperature Sensor data wire is plugged into this pin on MINI
#define ONE_WIRE_pwr 4
OneWire oneWire(ONE_WIRE_BUS);	        // Setup a oneWire instance to communicate with any OneWire device
DallasTemperature sensors(&oneWire);    // Pass oneWire reference to DallasTemperature library

File file;                              // cadalogger loads SdFat


void setup() {
  Serial.begin(9600);
  distlogger.initialise();     // Start up the MINI
  distlogger.enable_watchdog();

  pinMode(ONE_WIRE_BUS, INPUT); // turn on pins
  pinMode(ONE_WIRE_pwr, OUTPUT);
  sensors.begin();	            // Start up the DS18B20 Temperature Sensor 

  Serial.begin(9600);
  Serial2.begin(9600);         // Set up software serial port

}

void loop() {
  distlogger.feed_watchdog();    // loop takes about 15 seconds

  // read sensors ------------------------------------------------------------------
  
  // Turn thermometer on, get temperature, shut down
  digitalWrite(ONE_WIRE_BUS, HIGH);
  digitalWrite(ONE_WIRE_pwr, HIGH);

  sensors.requestTemperatures(); 
  float temp = sensors.getTempCByIndex(0);

  digitalWrite(ONE_WIRE_pwr, LOW);
  digitalWrite(ONE_WIRE_BUS, LOW);

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C | ");

  // Get distance
  float dist = get_distanceAVG();

  Serial.print("Distance: ");
  //Serial.print(get_distance());
  Serial.print(dist);
  Serial.println("mm");

  // print to microSD card ------------------------------------------------------------------
  // 1. wake up SD card
  distlogger.power_up_sd();

  // 2. open the file
  file = distlogger.SD.open("distlogger.csv", FILE_WRITE);

  // 3a. if file is avilable, write to it
  if (file) {
    distlogger.update_time();
    write_time();

    file.print(temp);
    file.print(",");

    file.println(dist);

    file.flush();
    file.close();
  }

  // 3b. if the file doesn't open, give an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  // 4. shut down sd card
  distlogger.power_down_sd();





  // put board to sleep ----------------------------------------------------------------------
  // sleep for 10 sec * sleep / 60 = X minutes
  // (5 minutes * 60 seconds / minute) / 10 second heartbeat = 30 sleep cycles
  for (byte sleep = 0; sleep < 4; sleep++) {
    distlogger.feed_watchdog();
    distlogger.go_to_sleep_until_RTC_wake();
    distlogger.flash(1);
  }
  
}










/* ---------------- FUNCTIONS ---------------- */

int get_distance() {
  Serial2.flush();      // Clear the serial port buffer
  Serial2.write(0x55);  // Request distance measurement
  int start = millis();
  while (Serial2.available() < 4 & millis() < (start + 5000)) {}

  int dist = -255;
  if (Serial2.available() >= 4) {      // Looking for 4 bytes to be returned
    StartByte = Serial2.read();        // Read first byte
    if (StartByte == 0xFF) {           // 1st byte is 0xFF for new measurement
      byte MSByte = Serial2.read();    // Read in the MSB
      byte LSByte = Serial2.read();    // Read in the LSB
      byte CheckSum = Serial2.read();  // Read the checksum byte
      dist = MSByte * 256 + LSByte;    // Calculate distance from the two bytes
    }
  }
  Serial2.write(0x55);           // turn off???
  //Serial.println(dist);
  //Serial.println(dist);
  return (dist);
}




float get_distanceAVG() {
  int samples[10];
  for (int i = 0; i < 14; i++) {

    samples[i] = get_distance();
    delay(1500);

    distlogger.feed_watchdog();    // each read takes about 1 second
    
  }

  int distMAX = samples[2];
  int distMIN = samples[2];
  for (int i = 3; i < 14; i++) {

    distMAX = max(samples[i], distMAX);
    distMIN = min(samples[i], distMIN);
  }

  int distSUM = 0;
  for (int i = 2; i < 14; i++) {
    distSUM = distSUM + samples[i];
  }
  int sum_ten = distSUM-distMAX-distMIN;
  float distAVG = ((double)sum_ten ) / 10.0;
  return (distAVG);
}




// function to compactify writing time/date to SD card
void write_time() {
  file.print(distlogger.time[0]); file.print(",");
  file.print(distlogger.time[1]); file.print(",");
  file.print(distlogger.time[2]); file.print(",");
  file.print(distlogger.time[3]); file.print(",");
  file.print(distlogger.time[4]); file.print(",");
  file.print(distlogger.time[5]); file.print(",");
}

