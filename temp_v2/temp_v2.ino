//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//These pin definitions are for v1.7 of the board
#define SLIDER1  A2 //Matches button 1
#define SLIDER2  A1 
#define SLIDER3  A0 //Matches button 3
//#define LEDSCL   A5
//#define LEDSDA   A4

#define FAN      2
#define BUZZER   3
#define LED1     5
#define LED2     6
#define HEAT     9

#include <Wire.h> 
//#include <math.h>
#include "Adafruit_LEDBackpack.h" //library for LED display
#include "Adafruit_GFX.h" //library for LED display
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

unsigned long timerled; //controls led delay times
int initcrit; //set initial condition for critical temperature
boolean led_run1=false; //controls for warning leds
boolean led_run2=true;
int count=0; //counter for heating safety mechanism
int comment=0;
int comment2=0;
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
}

void loop()
{
  delay(500); //Sets the timing on the void loop

  //Read inputs
  int sensorValue = analogRead(A3); //thermistor
  int val1 = analogRead(SLIDER1);
  int val2 = analogRead(SLIDER2);
  int val3 = analogRead(SLIDER3);
  long buzSound = map(val3, 0, 1020, 1000, 200); //mapping frequency tones to slider 3 location
  long lowtemp = map(val1, 0, 1020, 30, 23);     //mapping lower temperature threshold to slider 1 location
  long hightemp = map(val2, 0, 1020, 37, 27);    //mapping higher tempeature threshold to slider 2 location
  
  
  //convert analog reading to voltage
  float tempvolt = sensorValue*(5.0/1023.0);
  //converting voltage to temperature value
  float temperature= 100*(3.044625*pow(10,-5)*pow(tempvolt,6)-(.005209376*pow(tempvolt,5))+0.065699269*pow(tempvolt,4)-0.340695972*pow(tempvolt,3)+0.897136183*pow(tempvolt,2)-1.419855102*tempvolt+1.451672296); //voltage to temp conversion eqn
  
  //matrix.print(Math.round((temperature*10)/10));
  matrix.print(temperature);   //output measured temp on LED display
  matrix.writeDisplay();
  

   
  //======================ADDRESSING USER ERROR CASES=====================
  if (lowtemp>hightemp){ //addresses possible user error of incorrect temperature threshold settings
    //Serial.print(temperature);Serial.print(", ");
    //Serial.println("WARNING: Incorrect temperature settings! Check temperature threshold values!");
    comment = 1;
    tone(BUZZER, buzSound); delay(100);
    noTone(BUZZER); delay(30);
    tone(BUZZER, buzSound); delay(100);
    noTone(BUZZER); delay(100);
    }
  else if (temperature < 10 || temperature > 45){ //addresses temperature probe being out of range. Most likely the sensor is unpluged
    comment = 2;
    //Serial.print(temperature);Serial.print(", ");
    //Serial.println("WARNING: Sensor out of range - check wiring!");
    tone(BUZZER, buzSound);
    delay(100);
    noTone(BUZZER);
    delayMicroseconds(50);
    }

   //=============================HEATER CONTROL=============================
  else{
    comment = 0; //no comment  
   // if (temperature > lowtemp && temperature < hightemp){   //I don't think we need these two lines
   // Serial.println(temperature); }
    if (temperature < lowtemp){    //Baby's temperature is too low
      //Serial.println(temperature); 
      digitalWrite(HEAT, HIGH);   //heater turns on
    count++;} //safety counter increments
    //Serial.print(count);                    //THIS NEEDS TO GO SOMEWHERE ELSE
    //Serial.print(", ");
    if (count >25){            //if the heater is on for x amount of time
      digitalWrite(HEAT, LOW); //heater automatically turns off after the x amount of time
      if (count >50)           //resets the counter after a certain amount of time
        count = 0;}
    else                  //Baby's temperature is just right
      digitalWrite(HEAT, LOW);
  }


   //======================CRITICAL TEMPERATURE REACHED======================
    if(temperature > hightemp){ //if reaches too hot threshold temp
      tone(BUZZER, buzSound);   //alarm turns on
      comment2 = 1;
      //Serial.print(temperature); Serial.print(", "); Serial.println("It's too hot!");
      digitalWrite(FAN, HIGH);  //turns on fan
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

  Serial.print(temperature); Serial.print(", ");
  Serial.print(lowtemp); Serial.print(", ");
  Serial.print(hightemp); Serial.print(", ");
  Serial.print(count); Serial.print(", ");
  if(comment == 1)
    Serial.println("WARNING: Incorrect temperature settings! Check temperature threshold values!");
  else if(comment == 2)
    Serial.println("WARNING: Sensor out of range - check wiring!");
  else if(comment2 == 1)
    Serial.println("It's too hot!");
  else
    Serial.println();

}
