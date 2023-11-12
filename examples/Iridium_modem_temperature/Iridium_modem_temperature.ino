#include <IridiumSBD.h>
#include "cadalogger.h"
#include <OneWire.h>
#include <DS18B20.h>

#define ONE_WIRE_BUS 6 // aka sck on cadalogger mini board
OneWire oneWire(ONE_WIRE_BUS);
DS18B20 sensor(&oneWire);

cadalogger templogger(CADALOGGERMINI);

int signalQuality = -1;
int err;


#define IridiumSerial Serial2
#define DIAGNOSTICS false // Change this to see diagnostics

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial,7);  // sleep control on pin 7 (aka ss)

void setup()
{
  templogger.initialise(5);  // initialise and set up watchdog
  templogger.enable_watchdog();
  templogger.feed_watchdog();
  Serial.begin(9600);

  
  // Start the console serial port
  Serial.begin(9600);
  while (!Serial);

  sensor.begin();
  templogger.feed_watchdog();
}

void loop()
{
  templogger.feed_watchdog();

  // get temperature
  sensor.requestTemperatures();
  while (!sensor.isConversionComplete());  // wait until sensor is ready
  float temp = sensor.getTempC();

  templogger.feed_watchdog();

  // Start the serial port connected to the satellite modem
  IridiumSerial.begin(19200);

  // Begin satellite modem operation
  Serial.println("Starting modem...");
  err = modem.begin();
  if (err != ISBD_SUCCESS)
  {
    Serial.print("Begin failed: error ");
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      Serial.println("No modem detected: check wiring.");
    return;
  }

  templogger.feed_watchdog();
  // Example: Print the firmware revision
  char version[12];
  err = modem.getFirmwareVersion(version, sizeof(version));
  if (err != ISBD_SUCCESS)
  {
     Serial.print("FirmwareVersion failed: error ");
     Serial.println(err);
     return;
  }
  Serial.print("Firmware Version is ");
  Serial.print(version);
  Serial.println(".");

  templogger.feed_watchdog();

  // Example: Test the signal quality.
  // This returns a number between 0 and 5.
  // 2 or better is preferred.
  err = modem.getSignalQuality(signalQuality);
  if (err != ISBD_SUCCESS)
  {
    Serial.print("SignalQuality failed: error ");
    Serial.println(err);
    return;
  }

  Serial.print("On a scale of 0 to 5, signal quality is currently ");
  Serial.print(signalQuality);
  Serial.println(".");


  templogger.feed_watchdog();


  // Send the message
  Serial.print("Trying to send the message.  This might take several minutes.\r\n");

  char msg[20];
  char str_temp[6];
  dtostrf(temp, 4, 2, str_temp);
  sprintf(msg,"t: %s",str_temp);
  Serial.println(msg);
  err = modem.sendSBDText(msg);
  if (err != ISBD_SUCCESS)
  {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      Serial.println("Try again with a better view of the sky.");
  }

  else
  {
    Serial.println("Hey, it worked!");
  }
templogger.feed_watchdog();

  modem.sleep();

 for(int i = 0; i<710;i++){
  templogger.feed_watchdog();
  templogger.go_to_sleep_until_RTC_wake();
  templogger.flash(1);
 }

}

bool ISBDCallback()
{
  templogger.feed_watchdog();
  digitalWrite(24, (millis() / 1000) % 2 == 1 ? HIGH : LOW);
   return true;
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  Serial.write(c);
}
#endif