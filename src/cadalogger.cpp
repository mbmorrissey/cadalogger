// cadalogger.cpp
// Michael Morrissey and Rebecca Nagel
// 03 Sept 2025

#include "cadalogger.h"
#define rtc_DS3231 0x68
#define rtc_RV3032 0x51    




// constructor
cadalogger::cadalogger(byte boardVersion){
    _board_version = boardVersion;
}


// initialise() with no arguments for RTC defaults
void cadalogger::initialise(){

  delay(100);
  wdt_reset();
  wdt_disable();
  // pins
  if(_board_version==0){
    _uSD_SCL_pin = 16;
    _uSD_MISO_pin = 15;
    _uSD_MOSI_pin = 14;
    _uSD_SS_pin = 17;
    _uSD_power_pin = 21;  
    LED_pin = 37;
    _RTC_int_pin = 36;

    Wire.begin();
    cadalogger::_prepare_rtc_RV3032();
    for(byte i=0; i<40; i++){
      pinMode(i,INPUT_PULLUP);
    }
  }

  if(_board_version==1){
    _uSD_SCL_pin = 10;
    _uSD_MISO_pin = 9;
    _uSD_MOSI_pin = 8;
    _uSD_SS_pin = 11;
    _uSD_power_pin = 30;  
    LED_pin = 31;
    _RTC_int_pin = 28;

    Wire.begin();

    for(byte i=0; i<32; i++){
      pinMode(i,INPUT_PULLUP);
    }

  }

  if(_board_version == 0){
    pinMode(A15,INPUT);
  }else{
    pinMode(A13,INPUT);
  }

  
  pinMode(LED_pin,OUTPUT);
  digitalWrite(LED_pin, LOW);
  pinMode(_uSD_power_pin,OUTPUT);

  if(_board_version == 0){
    digitalWrite(_uSD_power_pin, LOW);
  }else{
    digitalWrite(_uSD_power_pin, HIGH);
  }

  cadalogger::power_up_sd();
  cadalogger::power_down_sd();
}









byte cadalogger::_getSecond(){
  byte n = -1;
  if(_board_version==0){
    Wire.beginTransmission(rtc_RV3032);  
    Wire.write(0x01); 
    Wire.endTransmission();
    Wire.requestFrom(rtc_RV3032,1);
    n = cadalogger::_BcdToDec(Wire.read());
    Wire.endTransmission();  
  }
  if(_board_version==1){
    Wire.beginTransmission(rtc_DS3231);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(rtc_DS3231,1);
    n = cadalogger::_BcdToDec(Wire.read());
    Wire.endTransmission();
  }
  return(n);
}



byte cadalogger::_getMinute(){
  byte n = -1;
  if(_board_version==0){
    Wire.beginTransmission(rtc_RV3032);  
    Wire.write(0x02); 
    Wire.endTransmission();
    Wire.requestFrom(rtc_RV3032,1);
    n = cadalogger::_BcdToDec(Wire.read());
    Wire.endTransmission();  
  }
  if(_board_version==1){
    Wire.beginTransmission(rtc_DS3231);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.requestFrom(rtc_DS3231,1);
    n = cadalogger::_BcdToDec(Wire.read());
    Wire.endTransmission();
  }
  return(n);
}



void cadalogger::_sleepToDS3231(int s){

  if(s>59) s = 0;

  Wire.beginTransmission(rtc_DS3231);
  Wire.write(0x07);
  Wire.write(cadalogger::_DecToBcd(s));
  Wire.write(0b10000000);
  Wire.write(0b10000000);
  Wire.write(0b10000000);
  Wire.endTransmission(); 

  Wire.beginTransmission(rtc_DS3231);
  Wire.write(0x0E);
  Wire.write(0b00011101);
  Wire.write(0b00000000);
  Wire.endTransmission(); 
  
  sleepToRTCwake();

  Wire.beginTransmission(rtc_DS3231);
  Wire.write(0x0F);
  Wire.write(0b00000000);
  Wire.endTransmission(); 
}





