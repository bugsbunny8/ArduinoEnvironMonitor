/* Creating  an Environment Monitor with  Arduino nano
 * DHT11(temp, Humidity)
 * light
 * Barometer
 * Connecting with ESP8266 WIFI Connector
 * 
 * IO define:
 * 
 */
 
#include <SoftwareSerial.h>

#include <dht11.h>

#include <avr/pgmspace.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
//  lcd_setup();
  
  BH1750FVI_setup();
  BMP180_Setup();

  Esp8266Setup();
}

void loop() {
  collectAndSendEsp8266();

  delay(1000);
}

