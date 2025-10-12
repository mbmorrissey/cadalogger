#include "cadalogger.h"

#define distSwitch A0
cadalogger stream(0);

File file;    // file object to record events
char df[80];  // data file name (to include date)

void setup() {
  stream.initialise(); 

  pinMode(distSwitch, OUTPUT);
  digitalWrite(distSwitch,HIGH); // turns off the distance sensor

  Serial2.begin(9600);
}



void loop() {
  
  double d = readDist();
  
  stream.update_time();
  stream.power_up_sd();
  sprintf(df, "streamlog_%d_%d.csv", stream.time[3], stream.time[4]);
  file = stream.SD.open(df, FILE_WRITE);
  write_time();
  file.print(",");
  file.print(d);
  file.print(",");
  file.println(stream.supply_voltage());
  file.flush();
  file.close();
  stream.power_down_sd();

  stream.wakeOnSecondMultiple(60);
}


// takes ten reads, and returns mean, excluding highest and lowest
double readDist(){
  int readsTot = 10;
  int sum = 0;
  int lowest;
  int highest;

  digitalWrite(distSwitch,LOW);
  pinMode(35,INPUT_PULLUP);
  Serial2.begin(9600);
  stream.rest(1024);
  while(Serial2.available()) Serial2.read();

  for(int i = 0; i < readsTot; i++){
    while(Serial2.available()) Serial2.read();
    char data[4];
    while(!Serial2.available()){}
    Serial2.read();  // discard initial 'R'
    for(int i = 0; i<4; i++){
      while(!Serial2.available()){}
      data[i] = Serial2.read();
    }
    int distmm = atoi(data);
    if(i==0){
      lowest = distmm;
      highest = distmm;
    } else {
      if(distmm<lowest) lowest=distmm;
      if(distmm>highest) highest=distmm;
    }
    sum=sum+distmm;
    stream.rest(512);
  }
  digitalWrite(distSwitch,HIGH);
  Serial2.end();
  pinMode(35,OUTPUT);
  digitalWrite(35,LOW);
  return( ((double) (sum-lowest-highest)) / ((double) (readsTot-2)) );
}

// compactly writes six fields of date and time to data file
void write_time() {
  file.print(stream.time[0]);
  file.print(",");
  file.print(stream.time[1]);
  file.print(",");
  file.print(stream.time[2]);
  file.print(",");
  file.print(stream.time[3]);
  file.print(",");
  file.print(stream.time[4]);
  file.print(",");
  file.print(stream.time[5]);
}
