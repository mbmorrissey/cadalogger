#include "cadalogger.h"

cadalogger blinky(0);


void setup() {
  blinky.initialise();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(blinky.LED_pin, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                         // wait for a second
  digitalWrite(blinky.LED_pin, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                         // wait for a second
}
