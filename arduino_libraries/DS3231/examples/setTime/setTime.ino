#include <Wire.h>
#include "DS3231.h"

DS3231 RTC; //Create the DS3231 object

char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void setup () 
{
	Serial.begin(9600);
	Wire.begin();
	RTC.begin();
	//Real Time Clock, set time with a DateTime Object
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	//Month format: Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
	RTC.adjust(DateTime((__DATE__), (__TIME__))); //Adjust automatically

	//Or manually:
	//year, month, date, hour, minute, second, dayOfWeek(0-6)
	RTC.adjust(DateTime(2011, 11, 10, 15, 18, 0, 5));
}

void loop () 
{
	DateTime now = RTC.now(); //get the current date-time
	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.date(), DEC);
	Serial.print(' ');
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
	Serial.print(weekDay[now.dayOfWeek()]);
	Serial.println();
	Serial.println(now.unixtime());
	delay(1000);
}
