#include <Arduino.h>
//timer library
#include <Event.h>
#include <Timer.h>
//debounce input buttons
#include <Bounce2.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

/* This is an Ozone Generator 
*/

//define pins

//switches
int boostPin = 3;     //Boost-button, low = active
int startPin = 2;     //Start-button, low = active

//LEDS                
int boostLed = 4;
int fanLed = 5;
int o3Led = 6;


//relays
int fanPin = 10;      //FAN Relay
int fanPWM = 11;      //FAN PWM pin (not relay)
int o3aPin = 7;       //O3-1
int o3bPin = 8;       //O3-2
int emptyRelay = 9;   //Empty relay port

//analog input
int powerPin = A0;
int timerPin = A2;

//LCD pins
//Pins will be A4 & A5

//define runtime variables
boolean systemActive = 0;     //is the system currently active?
int systemActiveID = - 1;     //ID of the cycle timer
boolean boostActive = 0;      //is the boost button active?

unsigned long totalTime = 0;  //how many s has the system been running in total
unsigned long boostTime = 0;  //how many s has the boost been active?

int cycleTime = 0;            //where are we in a 20 s cycle
int onTime = 0;               //how many seconds should we stay on
int totalRunTime = 0;         //how many seconds should we keep the system running?
int boostMaxTime = 600;       //how many [s] should the boost stay on?

//timers
//on some systems I experienced (or produced?) problems with too many entries per timer 
//hence we split vital functions apart

Timer sysTimer1;      //blocking/unblocking buttons             
Timer sysTimer2;      //duty cycle

//debounce buttons
Bounce debounceBoost = Bounce();
Bounce debounceStart = Bounce();

boolean boostBlocked = 0;                                      
boolean startBlocked = 0;      

//Function headers
void boostUnboostSystem();
void unblockBoost();
void startStopSystem();
void unblockStart();
void readAnalogButtons();
void fanOff();
void fanOn();
void dutyCycle();
void o3Off();
void o3On();


void setup() {
  //for debugging
  Serial.begin(9600);

  //initialize pins
  pinMode(boostPin, INPUT_PULLUP);
  pinMode(startPin, INPUT_PULLUP);  
  debounceBoost.attach(boostPin);
  debounceBoost.interval(5); // interval in ms
  debounceStart.attach(startPin);
  debounceStart.interval(5); // interval in ms

  pinMode(boostLed, OUTPUT);  digitalWrite(boostLed, LOW);
  pinMode(fanLed, OUTPUT);  digitalWrite(fanLed, LOW);
  pinMode(o3Led, OUTPUT);  digitalWrite(o3Led, LOW);
  pinMode(fanPin, OUTPUT);  digitalWrite(fanPin, HIGH);     // Must be connected on NC pin
  pinMode(o3aPin, OUTPUT);  digitalWrite(o3aPin, HIGH);
  pinMode(o3bPin, OUTPUT);  digitalWrite(o3bPin, HIGH);
  pinMode(fanPWM, OUTPUT);  digitalWrite(fanPWM, LOW);      //Fan PWM is set to low (duty cycle 0)

 //fan & gen pins are set to HIGH because of NC contact on the Relay Board 


  //init LCD
  lcd.init();
  lcd.backlight();
  lcd.home();lcd.clear();
  lcd.begin(16,2);
  

  Serial.println(F("System ready"));
}

void loop() {
 //update all timers & debounce objects
 sysTimer1.update();
 sysTimer2.update();
 debounceBoost.update();
 debounceStart.update();

 //Check if buttons have been pressed
 if ((boostBlocked == 0) && (debounceBoost.read() == LOW)) 
 { 
    Serial.println(F("Boost-Button was pressed")); 
    boostUnboostSystem(); 
 }
 if ((startBlocked == 0) && (debounceStart.read() == LOW)) 
 { 
   Serial.println(F("Start-Button was pressed")); 
   startStopSystem(); 
 }

 //Check if Boost has to be disabled
 if ((boostActive == 1) && ( systemActive == 1 ) && (boostTime >  boostMaxTime)) 
 { 
   Serial.println(F("Boost timer ended")); 
   boostUnboostSystem(); 
 }
 
}

void boostUnboostSystem(){
  /***
   * Start or Stop Boost Function
   * 
   */

  //After the start of the function block boost button
  // and after 500ms unlock the button
  boostBlocked = 1;
  sysTimer1.after(500, unblockBoost);

  //Activate boost and set timer to zero
  if (boostActive == 0) 
  { 
    boostActive = 1; 
    boostTime = 0; 
    digitalWrite(boostLed, HIGH); 
  }
  else 
  { 
    // deactivate boost
    boostActive = 0; 
    digitalWrite(boostLed, LOW); 
  }
}

