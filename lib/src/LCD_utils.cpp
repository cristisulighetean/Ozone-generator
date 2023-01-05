#include "LCD_utils.h"
#include <math.h>


void LCDinit(LiquidCrystal_I2C lcd)
{
    lcd.init();
    lcd.backlight();
    lcd.home();
    lcd.clear();
}

void LCDdisplayTemplateSelectTime(LiquidCrystal_I2C lcd)
{
  //Display template to LCD
  lcd.setCursor(0,0);               // set cursor on first line
  lcd.print("                  ");  // clears old values
  lcd.setCursor(0,0);               // set cursor to start of line
  lcd.print("Total g: ");
  
  // Perform the same operations for the second line
  lcd.setCursor(0,1);
  lcd.print("                  ");
  lcd.setCursor(0,1);  
  lcd.print("Total time: ");
}

void LCDdisplayRemainingTime(LiquidCrystal_I2C lcd, uint32_t secondsRemaining)
{
  uint16_t minutes, seconds;
  minutes = floor(secondsRemaining / 60); 
  seconds = secondsRemaining - (minutes * 60 );

  // Display Remaining time (clear old values first)
  lcd.setCursor(1,1);
  lcd.print("   ");
  lcd.setCursor(1,1);
  lcd.print(minutes);

  lcd.setCursor(9,1);
  lcd.print("  ");
  lcd.setCursor(9,1);
  lcd.print(seconds);
}

void LCDdisplayTemplateRemainingTime(LiquidCrystal_I2C lcd)
{
  lcd.setCursor(0,0);
  lcd.print("Remaining Time");

  lcd.setCursor(4,1);
  lcd.print(" min");
  
  lcd.setCursor(11,1);
  lcd.print(" sec");
}

void LCDdisplaySelectedGrams(LiquidCrystal_I2C lcd, uint16_t grams)
{
  lcd.setCursor(9,0);
  lcd.print("         ");
  lcd.setCursor(9,0);
  lcd.print(grams);
  lcd.setCursor(11,0);
  lcd.print("g");   //pos 11
}

void LCDdisplaySelectedMinutes(LiquidCrystal_I2C lcd, uint16_t minutes)
{
  lcd.setCursor(12,1);
  lcd.print("       ");
  lcd.setCursor(12,1);
  lcd.print(minutes / 60);
  lcd.setCursor(15,1); lcd.print('m');  // pos 15
}