// sleeps until a given second within the minute
// does not stop to allow WDT resets
void cadalogger::_sleepSeconds_DS3231(byte s){
  byte n = cadalogger::_getSecond();
  n = n+s;
  if(n>59) n = n-60;
  cadalogger::_sleepToDS3231(n);
}






void cadalogger::_prepare_rtc_RV3032(){
  /*
  RV3032 does not automatically have battery backup enabled, as
  does the DS3231.  This makes sure it is enabled.
  */
  
  // disable refresh
  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0x10);
  Wire.write(0b00000100);
  Wire.endTransmission();

  // enable backup battery power switchover
  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0xC0);
  Wire.write(0b00100000);
  Wire.endTransmission();

  // transfer RAM mirror to EEPROM
  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0x3F);
  Wire.write(0b00010001); // 11 in hex  to EECMD as per page 44 of manual
  Wire.endTransmission();
  
  // re-enable refresh (n.b., this register will subsequently be further modified to allow periodic Countdown Timer)
  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0x10);
  Wire.write(0b00000000);
  Wire.endTransmission();
}




void cadalogger::sleepFor(int seconds){
  if(_board_version==0) cadalogger::_sleepForRV3032(seconds);
  if(_board_version==1) cadalogger::_sleepForDS3231(seconds);
}

// sleeps for a given number of seconds, waking every
// five seconds to give a single blink and to reset
// the watchdog timer
void cadalogger::_sleepForDS3231(int seconds){
  // work out number of five second intervals
  int intervals = seconds / 5;
  // work out remainder short interval length
  int part_interval = seconds % 5;
  // sleep for required intervals and part interval
  for(int i = 0; i < intervals; i++){
    cadalogger::_sleepSeconds_DS3231(5);
    wdt_reset();
    flash(1);
  }
  if(part_interval>0){
    cadalogger::_sleepSeconds_DS3231(part_interval);
    wdt_reset();
    flash(1);
  }
}


/*
 * for setting the periodic countdown timer on the RV3032.  This function
 * uses the 1 Hz clock, which is most useful if a consistent interrupt is
 * needed over long periods of logging.  it can be imprecise and accumulate
 * substantial error if used repeatedly with resets to different periods
 */

void cadalogger::setHeartBeat(int interval){
  // set 0Bh register (standard binary format) lower 8 bits; CV-3032 manual page 21: 0001010b (10 seconds)
  // set 0Ch register (standard binary format) upper 4 bits; CV-3032 manual page 21: 0000000b (10 seconds)
  byte lower = interval&0xFF;
  byte upper = (interval >> 8)&0xF;

  Wire.beginTransmission(rtc_RV3032);  // set TE to zero so that countdown can start correctly when it is put back to 1
  Wire.write(0x10);   
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,1);
  byte  temp = Wire.read();
  Wire.endTransmission();  
  temp = temp & 0b11110111;  
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x10); 
  Wire.write(temp);
  Wire.endTransmission();
  
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x0B);  
  Wire.write(lower);
  Wire.endTransmission();
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x0C); 
  Wire.write(upper);
  Wire.endTransmission();

  // set 0Fh (Control Register) bit 4, Periodic Countdown Timer Interrupt Enable
  Wire.beginTransmission(rtc_RV3032); 
//  Wire.write(0x0F);  
  Wire.write(0x11);    
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,1);
  temp = Wire.read();
  Wire.endTransmission();  
  temp = temp | 0b00010000;
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x11);  
  Wire.write(temp);
  Wire.endTransmission();

  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0x10);   
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,1);
  temp = Wire.read();
  Wire.endTransmission();  
  temp = temp | 0b00001010;  
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x10); 
  Wire.write(temp);
  Wire.endTransmission();
}

/*
 * for setting the periodic countdown timer on the RV3032 with teh 64 Hz
 * clock.  This is useful to reduce accumulation of errors in first period
 * length.
 */

