// cadalogger.cpp
// Michael Morrissey and Rebecca Nagel

#include "cadalogger.h"
#define rtc_RV3032 0x51       // A2 in hex = 162 in dec, 162/2 = 81 in dec, 81 in dec = 51



// constructor
cadalogger::cadalogger(byte boardVersion){
  if(boardVersion==0){
    _board_version = 0;
  }
  if(boardVersion==1){
    _board_version = 1;
  }
}


// initialise() with no arguments for RTC defaults
void cadalogger::initialise(){

  // pins
  if(_board_version==0){
    _uSD_SCL_pin = CADALOGGERMINI_SD_SCL_PIN;
    _uSD_MISO_pin = CADALOGGERMINI_SD_MISO_PIN;
    _uSD_MOSI_pin = CADALOGGERMINI_SD_MOSI_PIN;
    _uSD_SS_pin = CADALOGGERMINI_SD_SS_PIN;
    _uSD_power_pin = CADALOGGERMINI_SD_POWER_PIN;
    _LED_pin = CADALOGGERMINI_SD_LED_PIN;
    _RTC_int_pin = CADALOGGERMINI_RTC_INT_PIN;

    Wire.begin();
    _heartbeat_interval_s = 0;     // RTC set-up will not generate regular interrupts
    cadalogger::_prepare_rtc();
    for(byte i=0; i<25; i++){
      pinMode(i,INPUT_PULLUP);
    }
  }

  pinMode(_LED_pin,OUTPUT);
  digitalWrite(_LED_pin, LOW);
  pinMode(_uSD_power_pin,OUTPUT);
  digitalWrite(_uSD_power_pin, LOW);

  cadalogger::power_up_sd();
  cadalogger::power_down_sd();
}


// initialise with interval for RTC interrupts
void cadalogger::initialise(int heartbeat){

  // pins
  if(_board_version==0){
    _uSD_SCL_pin = CADALOGGERMINI_SD_SCL_PIN;
    _uSD_MISO_pin = CADALOGGERMINI_SD_MISO_PIN;
    _uSD_MOSI_pin = CADALOGGERMINI_SD_MOSI_PIN;
    _uSD_SS_pin = CADALOGGERMINI_SD_SS_PIN;
    _uSD_power_pin = CADALOGGERMINI_SD_POWER_PIN;
    _LED_pin = CADALOGGERMINI_SD_LED_PIN;
    _RTC_int_pin = CADALOGGERMINI_RTC_INT_PIN;

    Wire.begin();
    _heartbeat_interval_s = heartbeat;
    cadalogger::_prepare_rtc();
    for(byte i=0; i<25; i++){
      pinMode(i,INPUT_PULLUP);
    }
  }

  pinMode(_LED_pin,OUTPUT);
  digitalWrite(_LED_pin, LOW);
  pinMode(_uSD_power_pin,OUTPUT);
  digitalWrite(_uSD_power_pin, LOW);

  cadalogger::power_up_sd();
  cadalogger::power_down_sd();
}


void cadalogger::flash(byte flashes){
  for(byte i = 0; i < flashes; i++){
    digitalWrite(_LED_pin, HIGH);
    cadalogger::rest(10);
    digitalWrite(_LED_pin, LOW);
    cadalogger::rest(90);
  }
}



// low-power alternative to delay()
// sleeps for 'cycles' cycles of the divide-by-1000 scaled 1024 mhz clock, such that 
// sleep is for approximately cycles ms (for 1s, use 1024 cycles)
void cadalogger::rest(int cycles){  
  cli();
  while (RTC.STATUS > 0) {} 
  RTC.CMP = cycles; // e.g., cycles 1024 for 1 sec timer.
  RTC.CNT = 0;
  RTC.INTCTRL = 1 << RTC_CMP_bp | 0 << RTC_OVF_bp; // interrupt
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc; // 1.024kHz
  RTC.CTRLA = 0b10000001;
  sei(); 
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_enable();
  sleep_cpu(); 
  // sleeps here until the counter generates interrupt
  sleep_disable();
//  RTC.CTRLA = 0b10000000; // disable MCUs RTC module
  RTC.INTCTRL = 0 << RTC_CMP_bp | 0 << RTC_OVF_bp; //disable interrupts.
}

