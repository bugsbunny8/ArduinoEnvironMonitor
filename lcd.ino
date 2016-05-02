/**
 *  PCF8574 with LCD1604
 *  Connection
 *  
 *   PCF8574      <--------> Arduino
 *   GND          ---------  GND
 *   VCC          ---------  5v
 *   SDA          ---------  A4
 *   SCL          ---------  A5
*/
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void lcd_setup(void)
{
  lcd.init();

  lcd.backlight();
  lcd.print(F("Hello world..."));
}

