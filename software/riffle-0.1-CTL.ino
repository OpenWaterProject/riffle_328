#include <JeeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include<stdlib.h>

#define led 6 //indicator led
#define photocellPin A2

#define debug 1 //whether or not to print statements out on serial

//sleeping
ISR(WDT_vect) { Sleepy::watchdogEvent(); }
#define LOG_INTERVAL_BASE  1000 // in millisec -- 60000 (60 sec) max 
#define LOG_INTERVAL_REPEAT 10 // number of times to repeat BASE

//battery
#define BATTERYPIN A3

//RTC
RTC_DS3231 RTC;

//sd card and filename
const int chipSelect = 7;    
int SDpower = 5;
int sensorPower = 4;

char filename[] = "LOGGER00.csv";
File dataFile;
String fileHeader = "DATETIME,RTC_TEMP_C,TEMP_C,CONDUCT_FREQ_HERTZ,BATTERY_LEVEL";


// temp variables 

// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 9550

int samples[NUMSAMPLES];

int interruptPin = 0; //corresponds to D2

// conductivity variables

long pulseCount = 0;  //a pulse counter variable

unsigned long pulseTime,lastTime, duration, totalDuration;

int samplingPeriod=2; // the number of seconds to measure 555 oscillations

int sensorBoard = A1; // the pin that powers the 555 subcircuit

void setup()
{
   if (debug==1){
    Serial.begin(9600);
  } 
  
      pinMode(sensorBoard, OUTPUT);   
      pinMode(THERMISTORPIN, INPUT);
      pinMode(led, OUTPUT);     
      
      //turn on SD card
      pinMode(SDpower,OUTPUT);
      //pinMode(sensorPower,OUTPUT); -- doesn't work on REVC, will work on REVD
      digitalWrite(SDpower,LOW);
      
      //initialize the SD card  
  if (debug==1){
    Serial.println();
    Serial.print("Initializing SD card...");
  }
  pinMode(SS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {  

     if (debug==1){
      Serial.println("Card failed, or not present");
    }
      
    for (int j=0;j<100;j++) {
        digitalWrite(led, LOW);
        delay(100);
        digitalWrite(led, HIGH);
         delay(100);
  }
  digitalWrite(led, LOW);
  
   
    // don't do anything more:
    while (1) ;
  }

  if (debug==1) {
    Serial.println("card initialized.");
  }
  
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      if (debug==1) {
        Serial.print("Writing to file: " );
        Serial.println(filename);
      }
      dataFile = SD.open(filename, FILE_WRITE);
      dataFile.println(fileHeader);
      dataFile.close();
      break;  // leave the loop!
    }
  }  
      
     // for i2c for RTC
  Wire.begin();
  RTC.begin();   
      
    // check on the RTC
  if (! RTC.isrunning()) {
    if (debug==1){
      Serial.println("RTC is NOT running!");
    }
      // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    if (debug==1) {
    Serial.println("RTC is older than compile time! Updating");
    }
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  if (debug==1) {
  Serial.println();
  Serial.println(fileHeader);  
  }
      
      
  //show that we're working
  if (debug==0) {
  for (int j=0;j<4;j++) {
  digitalWrite(led, LOW);
  delay(1000);
  digitalWrite(led, HIGH);
   delay(1000);
  }
  
   for (int j=0;j<3;j++) {
  digitalWrite(led, LOW);
  delay(500);
  digitalWrite(led, HIGH);
   delay(500);
  }
  
   digitalWrite(led, LOW);
   delay(1000);    
  }    
      
}

