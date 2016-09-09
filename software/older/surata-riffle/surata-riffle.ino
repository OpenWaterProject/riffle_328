nclude <JeeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include<stdlib.h>
#include <SD.h>

//sleeping stuff
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

//RTC stuff
RTC_DS3231 RTC;

//led
#define led 9

//battery stuff
#define batteryAnalogMeasurePin A3
#define batteryReadCircuitSwitch 4


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


// debugging -- only do Serial output if debuggin
#define debug 1 // 0: don't print anything out; 1: print out debugging statements

// how long to sleep between measurements
#define sleepSeconds 3

int samples[NUMSAMPLES];

int interruptPin = 1; //corresponds to D2

// conductivity variables

long pulseCount = 0;  //a pulse counter variable

unsigned long pulseTime,lastTime, duration, totalDuration;

int samplingPeriod=3; // the number of seconds to measure 555 oscillations

int sensorBoard = 8; // the pin that powers the 555 subcircuit

const int chipSelect = 7;
int SDpower = 6;


void setup() {
  
  pinMode(SDpower,OUTPUT);
    digitalWrite(SDpower,LOW);
    
     if (!SD.begin(chipSelect)) {
  //  Serial.println("Card failed, or not present");
    // don't do anything more:
    for (int j=0;j<20;j++) {
      digitalWrite(led,HIGH);
      delay(10);
      digitalWrite(led,LOW);
      
    }
  }
  
  if (debug) Serial.begin(115200);
  
  // begin I2C protocol (necessary for RTC, and any other I2C on board
  Wire.begin();

  // RTC -------------------------
  initialize_RTC(); // NOTE: need to initialize I2C first -- but also for any other I2C library
  
  // set mode for battery circuit control pin, and turn the circuit off
  pinMode(batteryReadCircuitSwitch,OUTPUT);
  pinMode(sensorBoard,OUTPUT);
  digitalWrite(batteryReadCircuitSwitch, HIGH);
  
  pinMode(led, OUTPUT);     

pinMode(sensorBoard,OUTPUT); //turns on the 555 timer and thermistor subcircuit
  digitalWrite(sensorBoard,LOW); //turns on the 555 timer and thermistor subcircuit

}

void loop () {
  
  uint8_t i;
  
  //measure the battery 
  digitalWrite(batteryReadCircuitSwitch, LOW); //turn on battery measurement circuit
  int batteryLevel = analogRead(batteryAnalogMeasurePin);
  digitalWrite(batteryReadCircuitSwitch, HIGH);
 
 
 //turn on the sensorBoard system
  
 //measure temp 
 
 //take temp samples
 // take N samples in a row, with a slight delay
 for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   //Serial.println(analogRead(THERMISTORPIN));
   delay(10);
  }
  
  //measure conductivity
  // conductivity --------------------------------------
  
  pulseCount=0; //reset the pulse counter
  totalDuration=0;  //reset the totalDuration of all pulses measured
  
  attachInterrupt(interruptPin,onPulse,RISING); //attach an interrupt counter to interrupt pin 1 (digital pin #3) -- the only other possible pin on the 328p is interrupt pin #0 (digital pin #2)
  
  pulseTime=micros(); // start the stopwatch
  
  delay(samplingPeriod*1000); //give ourselves samplingPeriod seconds to make this measurement, during which the "onPulse" function will count up all the pulses, and sum the total time they took as 'totalDuration' 
 
  detachInterrupt(interruptPin); //we've finished sampling, so detach the interrupt function -- don't count any more pulses
  
  
  //turn off the remote board
  
//digitalWrite(sensorBoard,HIGH); //turns on the 555 timer and thermistor subcircuit

  
//analyze temp
  float average;
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
  steinhart -= 273.15;   
  
  
   float freqHertz;
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
  dataString += String(batteryLevel);
  
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
   if(debug) Serial.println(dataString);
     
  digitalWrite(led, HIGH);   
  delay(20);
  digitalWrite(led, LOW);
  
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("error opening datalog.txt");
  }
  
  
  go_to_sleep_seconds(sleepSeconds); //
       
}
  
  void go_to_sleep_seconds(int seconds) {
  
  int LOG_INTERVAL_BASE = 1000; // 1 sec
  
  for (int k=0;k<seconds;k++) {
    Sleepy::loseSomeTime(LOG_INTERVAL_BASE); //-- will interfere with serial, so don't use when debugging 
   }
  
  
}

// RTC functions

int initialize_RTC() {
  
  
  RTC.begin();   
      
    // check on the RTC
  if (! RTC.isrunning()) {
    
      if (debug) Serial.println("RTC is NOT running!");
    
      // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
   
   if(debug) Serial.println("RTC is older than compile time! Updating");
    
    RTC.adjust(DateTime(__DATE__, __TIME__));
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
  
  
  
