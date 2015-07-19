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
*  This code tests Riffle's SD card reader
*    
*  It writes three randomish values to datalog.txt on a formatted SD card
*  ie: 234,253,302
*  Creates datalog.txt if it doesn't exist
*  
*  Also, we've found that running the serial monitor with this sketch can make
*  things buggy

 */

#include <SPI.h>
#include <SD.h>

int led = 9;

const int chipSelect = 7;
int SDpower = 6;


void setup()
{
  
   pinMode(SDpower,OUTPUT);
   digitalWrite(SDpower,LOW);
   delay(1000);
   
 // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    // don't do anything more:
    return;
  }
   
}

void loop()
{
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  
  }
  // if the file isn't open, pop up an error:
  else {
    // Need to add an error indicator. Maybe fast-blink
  }
  

}