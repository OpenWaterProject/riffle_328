/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 
 ----
 
  *  Modified for Riffle 0.96 
  *  By Don Blair
  *  July 2015
  *  
  *  This is a basic test, which blinks Riffle's blue LED quickly
  *  and also sends randomish values to the Serial Monitor
  *  
  *  Values should look like:
  *    23
  *    32
  *    44
  *    12
  *  
 */

int led = 9;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(led, OUTPUT);     
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.println(sensorValue);
  
  delay(1);        // delay in between reads for stability
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(100);               // wait for a second
  
}