void cadalogger::_setHeartBeatFast(int interval){
  // set 0Bh register (standard binary format) lower 8 bits; CV-3032 manual page 21: 0001010b (10 seconds)
  // set 0Ch register (standard binary format) upper 4 bits; CV-3032 manual page 21: 0000000b (10 seconds)
  byte lower = interval&0xFF;
  byte upper = (interval >> 8)&0xF;

  Wire.beginTransmission(rtc_RV3032);  // set TE to zero so that countdown can start correctly when it is put back to 1
  Wire.write(0x10);   
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,1);
  byte  temp = Wire.read();
  Wire.endTransmission();  
  temp = temp & 0b11110111;  
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x10); 
  Wire.write(temp);
  Wire.endTransmission();
  
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x0B);  
  Wire.write(lower);
  Wire.endTransmission();
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x0C); 
  Wire.write(upper);
  Wire.endTransmission();

  // set 0Fh (Control Register) bit 4, Periodic Countdown Timer Interrupt Enable
  Wire.beginTransmission(rtc_RV3032); 
//  Wire.write(0x0F);  
  Wire.write(0x11);    
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,1);
  temp = Wire.read();
  Wire.endTransmission();  
  temp = temp | 0b00010000;
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x11);  
  Wire.write(temp);
  Wire.endTransmission();

  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0x10);   
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,1);
  temp = Wire.read();
  Wire.endTransmission();  
  temp = temp | 0b00001001;              // 01 for bits 1 and 0 is 64 Hz
  Wire.beginTransmission(rtc_RV3032);
  Wire.write(0x10); 
  Wire.write(temp);
  Wire.endTransmission();
}


/*
 * See manual section starting page 77 and table on page 80.  There first
 * period can be up to double in length.  Consequently, using the 1Hz clock
 * option can be quite imprecise, especially if it needs to be used repeatedly
 * at different periods to make up the desired interval.  Here, the 64 Hz
 * setting is used, this gives a a reasonably precise countdown over time
 * periods most relevant to environmental data monitoring, divided into
 * appropriate sub-intervals for resetting the watchdog timer.
 */
void cadalogger::_sleepForMsRV3032(uint16_t ms){
  // work out number of five second intervals
  uint16_t intervals = ms / 5000;
  // work out remainder short interval length
  uint16_t part_interval = ms % 5000;


  if(intervals>0){
    // set periodic iterrupt timer to 5 s interval
    cadalogger::_setHeartBeatFast(320);   // 5s at 64 Hz
    // sleep for those intervals
    for(int i = 0; i<intervals; i++){
      sleepToRTCwake();
      wdt_reset();
      flash(1);
    }
  }
  if(part_interval>0){ // uncertainty in initial period typically makes a 1s over-run
    // set periodic interrupt timer to the part interval length
    cadalogger::_setHeartBeatFast((float) part_interval / 1000.0 * 64.0);
    // sleep for the part interval
    sleepToRTCwake();
    wdt_reset();
    flash(1);
  }
}






// wakes on a multiple of s seconds.  For e.g., if called with 
// s = 20 when the current second is 23, the logger would sleep
// until the RTC time reached the next 40s mark.  If the interval
// until the next wake is longer than 5s, the logger will wake
// every 5s to blink and reset the WDT
byte cadalogger::wakeOnSecondMultiple(int s){

  if(60 % s > 0) return(1);
    
  if(_board_version==0) {
    Wire.beginTransmission(rtc_RV3032);  
    Wire.write(0x00); 
    Wire.endTransmission();
    Wire.requestFrom(rtc_RV3032,2);
    int ch = cadalogger::_BcdToDec(Wire.read());  // current hundredth of second
    int cs = cadalogger::_BcdToDec(Wire.read());  // current second
    Wire.endTransmission();

    uint16_t t = cs * 1000 + ch*10;
    uint16_t m = ceil((float) t / ((float) s * 1000.0))*s*1000;

    cadalogger::_sleepForMsRV3032(m-t);
    
  }


  if(_board_version==1) {
    // get current second
    byte c = cadalogger::_getSecond();
    // find second to wake upon (this will be m*s)
    int m = 0; // multiplier of second interval that we need to wake upon
    while(c >= s*m && s*m <= 60) m++;
    cadalogger::_sleepForDS3231(s*m-c);
  }

  return(0);
}








