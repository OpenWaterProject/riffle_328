#include <Wire.h>
#include <SPI.h>
#include "SdFat.h"    //  https://github.com/greiman/SdFat
#include "EnableInterrupt.h"  //  https://github.com/GreyGnome/EnableInterrupt
#include "DS3231.h"   //  https://github.com/kinasmith/DS3231
#include "LowPower.h"   //  https://github.com/rocketscream/Low-Power

DS3231 rtc; //initialize the Real Time Clock

// SdFat variables
SdFat sd;
SdFile myFile;

const int DEBUG = 1; //Enable or disable Serial Printing with this line. 1 is enabled, 0 is disabled.  If enabled, the RTC time will also be set.

const int led = 9; //Feedback LED
const int bat_v_pin = A3;
const int bat_v_enable = 4; //enable pin for bat. voltage read
const int rtc_int = 5; //rtc interrupt pin
const int sd_pwr_enable = 6; //enable pin for SD power
const int hdr_pwr_enable = 8; //enable pin for header power
const int chipSelect = 7; //SPI Chip Select for SD Card

int interval_sec = 15; //Logging interval in seconds
float bat_v;
float temp;

void setup() {


  if (DEBUG) Serial.begin(9600); // only set up Serial if in DEBUG mode

  // enable any I2C devices
  Wire.begin();

  // start the RTC
  rtc.begin();

  // prepare the RTC hardware for the interrupt functionality
  pinMode(rtc_int, INPUT_PULLUP); 

  // if in DEBUG mode, adjust the RTC time using the system clock
  if (DEBUG) rtc.adjust(DateTime((__DATE__), (__TIME__)));

  // set the led blinking pin to OUTPUT mode
  pinMode(led, OUTPUT);

  // enable the SD card for writing
  pinMode(chipSelect, OUTPUT);

  // turn off the battery monitor circuitry for now, to save power
  pinMode(bat_v_enable, OUTPUT);
  digitalWrite(bat_v_enable, HIGH); //Turn off Battery Reading

  // put the SD card powering pin in the proper mode
  pinMode(sd_pwr_enable, OUTPUT);
}

void loop() {


  // get the current time from the RTC
  DateTime now = rtc.now();

  // set up the next wake time 
  DateTime nextAlarm = DateTime(now.unixtime() + interval_sec);

  // print out, if in DEBUG mode
  if (DEBUG) {
    Serial.print("Now: ");
    Serial.print(now.unixtime());
    Serial.print(" Alarm Set for: ");
    Serial.println(nextAlarm.unixtime());
    Serial.flush();
  }

  // get the battery voltage  
  bat_v = getBat_v(bat_v_pin, bat_v_enable); //takes 20ms

   // print out, if in DEBUG mode
  if (DEBUG) {
    Serial.print("Battery Voltage is: ");
    Serial.print(bat_v);
    Serial.println(" Volts.");
    Serial.flush();
  }

  // get the RTC temp sensor value 
  rtc.convertTemperature(); //prep temp registers from RTC
  temp = rtc.getTemperature(); //Read that value

  // print out, if in DEBUG mode
  if (DEBUG) {
    Serial.print("RTC Temp is: ");
    Serial.print(temp);
    Serial.println(" C.");
    Serial.flush();
  }

  // write to SD card 
  writeToSd(now.unixtime(), temp, bat_v);

  // print out, if in DEBUG mode
  if (DEBUG) {
    Serial.print("SD Card Written. Sleeping for ");
    Serial.print(interval_sec);
    Serial.print(" seconds.");
    Serial.println();
    Serial.println("---------------------------------");
    Serial.flush();
  }

  // sleep in low power mode  
  enterSleep(nextAlarm); //Sleep until next scheduled wakeup time
}

////////// Support functions

// Waking the MCU from sleep on Alarm Pin Change from RTC
void rtc_interrupt() {
  disableInterrupt(rtc_int); //first it Disables the interrupt so it doesn't get retriggered
}

// Turns off SD Card Power, Sets Wake Alarm and Interrupt, and Powers down the MCU
void enterSleep(DateTime& dt) { //argument is Wake Time as a DateTime object
  delay(50); //Wait for file writing to finish. 10ms works somethings, 50 is more stable
  digitalWrite(sd_pwr_enable, HIGH); //Turn off power to SD Card
  delay(100); //wait for SD Card to power down was 100ms
  rtc.clearAlarm(); //resets the alarm interrupt status on the rtc
  enableInterrupt(rtc_int, rtc_interrupt, FALLING); //enables the interrupt on Pin5
  rtc.enableAlarm(dt); //Sets the alarm on the rtc to the specified time (using the DateTime Object passed in)
  delay(1); //wait for a moment for everything to complete
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //power down everything until the alarm fires
}

//returns battery voltage 
float getBat_v(byte p, byte en) {
  float v;
  digitalWrite(en, LOW); //write mosfet low to enable read
  delay(10); //wait for it to settle
  v = analogRead(p); //read voltage
  delay(10); //wait some more...for some reason
  digitalWrite(en, HIGH); //disable read circuit
  v = (v * (3.3 / 1024.0)) * 2.0; //calculate actual voltage
  return v;
}

//Blinks a digital Pin
///////////////////////////////////////////////////
void blink(byte PIN, int DELAY_MS) {
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
  delay(DELAY_MS);
}

//Powers on SD Card, and records the give values into "data.csv"
//Notes: The delay times are important. The SD Card initializations
//     will fail if there isn't enough time between writing and sleeping
void writeToSd(long t, float v, float temp) {
  digitalWrite(led, HIGH); //LED ON, write cycle start
  /**** POWER ON SD CARD ****/
  digitalWrite(sd_pwr_enable, LOW); //Turn power to SD Card On
  delay(100); //wait for power to stabilize (!!) 10ms works sometimes
  /**** INIT SD CARD ****/
  if (DEBUG) Serial.print("SD Card Initializing...");
  if (!sd.begin(chipSelect)) {  //init. card
    if (DEBUG) Serial.println("Failed!");
    while (1); //if card fails to init. the led will stay lit.
  }
  if (DEBUG) Serial.println("Success");
  /**** OPEN FILE ****/
  if (DEBUG) Serial.print("File Opening...");
  if (!myFile.open("data.csv", O_RDWR | O_CREAT | O_AT_END)) {  //open file
    if (DEBUG) Serial.println("Failed!");
    while (1);
  }
  if (DEBUG) Serial.println("Success");
  /**** WRITE TO FILE ****/
  myFile.print(t);
  myFile.print(",");
  myFile.print(temp);
  myFile.print(",");
  myFile.print(v);
  myFile.println();
  myFile.close();
  digitalWrite(led, LOW); //LED will stay on if something broke
}