// interrupt service routine to service the sleep function in logger::rest()
ISR(RTC_CNT_vect) // n.b. RTC here refers to the 4808's real time counter, not the real time clock
{
//  RTC.INTFLAGS = RTC_CMP_bm;
  RTC.INTFLAGS = 0b00000011;
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



void cadalogger::_prepare_rtc(){
  if(_board_version==0){
    cadalogger::_prepare_rtc_RV3032();
  }
  if(_board_version==1){
    cadalogger::_prepare_rtc_DS3231();
  }
}

void cadalogger::_prepare_rtc_DS3231(){
 // Rebecca code to go here
}

void cadalogger::_prepare_rtc_RV3032(){

  // see manual page 72 for setting of configuration registers in EEPROM via RAM mirrors

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
  
  // re-enable refresh (n.b., this register may subsequently be further modified to allow periodic Countdown Timer)
  Wire.beginTransmission(rtc_RV3032); 
  Wire.write(0x10);
  Wire.write(0b00000000);
  Wire.endTransmission();
  
  // sets up the heartbeat
  if(_heartbeat_interval_s>0){
    // set 0Bh register (standard binary format) lower 8 bits; CV-3032 manual page 21: 0001010b (10 seconds)
    // set 0Ch register (standard binary format) upper 4 bits; CV-3032 manual page 21: 0000000b (10 seconds)
    byte lower = _heartbeat_interval_s&0xFF;
    byte upper = (_heartbeat_interval_s >> 8)&0xF;

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
    Wire.write(0x11);    
    Wire.endTransmission();
    Wire.requestFrom(rtc_RV3032,1);
    byte temp = Wire.read();
    Wire.endTransmission();  
    temp = temp | 0b00010000;
    Wire.beginTransmission(rtc_RV3032);
    Wire.write(0x11);  
    Wire.write(temp);
    Wire.endTransmission();

    // set 0Dh (Extension Register) bit 4, Period Countdown Timer Enable
    // ***** it is bit 3 on the CV-3032
    // set 0Dh bits 1:0 to 10b to select 1Hz
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
}



double cadalogger::rtc_temp(){
  if(_board_version==0){
    cadalogger::_rtc_temp_RV3032();
  }
  if(_board_version==1){
    cadalogger::_rtc_temp_DS3231();
  }
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
  // Rebecca code to go here
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


// This might be good to include logger::file.close(); before the rest of the shutdown routine
void cadalogger::power_down_sd(){
  rest(1024);  // give 1 second for uSD card housekeeping as per SD specification
  SD.end();
  SPI.end();   
  SPI.swap(0); // pins back to default for normal SPI use
  pinMode(_uSD_SCL_pin, INPUT);
  pinMode(_uSD_MISO_pin, INPUT);
  pinMode(_uSD_MOSI_pin, INPUT);
  pinMode(_uSD_SS_pin, INPUT);
  digitalWrite(_uSD_power_pin, LOW);
}

void cadalogger::power_up_sd(){
  pinMode(_uSD_SS_pin, OUTPUT);
  digitalWrite(_uSD_SS_pin, HIGH); //pullup the CS pin on the SD card (but only if you don’t already have a hardware pullup on your module)
  pinMode(_uSD_MOSI_pin, OUTPUT);
  digitalWrite(_uSD_MOSI_pin, HIGH); //pullup the MOSI pin on the SD card
  pinMode(_uSD_MISO_pin,INPUT_PULLUP);//pullup the MISO pin on the SD card
  pinMode(_uSD_SCL_pin, OUTPUT);
  digitalWrite(_uSD_SCL_pin, LOW); //pull DOWN the 13scl pin on the SD card (IDLES LOW IN MODE0)
  digitalWrite(_uSD_power_pin, HIGH);
  delay(10);
  SPI.swap(1);
  SPI.begin();
  SD.begin(_uSD_SS_pin);
}

void cadalogger::go_to_sleep_until_RTC_wake(){

  pinMode(_RTC_int_pin,INPUT);
  cli();
  set_sleep_mode(SLEEP_MODE_STANDBY);  //  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(_RTC_int_pin), cadalogger::_RTC_interrupt, FALLING);
  sei();
  sleep_cpu(); 
  // sleeps here until the interrupt V falls
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(_RTC_int_pin));
}

// possibly not needed at all now? - RTC's low signal is automatically cleared, I think
static void cadalogger::_RTC_interrupt(){
}


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
