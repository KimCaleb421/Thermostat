// Shift register bit values to display a cycling line segment when the alarm sounds
const byte ledCharSet[10] = {
 B00000001,
  B00000010,
  B00000100,
  B00001000,
  B00010000,
  B00100000 
};

// Pin definitions

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//These pin definitions are for v1.6 of the board
/*
#define SLIDER1  A0
 #define SLIDER2  A1
 #define SLIDER3  A2
 #define LIGHT    A3
 #define TEMP     A4
 
 
 #define DATA     4
 #define LED1     5
 #define LED2     6
 #define LATCH    7
 #define CLOCK    8
 #define BUZZER   9
 #define BUTTON1  10
 #define BUTTON2  11
 #define BUTTON3  12
 //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//These pin definitions are for v1.7 of the board
#define SLIDER1  A2 //Matches button 1
#define SLIDER2  A1 
#define SLIDER3  A0 //Matches button 3
#define LEDSCL   A5
#define LEDSDA   A4

#define FAN      2
#define BUZZER   3
#define DATA     4
#define LED1     5
#define LED2     6
#define LATCH    7
#define CLOCK    8
#define HEAT     9
#define BUTTON1  10
#define BUTTON2  11
#define BUTTON3  12
//v1.7 uses CapSense
//This relies on the Capactive Sensor library here: http://playground.arduino.cc/Main/CapacitiveSensor
#include <CapacitiveSensor.h>
#include <Wire.h> 
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

CapacitiveSensor capPadOn92 = CapacitiveSensor(9, 2);   //Use digital pins 2 and 9,
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

unsigned long timerled; //controls led delay times
int initcrit; //set initial condition for 
boolean led_run1=false; //controls for leds
boolean led_run2=true;
boolean test = true;
int count=0; //counter for heating safety mechanism
Adafruit_7segment matrix = Adafruit_7segment();
void setup()
{
  #ifndef __AVR_ATtiny85__
  Serial.println("7 Segment Backpack Test");
  #endif
  matrix.begin(0x70);
  Serial.begin(9600);

  //Initialize inputs and outputs
  pinMode(SLIDER1, INPUT);
  pinMode(SLIDER2, INPUT);
  pinMode(SLIDER3, INPUT);
  //pinMode(LIGHT, INPUT);
  //pinMode(TEMP, INPUT);
  pinMode(A3, INPUT); //thermistor 

  //Enable internal pullups
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(HEAT, OUTPUT);
  pinMode(FAN, OUTPUT);

  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);

  //Initialize the capsense
  capPadOn92.set_CS_AutocaL_Millis(0xFFFFFFFF); // Turn off autocalibrate on channel 1 - From Capacitive Sensor example sketch

  Serial.println("Danger Shield Component Test");  

  //Take 16 readings from the light sensor and average them together
 /* avgLightLevel = 0;
  for(int x = 0 ; x < 16 ; x++)
    avgLightLevel += analogRead(LIGHT);
  avgLightLevel /= 16;
  Serial.print("Avg: ");
  Serial.println(avgLightLevel);*/
 initcrit = 0;
}