void unblockBoost() 
{ 
  boostBlocked = 0; 
}

void startStopSystem(){
  /*
  *
  */
  //unblock Button
  startBlocked = 1;     //Does not let any aditional start
  sysTimer1.after(500, unblockStart);

  //reset boost timer
  boostTime = 0;

  if (systemActive == 0)
  {
    Serial.println(F("Starting System"));

    //Print status on LCD
    lcd.clear();
    lcd.home();
    lcd.print("System starting");


    readAnalogButtons(); 
    systemActive = 1;
    fanOn();
    systemActiveID = sysTimer2.every(1000, dutyCycle);    //call duty cycle every 1000ms
    Serial.print("Timer2 ID: "); Serial.println(systemActiveID);     //print id of the timer
  }
  else
  {
    Serial.println(F("Stopping System"));
    systemActive = 0;
    fanOff();
    o3Off();
    sysTimer2.stop(systemActiveID);    // stop the call to duty cycle
    //disable System

    //Print status on LCD
    lcd.clear();
    lcd.home();
    lcd.print("System stopping");
  }

  
}
void unblockStart() 
{ 
  startBlocked = 0; 
}

void readAnalogButtons()
{

  //Analog values will be between 0 & 1023
  //225 intervals

  int powerVal = analogRead(powerPin);
  int timerVal = analogRead(timerPin);


  //Choose value of ozone generated
  if ((powerVal > 120) && (powerVal < 220)) 
  { onTime = 1; }     //1 g/h
  else if ((powerVal > 270) && (powerVal < 370)) 
  { onTime = 2; }     //2 g/h
  else if ((powerVal > 450) && (powerVal < 550)) 
  { onTime = 5; }     //5 g/h
  else if ((powerVal > 610) && (powerVal < 710)) 
  { onTime = 10; }     //10 g/h
  else if ((powerVal > 790) && (powerVal < 890)) 
  { onTime = 20; }     //20 g/h
  else 
  { onTime = 0; }

  Serial.print(F("Select onTime[s] of: ")); Serial.println(powerVal);
  
  lcd.clear();
  //lcd.setCursor();
  lcd.print({"On time: "});
  //print Grams of O3


  //Choose interval of generator on
  if ((timerVal > 120) && (timerVal < 220)) 
  { totalTime = 10 * 60; }     //10 min
  else if ((timerVal > 270) && (timerVal < 370)) 
  { totalTime = 30 * 60; }     //30 min
  else if ((timerVal > 450) && (timerVal < 550)) 
  { totalTime = 60 * 60; }     //60 min
  else if ((timerVal > 610) && (timerVal < 710)) 
  { totalTime = 120 * 60; }     //120 min
  else if ((timerVal > 790) && (timerVal < 890)) 
  { totalTime = 240 * 60; }     //240 min
  else 
  { totalTime = 0; }

  Serial.print(F("Select totalTime[s] of: ")); Serial.println(timerVal);

  lcd.clear();
  //lcd.setCursor();
  lcd.print({"Total time: "});
  //print total time in min
}

void fanOn(){
  Serial.println(F("The Fans are ON"));
  digitalWrite(fanLed, HIGH);
  digitalWrite(fanPin, LOW);
  digitalWrite(fanPWM, HIGH);      
}
void fanOff(){
  digitalWrite(fanLed, LOW);
  digitalWrite(fanPin, HIGH);
  digitalWrite(fanPWM, LOW);

}

void o3On(){
  Serial.println(F("All O3 generators are ON"));
  digitalWrite(o3Led, HIGH);
  digitalWrite(o3aPin, LOW);
  digitalWrite(o3bPin, LOW);
}
void o3Off(){
  Serial.println(F("All O3 generators are OFF"));
  digitalWrite(o3Led, LOW);
  digitalWrite(o3aPin, HIGH);
  digitalWrite(o3bPin, HIGH);
}

void dutyCycle(){
  /*
    The amount of time the generators are on
  */
  //increment timers
  totalTime = totalTime + 1;

  if (boostActive == 1) { 
    boostTime = boostTime + 1; 
    o3On(); 
    return; 
  }

  //check if cycle time is bigger than the on time selected
  if (cycleTime < onTime) { 
    o3On(); }
  else { 
    o3Off(); }
  
  // increase cycle time value
  cycleTime = cycleTime+1;

  // reset cycle time is it is bigger than 20 s
  if (cycleTime >= 20) { 
    cycleTime = 0; }

  //print generator running 
  lcd.clear();
  lcd.home();
  lcd.print("Generator Running");
}