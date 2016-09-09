/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

---
*  Modified for Riffle 0.96 
*  By Don Blair
*  July 2015
*  
*  This code tests Riffle's SD card reader AND Real Time Clock
*    
*  It writes time and temperature to datalog.txt on a formatted SD card
*  Like: 1437331087,2015-07-19 18:38:07,30.50
*  Which is utc time, YYYY-MM-DD HH:MM:SS,onboard temp in °C
*  Creates datalog.txt if it doesn't exist
*  
*  Also, we've found that running the serial monitor with this sketch can make
*  things buggy, so those lines are commented out
*  
 */
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include <RTClib.h>
#include <RTC_DS3231.h>


int led = 9;

const int chipSelect = 7;
int SDpower = 6;

//RTC
RTC_DS3231 RTC;

void setup()
{
  

 // Serial.print("Initializing SD card...");

  
 // Serial.println("card initialized.");
  
   pinMode(SDpower,OUTPUT);
    digitalWrite(SDpower,LOW);
   delay(1000);
   
   // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
  //  Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
   
   
   //check on RTC
   
       // for i2c for RTC
  Wire.begin();
  RTC.begin();   
      
    // check on the RTC
  if (! RTC.isrunning()) {
      // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
   
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  
}

void loop()
{
 


//rtc stuff

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
 
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
   // Serial.println(dataString);
     
  digitalWrite(led, HIGH);   
  delay(20);
  digitalWrite(led, LOW);
  
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("error opening datalog.txt");
  }
  
  
delay (10000);

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