byte cadalogger::wakeOnMinuteMultiple(int s){

  if(60 % s > 0) return(1);

  // get current second
  byte c = cadalogger::_getMinute();

  // find minute to wake upon
  int m = 0; // multiplier of minute interval that we need to wake upon
  while(c >= s*m && s*m <= 60) m++;
  // sleep until minute s*m
  while(s*m > cadalogger::_getMinute()){
    wakeOnSecondMultiple(60);
  }
  return(0);
}




void cadalogger::sleepToRTCwake(){
  set_sleep_mode(SLEEP_MODE_STANDBY);  
/*
  PORTF_PIN2CTRL = PORT_ISC_FALLING_gc      // sense falling edge on PORTF pin 2 (ext-int)
                    | PORT_PULLUPEN_bm;     // Set PF2-state high
*/
  attachInterrupt(digitalPinToInterrupt(_RTC_int_pin),cadalogger::RTCwake,FALLING);
  cli();
  sleep_enable();
  sei();
  sleep_cpu(); 
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(_RTC_int_pin));
}

static void cadalogger::RTCwake(){
  
}
/*
ISR for wakeup from real time clock alarm on port F pin 2
ISR(PORTF_PORT_vect){
  PORTF_INTFLAGS = PIN2_bm;           //clear port interrupt flag
  PORTF_PIN2CTRL =  PORT_PULLUPEN_bm;     // Set PF2-state high, no interrupt
}
*/



void cadalogger::flash(byte flashes){
  if(flashes > 1){
    for(byte i = 0; i < (flashes-1); i++){
      digitalWrite(LED_pin, HIGH);
      rest(10);
      digitalWrite(LED_pin, LOW);
      rest(90);
    }
    digitalWrite(LED_pin, HIGH);
    rest(10);
    digitalWrite(LED_pin, LOW);
  }else{
    digitalWrite(LED_pin, HIGH);
    rest(10);
    digitalWrite(LED_pin, LOW);
  }
}


void cadalogger::rest(int cycles){  
//  cli();
  RTC.CTRLA = 0; // disable MCUs RTC module 
  while (RTC.STATUS > 0) {} // probably unnecessary, given line above!
  RTC.CMP = cycles; // e.g., cycles 1024 for 1 sec timer.
  RTC.CNT = 0;
  RTC.INTCTRL = RTC_CMP_bm ; // compare match interrupt
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc; // 1.024kHz
  RTC.CTRLA = RTC_RUNSTDBY_bm | RTC_RTCEN_bm;  // enable MCUs RTC module and ensure running in standby mode
  cli();
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_enable();
  sei();   
  sleep_cpu(); 
  // sleeps here until the counter generates interrupt
  sleep_disable();
  RTC.CTRLA = 0; // disable MCUs RTC module
  RTC.INTCTRL = 0 ; //disable MCUs RTC module interrupts.
} 

// interrupt service routine to service the sleep function in logger::rest()
ISR(RTC_CNT_vect) {
  RTC.INTFLAGS = RTC_CMP_bm; // clear compare match interrupt flag
}






// conversions for binary coded decimal; used by RTC
// handy explanation at http://danceswithferrets.org/geekblog/?p=504
byte cadalogger::_DecToBcd(byte val){
   return( ((val/10) << 4) | (val%10) );
}
byte cadalogger::_BcdToDec(byte bcd){
   return( ((bcd >> 4) * 10) + (bcd&0xF) );
}


// copies time from the RTC to the logger::time[] vector; good to do this
// just before writing or displaying current time
void cadalogger::update_time(){
  if(_board_version==0){
    cadalogger::_update_time_RV3032();
  }
  if(_board_version==1){
    cadalogger::_update_time_DS3231();
  }
}

