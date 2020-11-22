#include <Arduino.h>

//timer library
#include <Event.h>
#include <Timer.h>

//debounce input buttons
#include <Bounce2.h>

//LCD library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//LCD object
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//define pins

//switches
int boostPin = 3;     //Boost-button, low = active
int startPin = 2;     //Start-button, low = active

//LEDS                
int boostLed = 6;
int o3Led = 5;
int GenOnLed = 4;


//relays
int o3aPin = 7;          //O3-1
int o3bPin = 8;          //O3-2
int emptyRelay = 9;      //Empty relay port
int emptyRelay2 = 10;     //Empty relay

//Fan pins
int fanPWM1 = 11;        //FAN PWM pin 
int fanPWM2 = 12;        //Fan PWM pin

//analog input
int powerPin = A0;
int timerPin = A1;

//LCD pins
//Pins will be A4 & A5

//define runtime variables
boolean systemActive = 0;     //is the system currently active?
int systemActiveID = - 1;     //ID of the cycle timer
boolean boostActive = 0;      //is the boost button active?
int lcdTimerID = -1;          //ID of the LCD timer

unsigned long boostTime = 0;  //how many s has the boost been active?

int cycleTime = 0;            //where are we in a 20 s cycle
int GenOnTime= 0;             //how many seconds should we stay on
int currentTimeOn = 0;        //current time on
int boostMaxTime = 600;       //how many [s] should the boost stay on?
//timers
//on some systems I experienced (or produced?) problems with too many entries per timer 
//hence we split vital functions apart


Timer sysTimer1;      //blocking/unblocking buttons             
Timer sysTimer2;      //duty cycle
Timer sysTimer3;      //lcd selecting values


//debounce buttons
Bounce debounceBoost = Bounce();
Bounce debounceStart = Bounce();

boolean boostBlocked = 0;                                      
boolean startBlocked = 0;      

//local varibles for display
int gramsSelected{};
int totalTimeSelected{};
bool LCDDone{false};  //bool to know if template was printed


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
void selectValues();
void displayTemplate();


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
  pinMode(GenOnLed, OUTPUT);  digitalWrite(GenOnLed, LOW);
  pinMode(o3Led, OUTPUT);  digitalWrite(o3Led, LOW);
  pinMode(o3aPin, OUTPUT);  digitalWrite(o3aPin, HIGH);
  pinMode(o3bPin, OUTPUT);  digitalWrite(o3bPin, HIGH);
  pinMode(fanPWM1, OUTPUT);  analogWrite(fanPWM1, 0);  
  pinMode(fanPWM2, OUTPUT);  analogWrite(fanPWM2, 0);   
 
  //init LCD
  lcd.init();
  lcd.backlight();
  lcd.home();lcd.clear();

  
  Serial.println(F("System ready"));
}

void loop() {
 //update all timers & debounce objects
 sysTimer1.update();
 sysTimer2.update();
 sysTimer3.update();
 debounceBoost.update();
 debounceStart.update();

 //Boost Function conditions
 if ((boostBlocked == 0) && (debounceBoost.read() == LOW)) 
 { 
    Serial.println(F("Boost-Button was pressed")); 
    boostUnboostSystem(); 
 }

 //Start Stop Condtions
 if ((startBlocked == 0) && (debounceStart.read() == LOW)) 
 { 
   Serial.println(F("Start-Button was pressed")); 
   startStopSystem(); 
 }

if(systemActive == 1 && currentTimeOn >= totalTimeSelected){
  Serial.println("System finished operation");
  startStopSystem();
}


 //Check if Boost has to be disabled
 if ((boostActive == 1) && ( systemActive == 1 ) && (boostTime >  boostMaxTime)) 
 { 
   Serial.println(F("Boost timer ended")); 
   boostUnboostSystem(); 
 }

 //LCD Update
  if (systemActive == 0 && lcdTimerID == -1){
    //call a function to write to lcd template
    lcdTimerID = sysTimer3.every(1000,selectValues);
    
    
  }
  if(LCDDone != true && systemActive == 0){
      displayTemplate();
      LCDDone = true;
    }
  //Serial.println("LCD done: ");
  //Serial.println(LCDDone);
  
}

void displayTemplate(){
  //Display template to LCD
  lcd.setCursor(0,0);
  lcd.print("                  ");  //clears old values
  lcd.setCursor(0,0);
  lcd.print("Total g: ");
  
  lcd.setCursor(0,1);
  lcd.print("                  ");  //clears old values
  lcd.setCursor(0,1);
  lcd.print("Total time: ");


}

