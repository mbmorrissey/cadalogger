/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  This is modified from the standard Arduino 'Blink' sketch, which
  itself is modified 8 May 2014 by Scott Fitzgerald,  modified 2 Sep
  2016 by Arturo Guadalupi modified 8 Sep 2016 by Colby Newman

*/

// the setup function runs once when you press reset or power the board

#define LED_PIN 24 // use this line if using the standard 'cadalogger mini' board
// #define LED_PIN 28 // uncomment if using the hand-assembled 'cadalogger maxi' board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}
