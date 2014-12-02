//=============================HOW TO USE============================
/*
Below is a code for a thermostat for use in incubators in the developing world. 
It is intended as an automated, self-regulating system and should require little user system interaction. 
It consists mainly of a temperature sensor along with a heat source and cooling fan to allow for bang-bang 
heating control for the environment. Additional features are an alarm system (with buzzer and LEDS), 
LED display (for easy temperature read-out), and data logging features (exporting the Serial data to a .csv 
file for easy plotting and analysis in excel). The user is able to control the desired temperature of the 
incubator, the critical threshold temperature considered too hot for the enclosure, and the alarm frequency 
using sliders controls. Built-in error cases include notifying the user of incorrect threshold settings, 
disconnected thermistor or wiring failures, and automatic timed heating shut-off to prevent dangerous overheating.
*/

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//These pin definitions are for v1.7 of the board
#define SLIDER1  A2 //Sets ambient temperature value for the incubator
#define SLIDER2  A1 //Sets high critical temperature value
#define SLIDER3  A0 //Sets alarm frequency value

#define FAN      2 //Cooling fan
#define BUZZER   3 //buzzer (from DangerShield)
#define LED1     5 //alarm LEDs
#define LED2     6 //
#define HEAT     9 //Space heater attached via Powerswitch tail relay

#include <Wire.h> 
#include "Adafruit_LEDBackpack.h" //library for LED display
#include "Adafruit_GFX.h" //library for LED display
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

unsigned long timerled;  //controls led delay times
unsigned long start_time; 
unsigned long elapsed_time; //creates time stamp for data recording
int initcrit; //set initial condition for critical temperature
boolean led_run1=false; //controls for warning leds
boolean led_run2=true;
boolean heat_flag=false; //flag if the heater is on
int comment=0;  //flags for possible comments/warnings
int comment2=0;
int count=0; //counter for heating safety mechanism

Adafruit_7segment matrix = Adafruit_7segment(); //set up for LED display

void setup()
{ 
  //Initialize inputs and outputs
  pinMode(SLIDER1, INPUT);
  pinMode(SLIDER2, INPUT);
  pinMode(SLIDER3, INPUT);
  pinMode(A3, INPUT); //thermistor 

  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(HEAT, OUTPUT);
  pinMode(FAN, OUTPUT);

  //Set up for LED display
  #ifndef __AVR_ATtiny85__ 
  Serial.begin(9600);//can adjust temp display time by adjusting serial.begin
  #endif
  matrix.begin(0x70);

  initcrit = 0; //The critical condition is not initialized
  start_time = millis(); //initializes the timer
  //Serial.print("Time"); Serial.print(", "); Serial.print("Temperature"); Serial.print(", "); Serial.print("Baby T Threshold"); Serial.print(", "); //Headers
  //Serial.print("T Crit"); Serial.print(", "); Serial.print("Bang-Bang Counter"); Serial.print(", "); Serial.println("Comments");
}

void loop()
{
  delay(1000); //Sets the timing on the void loop
  elapsed_time = (millis() - start_time)/1000;


  //=============================READ INPUTS============================
  int sensorValue = analogRead(A3); //thermistor
  int val1 = analogRead(SLIDER1);
  int val2 = analogRead(SLIDER2);
  int val3 = analogRead(SLIDER3);
  long buzSound = map(val3, 0, 1020, 1000, 200); //mapping frequency tones to slider 3 location
  long lowtemp = map(val1, 0, 1020, 30, 23);     //mapping lower temperature threshold to slider 1 location
  long hightemp = map(val2, 0, 1020, 37, 27);    //mapping higher tempeature threshold to slider 2 location
    
  float tempvolt = sensorValue*(5.0/1023.0);  //convert analog reading to voltage
  //converting voltage to temperature value
  float temperature= 100*(3.044625*pow(10,-5)*pow(tempvolt,6)-(.005209376*pow(tempvolt,5))+0.065699269*pow(tempvolt,4)-0.340695972*pow(tempvolt,3)+0.897136183*pow(tempvolt,2)-1.419855102*tempvolt+1.451672296); //voltage to temp conversion eqn

    
   
  //======================ADDRESSING USER ERROR CASES=====================
  if (lowtemp>hightemp){ //addresses possible user error of incorrect temperature threshold settings  (i.e. the threshold for the upper limit is less than the desired temperature setting)
    comment = 1;
    tone(BUZZER, buzSound); delay(100);
    noTone(BUZZER); delay(30);
    tone(BUZZER, buzSound); delay(100);
    noTone(BUZZER); delay(100);
    }
  else if (temperature < 10 || temperature > 45){ //addresses temperature probe being out of range. Most likely the sensor is unpluged
    comment = 2;
    tone(BUZZER, buzSound);
    delay(100);
    noTone(BUZZER);
    delayMicroseconds(50);
    }
    

   //=============================HEATER CONTROL=============================
  else{
    comment = 0; //no comment  
    if (temperature < lowtemp){    //Baby's temperature is too low
      digitalWrite(HEAT, HIGH);   //heater turns on
      heat_flag = true;
      count++; //safety counter increments
       if (count >25){            //Bang-bang control: if the heater is on for x amount of time
         digitalWrite(HEAT, LOW); //heater automatically turns off after the x amount of time
         heat_flag = !heat_flag;
         if (count >50)           //resets the counter after a certain amount of time
           count = 0;}}
    else{                  //Baby's temperature is just right
      digitalWrite(HEAT, LOW);
      heat_flag = false;}
      
  }


   //======================CRITICAL TEMPERATURE REACHED======================
    if(temperature > hightemp){ //if reaches too hot threshold temp
      tone(BUZZER, buzSound);   //alarm turns on
      digitalWrite(FAN, HIGH);  //turns on fan
      comment2 = 1;
      if (initcrit==0){    //initial case to set up alternating led and timer (timerled)
        timerled=millis(); //built-in overall timer for led delays
        initcrit = 1;      //escapes the initial case from now on
        led_run1 = false;
        led_run2 = true;}  //changes led states for alternating lights
      if (millis()-timerled >= 75UL){  //changes state of leds
        led_run1 = !led_run1;
        digitalWrite(LED1, led_run1);
        led_run2 = !led_run2;
        digitalWrite(LED2, led_run2);
        timerled=millis();}  //resets the timer
    }
    else{
      digitalWrite(FAN, LOW);  //turns off the fan
      digitalWrite(LED1, LOW); //turns off the leds
      digitalWrite(LED2, LOW);
      noTone(BUZZER); //turns off the buzzer
      initcrit = 0;     //resets the initial critical condition
      comment2 = 0;} 


   //=============================PRINT OUTS============================
  //Print to 7-segment Display
  matrix.writeDigitNum(1,floor(temperature/10)); //Tens digit
  matrix.writeDigitNum(3,(int) floor(temperature) % 10,true); //Ones digit, decimal is true
  matrix.writeDigitNum(4,round(temperature*10) % 10);  //Tenths digit (rounded)
  matrix.writeDisplay();
  
  Serial.print(elapsed_time); Serial.print(", ");  
  Serial.print(temperature); Serial.print(", ");
  Serial.print(lowtemp); Serial.print(", ");
  Serial.print(hightemp); Serial.print(", ");
  Serial.print(count); Serial.print(", ");
  Serial.print(heat_flag); Serial.print(", ");
  if(comment == 1)
    Serial.println("WARNING: Incorrect temperature settings! Check temperature threshold values!");
  else if(comment == 2)
    Serial.println("WARNING: Sensor out of range - check wiring!");
  else if(comment2 == 1)
    Serial.println("It's too hot!");
  else
    Serial.println();

}