void cadalogger::_update_time_DS3231(){
  Wire.beginTransmission(rtc_DS3231);  
  Wire.write(0);  // pointer
  Wire.endTransmission();
  Wire.requestFrom(rtc_DS3231,7);
  for(byte i=0; i<3;i++){
    cadalogger::time[i] = cadalogger::_BcdToDec(Wire.read());
  }
  byte weekday = Wire.read();
  for(byte i=3; i<6;i++){
    cadalogger::time[i] = cadalogger::_BcdToDec(Wire.read());
  }
  Wire.endTransmission();  
}

void cadalogger::_update_time_RV3032(){
  Wire.beginTransmission(rtc_RV3032);  
  Wire.write(1);  // pointer
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,7);
  for(byte i=0; i<3;i++){
    cadalogger::time[i] = cadalogger::_BcdToDec(Wire.read());
  }
  byte weekday = Wire.read();
  for(byte i=3; i<6;i++){
    cadalogger::time[i] = cadalogger::_BcdToDec(Wire.read());
  }
  Wire.endTransmission();  
}


double cadalogger::rtc_temp(){
  double t;
  if(_board_version==0){
    t = cadalogger::_rtc_temp_RV3032();
  }
  if(_board_version==1){
    t = cadalogger::_rtc_temp_DS3231();
  }
  return(t);
}

double  cadalogger::_rtc_temp_RV3032(){
  Wire.beginTransmission(rtc_RV3032);  
  Wire.write(0x0E);  // pointer
  Wire.endTransmission();
  Wire.requestFrom(rtc_RV3032,2);
  byte temp_lsb = Wire.read();
  byte temp_msb = Wire.read();
  Wire.endTransmission(); 
  byte sign = temp_msb >> 7;
  if(sign==0){
    temp_msb = temp_msb & 0x7F;
    temp_lsb = (temp_lsb  & 0xF0) >> 4;
  }else{
    temp_msb = (~temp_msb) & 0x7F;
    temp_lsb = (((~temp_lsb)  & 0xF0) >> 4) + 1;
  }
  double temp_decimal = ((-2*sign)+1)*(temp_msb + 0.0625*temp_lsb);
  return(temp_decimal);
}

double  cadalogger::_rtc_temp_DS3231(){
  Wire.beginTransmission(rtc_DS3231);  
  Wire.write(0x11);  // pointer
  Wire.endTransmission();
  Wire.requestFrom(rtc_DS3231,2);
  byte temp_msb = Wire.read();
  byte temp_lsb = Wire.read();
  Wire.endTransmission();
  byte sign = temp_msb >> 7;
  if(sign==0){
    temp_msb = temp_msb & 0x7F;
    temp_lsb = (temp_lsb  & 0xC0) >> 6;
  }else{
    temp_msb = (~temp_msb) & 0x7F;
    temp_lsb = (((~temp_lsb)  & 0xC0) >> 6) + 1;
  }
  double temp_decimal = ((-2*sign)+1)*(temp_msb + 0.25*temp_lsb);
  return(temp_decimal);
}




void cadalogger::write_time_to_rtc(){
  if(_board_version==0){
    cadalogger::_write_time_to_RV3032();
  }
  if(_board_version==1){
    cadalogger::_write_time_to_DS3231();
  }
}

void cadalogger::_write_time_to_DS3231(){
  // write initial time to rtc
  for(byte i=0; i<3;i++){
    Wire.beginTransmission(rtc_DS3231);
    Wire.write(i);  // pointer
    Wire.write(cadalogger::_DecToBcd(cadalogger::time[i]));
    Wire.endTransmission();    
  }
  for(byte i=4; i<7;i++){
    Wire.beginTransmission(rtc_DS3231);
    Wire.write(i);  // pointer
    Wire.write(cadalogger::_DecToBcd(cadalogger::time[i-1]));
    Wire.endTransmission();    
  }
}