void selectValues(){
  //Used to select var
  //this function is called by systimer3 to display to the LCD the values 
  //selected by the 2 potentiometers
  //Analog values will be between 0 & 1023
  //225 intervals

  int powerVal = analogRead(powerPin);
  int timerVal = analogRead(timerPin);


  //Choose value of ozone generated
  if ((powerVal >= 170) && (powerVal < 343)) 
  { gramsSelected = 1; }     //1 g/h
  else if ((powerVal >= 343) && (powerVal < 513)) 
  { gramsSelected = 2; }     //2 g/h
  else if ((powerVal >= 513) && (powerVal < 683)) 
  { gramsSelected = 5; }     //5 g/h
  else if ((powerVal >= 683) && (powerVal < 853)) 
  { gramsSelected = 10; }     //10 g/h
  else if ((powerVal >= 853) && (powerVal < 1100)) 
  { gramsSelected = 20; }     //20 g/h
  else 
  { gramsSelected = 0; }
  

  
  lcd.setCursor(9,0);
  lcd.print("         ");
  lcd.setCursor(9,0);
  lcd.print(gramsSelected);
  lcd.setCursor(11,0);
  lcd.print("g");//pos 11


  //Choose interval of generator on
  if ((timerVal >= 173) && (timerVal < 343)) 
  { totalTimeSelected = 10 * 60; }     //10 min
  else if ((timerVal >= 343) && (timerVal < 513)) 
  { totalTimeSelected = 30 * 60; }     //30 min
  else if ((timerVal >= 513) && (timerVal < 683)) 
  { totalTimeSelected = 60 * 60; }     //60 min
  else if ((timerVal >= 683) && (timerVal < 853)) 
  { totalTimeSelected = 120 * 60; }     //120 min
  else if ((timerVal >= 853) && (timerVal < 1100)) 
  { totalTimeSelected = 240 * 60; }     //240 min
  else 
  { totalTimeSelected = 0; }



  //print values to lcd
  lcd.setCursor(12,1);
  lcd.print("       ");
  lcd.setCursor(12,1);
  lcd.print(totalTimeSelected/60);
  lcd.setCursor(15,1); lcd.print('m'); // pos 15
  //pos 13
}

void boostUnboostSystem(){
  /***
   * Start or Stop Boost Function
   */

  //After the start of the function block boost button
  // and after 500ms unlock the button
  boostBlocked = 1;
  sysTimer1.after(1000, unblockBoost);

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
  sysTimer1.after(1000, unblockStart);

  //reset boost timer
  boostTime = 0;

  if (systemActive == 0 && (gramsSelected > 0) && (totalTimeSelected > 0))
  {
    Serial.println(F("Starting System"));

    //Print status on LCD
    lcd.clear();
    lcd.print("System running");
    digitalWrite(GenOnLed, HIGH);

    //readAnalogButtons(); 
    GenOnTime = gramsSelected;

    systemActive = 1;
    fanOn();

    sysTimer3.stop(lcdTimerID);     //stop timer
       

    systemActiveID = sysTimer2.every(1000, dutyCycle);    //call duty cycle every 1000ms    
    Serial.print("Timer2 ID: "); Serial.println(systemActiveID);     //print id of the timer
    
  }
  else if(systemActive == 1)
  {
    Serial.println(F("Stopping System"));
    digitalWrite(GenOnLed, LOW);
    systemActive = 0;
    fanOff();
    o3Off();

    //deactivate boost timer
    boostActive = 0;
    boostTime = 0;

    //Reset functioning time
    currentTimeOn = 0;

    LCDDone = false;

    sysTimer2.stop(systemActiveID);    // stop the call to duty cycle
    lcdTimerID = sysTimer3.every(1000,selectValues);
    
    
  }

  
}
void unblockStart() 
{ 
  startBlocked = 0; 
}


void fanOn(){
  Serial.println(F("The Fans are ON"));
  analogWrite(fanPWM1,255);
  analogWrite(fanPWM2,255);      
}
void fanOff(){
  analogWrite(fanPWM1,0);
  analogWrite(fanPWM2,0);

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
  currentTimeOn = currentTimeOn + 1;

  //Debug time remaining
  Serial.print("Remaining time:");Serial.println(totalTimeSelected-currentTimeOn);

  if (boostActive == 1) { 
    boostTime = boostTime + 1; 
    o3On(); 
    return; 
  }

  //check if cycle time is bigger than the on time selected
  if (cycleTime < GenOnTime) { 
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
  lcd.print("Generator On");
}

