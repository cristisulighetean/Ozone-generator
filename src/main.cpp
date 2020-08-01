#include <Arduino.h>
//timer library
#include <Event.h>
#include <Timer.h>
//debounce input buttons
#include <Bounce2.h>


//define pins
//switches
int boostPin = 2;     //Boost-button, low = active
int startPin = 4;     //Start-button, low = active
//LEDS                //LEDS
int boostLed = 3;
int fanLed = 5;
int o3aLed = 6;
int o3bLed = 7;
//relays
int fanPin = 13;      //FAN
int empytRelay = 12;  //empty
int o3aPin = 11;      //O3-1
int o3bPin = 10;      //O3-2
//analog input
int powerPin = A5;
int timerPin = A3;

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
  pinMode(o3aLed, OUTPUT);  digitalWrite(o3aLed, LOW);
  pinMode(o3bLed, OUTPUT);  digitalWrite(o3bLed, LOW);
  pinMode(fanPin, OUTPUT);  digitalWrite(fanPin, HIGH);
  pinMode(o3aPin, OUTPUT);  digitalWrite(o3aPin, HIGH);
  pinMode(o3bPin, OUTPUT);  digitalWrite(o3bPin, HIGH);
  pinMode(fanPin, OUTPUT);  digitalWrite(fanPin, HIGH);

  //sysTimer1.every(1000, incTime);
  
  Serial.println(F("System ready"));
}

void loop() {
 //update all timers
 sysTimer1.update();
 sysTimer2.update();
 debounceBoost.update();
 debounceStart.update();

 //Check if buttons have been pressed
 if ((boostBlocked == 0) && (debounceBoost.read() == LOW)) { Serial.println(F("Boost-Button was pressed")); boostUnboostSystem(); }
 if ((startBlocked == 0) && (debounceStart.read() == LOW)) { Serial.println(F("Start-Button was pressed")); startStopSystem(); }

 //Check if Boost has to be disabled
 if ((boostActive == 1) && ( systemActive == 1 ) && (boostTime >  boostMaxTime)) { Serial.println(F("Boost timer ended")); boostUnboostSystem(); }
 
}

void boostUnboostSystem(){
  boostBlocked = 1;
  sysTimer1.after(500, unblockBoost);
  if (boostActive == 0) { 
    boostActive = 1; boostTime = 0; digitalWrite(boostLed, HIGH); }
  else { 
    boostActive = 0; digitalWrite(boostLed, LOW); }
}
void unblockBoost() { boostBlocked = 0; }

void startStopSystem(){
  //unblock Button
  startBlocked = 1;
  sysTimer1.after(500, unblockStart);

  //reset boost timer
  boostTime = 0;

  if (systemActive == 0)
  {
    Serial.println(F("Starting System"));
    readAnalogButtons();
    systemActive = 1;
    fanOn();
    systemActiveID = sysTimer2.every(1000, dutyCycle);
    Serial.print("Timer2 ID: "); Serial.println(systemActiveID);
  }
  else
  {
    Serial.println(F("Stopping System"));
    systemActive = 0;
    fanOff();
    o3Off();
    sysTimer2.stop(systemActiveID);
    //disable System
  }

  
}
void unblockStart() { startBlocked = 0; }

void readAnalogButtons()
{
  int powerVal = analogRead(powerPin);
  int timerVal = analogRead(timerPin);

  if ((powerVal > 120) && (powerVal < 220)) { onTime = 1; }     //1 g/h
  else if ((powerVal > 270) && (powerVal < 370)) { onTime = 2; }     //2 g/h
  else if ((powerVal > 450) && (powerVal < 550)) { onTime = 5; }     //5 g/h
  else if ((powerVal > 610) && (powerVal < 710)) { onTime = 10; }     //10 g/h
  else if ((powerVal > 790) && (powerVal < 890)) { onTime = 20; }     //20 g/h
  else { onTime = 0; }
  Serial.print(F("Select onTime[s] of: ")); Serial.println(onTime);

  if ((timerVal > 120) && (timerVal < 220)) { totalTime = 10 * 60; }     //10 min
  else if ((timerVal > 270) && (timerVal < 370)) { totalTime = 30 * 60; }     //30 min
  else if ((timerVal > 450) && (timerVal < 550)) { totalTime = 60 * 60; }     //60 min
  else if ((timerVal > 610) && (timerVal < 710)) { totalTime = 120 * 60; }     //120 min
  else if ((timerVal > 790) && (timerVal < 890)) { totalTime = 240 * 60; }     //240 min
  else { totalTime = 0; }
  Serial.print(F("Select totalTime[s] of: ")); Serial.println(totalTime);
}

void fanOn(){
  digitalWrite(fanLed, HIGH);
  digitalWrite(fanPin, LOW);
}
void fanOff(){
  digitalWrite(fanLed, LOW);
  digitalWrite(fanPin, HIGH);
}

void o3On(){
  Serial.println(F("All ON"));
  digitalWrite(o3aLed, HIGH);
  digitalWrite(o3bLed, HIGH);
  digitalWrite(o3aPin, LOW);
  digitalWrite(o3bPin, LOW);
}
void o3Off(){
  Serial.println(F("All OFF"));
  digitalWrite(o3aLed, LOW);
  digitalWrite(o3bLed, LOW);
  digitalWrite(o3aPin, HIGH);
  digitalWrite(o3bPin, HIGH);
}

void dutyCycle(){
  //increment timers
  totalTime = totalTime + 1;
  if (boostActive == 1) { 
    boostTime = boostTime + 1; 
    o3On(); 
    return; }

  if (cycleTime < onTime) { 
    o3On(); }
  else { 
    o3Off(); }

  cycleTime = cycleTime+1;
  if (cycleTime >= 20) { 
    cycleTime = 0; }
}