void cadalogger::_write_time_to_RV3032(){
  // write initial time to rtc
  for(byte i=1; i<4;i++){
    Wire.beginTransmission(rtc_RV3032);
    Wire.write(i);  // pointer
    Wire.write(cadalogger::_DecToBcd(cadalogger::time[i-1]));
    Wire.endTransmission();    
  }
  for(byte i=5; i<8;i++){
    Wire.beginTransmission(rtc_RV3032);
    Wire.write(i);  // pointer
    Wire.write(cadalogger::_DecToBcd(cadalogger::time[i-2]));
    Wire.endTransmission();    
  }
}



void cadalogger::power_down_sd(){
  rest(1024);  // give 1 second for uSD card housekeeping as per SD specification
  SD.end();
  SPI.end();   
  SPI.swap(0); // pins back to default for normal SPI use
  pinMode(_uSD_SCL_pin, INPUT);
  pinMode(_uSD_MISO_pin, INPUT);
  pinMode(_uSD_MOSI_pin, INPUT);
  pinMode(_uSD_SS_pin, INPUT);

  if(_board_version == 0){
    digitalWrite(_uSD_power_pin, LOW);
  }else{
    digitalWrite(_uSD_power_pin, HIGH);
  }
}

void cadalogger::power_down_sd(bool wait){
  if(wait) rest(1024);  // give 1 second for uSD card housekeeping as per SD specification
  SD.end();
  SPI.end();   
  SPI.swap(0); // pins back to default for normal SPI use
  pinMode(_uSD_SCL_pin, INPUT);
  pinMode(_uSD_MISO_pin, INPUT);
  pinMode(_uSD_MOSI_pin, INPUT);
  pinMode(_uSD_SS_pin, INPUT);

  if(_board_version == 0){
    digitalWrite(_uSD_power_pin, LOW);
  }else{
    digitalWrite(_uSD_power_pin, HIGH);
  }
}

void cadalogger::power_up_sd(){

  // not sure if this is redundant: check if SPI.begin() actually does all this?
  pinMode(_uSD_SS_pin, OUTPUT);
  digitalWrite(_uSD_SS_pin, HIGH); //pullup the CS pin on the SD card 
  pinMode(_uSD_MOSI_pin, OUTPUT);
  digitalWrite(_uSD_MOSI_pin, HIGH); //pullup the MOSI pin on the SD card
  pinMode(_uSD_MISO_pin,INPUT_PULLUP);//pullup the MISO pin on the SD card
  pinMode(_uSD_SCL_pin, OUTPUT);
  digitalWrite(_uSD_SCL_pin, LOW); //pull DOWN the 13scl pin on the SD card (IDLES LOW IN MODE0)

  if(_board_version == 0){
    digitalWrite(_uSD_power_pin, HIGH);
  }else{
    digitalWrite(_uSD_power_pin, LOW);
  }
  
  //rest(10); 
    delay(10);

  SPI.swap(1);
  SPI.begin();
  SD.begin(_uSD_SS_pin);
}


// I can't seem to figure out how to get the standard wdt enable command to work, though I can
// get the standard disable one to work.  For now, the cleanest thing is to do it by writing directly
// to WDT.CTRLA, so for consistency I here are functions to both enable and disable by this means
void cadalogger::enable_watchdog(){
  RSTCTRL.RSTFR |= RSTCTRL_WDRF_bm;
  wdt_enable(WDT_PERIOD_8KCLK_gc);
  wdt_reset(); // probably redundant
}
void cadalogger::disable_watchdog(){
  wdt_disable();
}
void cadalogger::feed_watchdog(){
  wdt_reset();
}


float cadalogger::supply_voltage(){

  int adc;
  if(_board_version == 0){
    adc = analogRead(A15);
  }else{
    adc = analogRead(A13);
  }
  
  return((float) adc / 1023.0 * 3.3 * 2.0);
}