void loop()
{
  //Read inputs
  int sensorValue = analogRead(A3); //thermistor
  int val1 = analogRead(SLIDER1);
  int val2 = analogRead(SLIDER2);
  int val3 = analogRead(SLIDER3);

 // int lightLevel = analogRead(LIGHT);
 // long temperature = analogRead(TEMP);

  //Read the cap sense pad
//  long capLevel = capPadOn92.capacitiveSensor(30);
 /* 
  //Do conversion of temp from analog value to digital
  temperature *= 5000; //5V is the same as 1023 from the ADC (12-bit) - example, the ADC returns 153 * 5000 = 765,000
  temperature /= 1023; //It's now in mV - 765,000 / 1023 = 747
  //The TMP36 reports 500mV at 0C so let's subtract off the 500mV offset
  temperature -= 500; //747 - 500 = 247
  //Temp is now in C where 247 = 24.7
/*
  //Display values
  //sprintf is a great way to combine a bunch of variables and strings together into one print statement
  char tempString[200];
  sprintf(tempString, "Sliders: %04d %04d %04d Light: %04d Capsense: %04d", val1, val2, val3, lightLevel, capLevel);
  Serial.print(tempString); 
  sprintf(tempString, " Temp: %03d.%01dC", (int)temperature/10, (int)temperature%10);
  Serial.print(tempString); 
 */
  delay(500);
  //convert analog reading to voltage
  float tempvolt = sensorValue*(5.0/1023.0);
  //converting voltage to temperature value
  //float temperature = ((2*tempvolt/5)-.8)/-.21;
  float temperature= 100*(3.044625*pow(10,-5)*pow(tempvolt,6)-(.005209376*pow(tempvolt,5))+0.065699269*pow(tempvolt,4)-0.340695972*pow(tempvolt,3)+0.897136183*pow(tempvolt,2)-1.419855102*tempvolt+1.451672296); //voltage to temp conversion eqn
  matrix.print(temperature);
  matrix.writeDisplay();
  Serial.println(temperature);

//flashes LED alerts when temperature passes a set threshold value  
  if (temperature < 27){ //if temp is too low
    digitalWrite(HEAT, HIGH); //heater turns on
    count++; //safety counter starts
    Serial.print(count);
    Serial.print(", ");
    if (count >25){ //if the heater is on for x amount of time
      digitalWrite(HEAT, LOW); //heater automatically turns off after the x amount of time
      if (count >50) //resets the counter after a certain amount of time
        count = 0;}}
  else
    digitalWrite(HEAT, LOW);

  if(temperature > 29) //if reaches too hot threshold temp
  {
    tone(BUZZER, 262); //alarm turns on
    Serial.print(" It's too hot!");
    digitalWrite(FAN, HIGH); //turns on fan
    if (initcrit==0){ //initial case to set up led differences and overall timer (timerled)
      timerled=millis(); //built-in overall timer for led delays
      initcrit = 1;
      led_run2 = true;} //changes led states for alternating lights
    if (millis()-timerled >= 75UL){  //changes state of leds
      led_run1 = !led_run1;
      digitalWrite(LED1, led_run1);
      led_run2 = !led_run2;
      digitalWrite(LED2, led_run2);
      timerled=millis();} //sets up timer for small, led delays
      //Serial.print(led_run2);
            
  }
  else{
    digitalWrite(FAN, LOW); //turns off the fan
    digitalWrite(LED1, LOW); //turns off the leds
    digitalWrite(LED2, LOW);
    noTone(BUZZER); //turns off the buzzer
    initcrit=0;} //resets the initial critical condition

 /* Serial.println();

  //Set the brightness on LED #2 (D6) based on slider 1
  int ledLevel = map(val1, 0, 1020, 0, 255); //Map the slider level to a value we can set on the LED
  analogWrite(LED2, ledLevel); //Set LED brightness

  //Set 7 segment display based on the 2nd slider
  int numToDisplay = map(val2, 0, 1020, 0, 9); //Map the slider value to a displayable value
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, ~(ledCharSet[numToDisplay]));
  digitalWrite(LATCH, HIGH);

  //Set the sound based on the 3rd slider
  long buzSound = map(val3, 0, 1020, 1000, 10000); //Map the slider value to an audible frequency
  if(buzSound > 1100)
    tone(BUZZER, buzSound); //Set sound value
  else
    noTone(BUZZER);

  //If light sensor is less than 3/4 of the average (covered up) then freak out
  while(lightLevel < (avgLightLevel * 3 / 4))
  {
    //Blink the status LEDs back and forth
    if(digitalRead(LED1) == LOW)
    {
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
    }
    else {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
    }

    //Display an increasing number on the 7-segment display
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, MSBFIRST, ~(ledCharSet[numToDisplay]));
    digitalWrite(LATCH, HIGH);
    numToDisplay++; //Goto next number
    if(numToDisplay > 9) numToDisplay = 0; //Loop number

    //Play a horrendously annoying sound
    //Frequency increases with each loop
    tone(BUZZER, buzSound);
    buzSound += 100;
    if(buzSound > 3000) buzSound = 1000;

    delay(25);

    lightLevel = analogRead(LIGHT); //We need to take another reading to be able to exit the while loop
  }*/
}


//Serial.print("blah blah:" + variable);
//Serial.println(variable);
//Serial.


