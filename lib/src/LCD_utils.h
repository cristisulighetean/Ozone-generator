#ifndef LCD_utils_H
#define LCD_utils_H

#include <LiquidCrystal_I2C.h>

/**
 * @brief Initializes the lcd object
 * 
 * @param lcd Object of the lcd driver
 */
void LCDinit(LiquidCrystal_I2C lcd);

/**
 * @brief Displays the template for selecting time and grams
 * 
 * @param lcd Object of the lcd driver
 */
void LCDdisplayTemplateSelectTime(LiquidCrystal_I2C lcd);

/**
 * @brief Displayes the remaining time on the lcd
 * 
 * @param lcd 
 * @param secondsRemaining seconds remaining
 */
void LCDdisplayRemainingTime(LiquidCrystal_I2C lcd, uint32_t secondsRemaining);

/**
 * @brief Displays the remaining time template on the lcd
 * 
 * @param lcd 
 */
void LCDdisplayTemplateRemainingTime(LiquidCrystal_I2C lcd);

/**
 * @brief Display selected grams value
 * 
 * @param lcd 
 * @param grams 
 */
void LCDdisplaySelectedGrams(LiquidCrystal_I2C lcd, uint16_t grams);

/**
 * @brief Display selected minutes value
 * 
 * @param lcd 
 * @param minutes 
 */
void LCDdisplaySelectedMinutes(LiquidCrystal_I2C lcd, uint16_t minutes);



#endif