void loop()
{
   uint8_t i;
  float average;
  
  
  //turn on the sensorBoard system
  digitalWrite(sensorBoard,HIGH); //turns on the 555 timer subcircuit
  
  
  // temperature ---------------------------------------------
 
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   //Serial.println(analogRead(THERMISTORPIN));
   delay(10);
  }
  
  // conductivity --------------------------------------
  
  pulseCount=0; //reset the pulse counter
  totalDuration=0;  //reset the totalDuration of all pulses measured
  
  attachInterrupt(interruptPin,onPulse,RISING); //attach an interrupt counter to interrupt pin 1 (digital pin #3) -- the only other possible pin on the 328p is interrupt pin #0 (digital pin #2)
  
  pulseTime=micros(); // start the stopwatch
  
  delay(samplingPeriod*1000); //give ourselves samplingPeriod seconds to make this measurement, during which the "onPulse" function will count up all the pulses, and sum the total time they took as 'totalDuration' 
 
  detachInterrupt(interruptPin); //we've finished sampling, so detach the interrupt function -- don't count any more pulses
  
  
  // get the photodiode reading
  int photocellReading;
  photocellReading = analogRead(photocellPin); 
  
  //turn off the sensorBoard system
  digitalWrite(sensorBoard,LOW); //turns on the 555 timer and thermistor subcircuit
  
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  //Serial.print(steinhart);
  //Serial.println(" *C");
  
 
  //do the conductivity calculations
  
  double freqHertz;
  if (pulseCount>0) { //use this logic in case something went wrong
  
  double durationS=(totalDuration/double(pulseCount))/1000000.; //the total duration, in seconds, per pulse (note that totalDuration was in microseconds)
  freqHertz=1./durationS;
  }
  else {
    freqHertz=0.;
  }
  
  // Onboard temp from the RTC
  float rtcTemp = RTC.getTempAsFloat();
  
  //get the time
  DateTime now = RTC.now();
  long unixNow = now.unixtime();
  
  // Get the battery level
  int batteryLevel = analogRead(BATTERYPIN);
  
  
  
  // make a string for assembling the data to log:
  String dataString = "";
  
  // dataString += String(unixNow);
  dataString += now.unixtime();
  dataString += ",";
  dataString += now.year();
  dataString += "-";
  dataString += padInt(now.month(), 2);
  dataString += "-";
  dataString += padInt(now.day(), 2);
  dataString += " ";
  dataString += padInt(now.hour(), 2);
  dataString += ":";
  dataString += padInt(now.minute(), 2);
  dataString += ":";
  dataString += padInt(now.second(), 2);
  dataString += ",";
  char buffer[10];
  dataString += dtostrf(rtcTemp, 5, 2, buffer);
  dataString += ",";
  dataString += dtostrf(steinhart,5,2,buffer);
  dataString += ",";
  dataString += dtostrf(freqHertz,6,2,buffer);
  dataString += ",";
  dataString += String(photocellReading);
  dataString += ",";
  dataString += String(batteryLevel);

  
  // Open up the file we're going to log to!
  dataFile = SD.open(filename, FILE_WRITE);
  if (!dataFile) {
    if (debug==1){
      Serial.print("Error opening file:");
      Serial.println(filename);
    }
    
    // Wait forever since we cant write data
    while (1) ;
  }
  
   digitalWrite(led, HIGH);
  delay(30);
  
  // Write the string to the card
  dataFile.println(dataString);
  dataFile.close();
  
  digitalWrite(led,LOW);
  
  if (debug==1) {
    Serial.println(dataString);
  }
  
// go to sleep!
   
     if (debug==0) {
       for (int k=0;k<LOG_INTERVAL_REPEAT;k++) {
    Sleepy::loseSomeTime(LOG_INTERVAL_BASE); //-- will interfere with serial, so don't use when debugging 
       }
  } else {
    delay (LOG_INTERVAL_BASE); // use when debugging -- loseSomeTime does goofy things w/ serial
  }
  
   
  
}
  
String padInt(int x, int pad) {
  String strInt = String(x);
  
  String str = "";
  
  if (strInt.length() >= pad) {
    return strInt;
  }
  
  for (int i=0; i < (pad-strInt.length()); i++) {
    str += "0";
  }
  
  str += strInt;
  
  return str;
}

String int2string(int x) {
  // formats an integer as a string assuming x is in 1/100ths
  String str = String(x);
  int strLen = str.length();
  if (strLen <= 2) {
    str = "0." + str;
  } else if (strLen <= 3) {
    str = str.substring(0, 1) + "." + str.substring(1);
  } else if (strLen <= 4) {
    str = str.substring(0, 2) + "." + str.substring(2);
  } else {
    str = "-9999";
  }
  
  return str;
}

void onPulse()
{
  pulseCount++;
  //Serial.print("pulsecount=");
  //Serial.println(pulseCount);
  lastTime = pulseTime;
  pulseTime = micros();
  duration=pulseTime-lastTime;
  totalDuration+=duration;
  //Serial.println(totalDuration);
}
