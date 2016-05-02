#include "Wire.h"

/* BMP180 */
/*BH1750FVI I2C Mapping, TWI */
/*******************************************
* BMP180                          Arduino Nano
VCC     --------------------->    3.3V
SCL     --------------------->    Analog pin 5(A5)
SDA     --------------------->    Analog pin 4(A4)
GND     --------------------->    GND
*******************************************/
#include <SFE_BMP180.h>
SFE_BMP180 pressure;
double Pres;

#define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters

/* Arduino Nano */
/*******************************************
* BH1750                          Arduino Nano
VCC     --------------------->    5V
SCL     --------------------->    Analog pin 5(A5)
SDA     --------------------->    Analog pin 4(A4)
ADD     --------------------->    Leave it open
GND     --------------------->    GND
*******************************************/
int BH1750FVI_Device=0x23; /*I2C Address */
unsigned int Lux;
float FtCd, Wattsm2;

/*DHT11*/
/*******************************
 * Out ------------------> PIN 4
 ********************************/
dht11 DHT11;
int dht11ReadPin=4;

/* read DHT11 info */
void dht11Read(){
  int chk = 0;

  DbgPrintln(F("Read sensor: "));
  chk = DHT11.read(dht11ReadPin);
  switch (chk)
  {
  case DHTLIB_OK: 
    DbgPrintln(F("OK")); 
    break;
    
  case DHTLIB_ERROR_CHECKSUM: 
    DbgPrintln(F("Checksum error")); 
    break;
    
  case DHTLIB_ERROR_TIMEOUT: 
    DbgPrintln(F("Time out error")); 
    break;
    
  default: 
    DbgPrintln(F("Unknown error")); 
    break;
  }
  
  DbgPrint(F("Humidity (%): "));
  DbgPrintln((float)DHT11.humidity, 2);
  DbgPrint(F("Temperature (oC): "));
  DbgPrintln((float)DHT11.temperature-2, 2);
}

void BH1750FVI_setup()
{
  Wire.begin();
  
  BH1750FVI_Config();
  delay(300);
}

void BH1750FVI_Read()
{
  Lux = BH1750_ReadLux();

  DbgPrint(Lux, DEC);
  DbgPrintln(F("[lx]"));
#if 0
  FtCd = Lux/10.764;
  Wattsm2 = Lux/683.0;
  DbgPrint(FtCd, 2);
  DbgPrintln(F("[FC]"));
  DbgPrint(Wattsm2, 4);
  DbgPrintln(F("[Watts/m^2]"));
#endif
}

unsigned int BH1750_ReadLux()
{
  unsigned int i = 0;
  
  Wire.beginTransmission(BH1750FVI_Device);
  Wire.requestFrom(BH1750FVI_Device, 2);
  
  while(Wire.available()){
    i << 8;
    i |= Wire.read();
  }
  Wire.endTransmission();
  return i/1.2; /* Convert to Lux*/
}

void BH1750FVI_Config()
{
  Wire.beginTransmission(BH1750FVI_Device);
  Wire.write(0x10); // Set resolution to 1 Lux
  Wire.endTransmission();
}

void BMP180_Setup()
{
  if (pressure.begin()){
    DbgPrintln(F("BMP180 init success"));
  }else{
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
    DbgPrintln(F("BMP180 init fail\n\n"));
    while(1); // Pause forever.
  }
}

void BMP180_Read(void)
{
  char status;
  double T,P,p0,a;

  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  DbgPrintln();
  DbgPrint(F("provided altitude: "));
  DbgPrint(ALTITUDE,0);
  DbgPrint(F(" meters, "));
  DbgPrint(ALTITUDE*3.28084,0);
  DbgPrintln(F(" feet"));
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status == 0){
    DbgPrintln(F("error starting temperature measurement\n"));
    return;
  }

  // Wait for the measurement to complete:
  delay(status);

  // Retrieve the completed temperature measurement:
  // Note that the measurement is stored in the variable T.
  // Function returns 1 if successful, 0 if failure.

  status = pressure.getTemperature(T);
  if (status == 0){
    DbgPrintln(F("error retrieving temperature measurement\n"));
    return;
  }
    
  // Print out the measurement:
  DbgPrint(F("temperature: "));
  DbgPrint(T,2);
  DbgPrint(F(" deg C, "));
  DbgPrint((9.0/5.0)*T+32.0,2);
  DbgPrintln(F(" deg F"));
      
  // Start a pressure measurement:
  // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startPressure(3);
  if (status == 0){
    DbgPrintln(F("error starting pressure measurement\n"));
    return;
  }
    
  // Wait for the measurement to complete:
  delay(status);

  // Retrieve the completed pressure measurement:
  // Note that the measurement is stored in the variable P.
  // Note also that the function requires the previous temperature measurement (T).
  // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
  // Function returns 1 if successful, 0 if failure.

  status = pressure.getPressure(P,T);
  if (status == 0){
    DbgPrintln(F("error retrieving pressure measurement\n"));
    return;
  }

  Pres = P;

  // Print out the measurement:
  DbgPrint(F("absolute pressure: "));
  DbgPrint(P,2);
  DbgPrint(F(" mb, "));
  DbgPrint(P*0.0295333727,2);
  DbgPrintln(F(" inHg"));

#if 0
  // The pressure sensor returns abolute pressure, which varies with altitude.
  // To remove the effects of altitude, use the sealevel function and your current altitude.
  // This number is commonly used in weather reports.
  // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
  // Result: p0 = sea-level compensated pressure in mb
  
  p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
  DbgPrint(F("relative (sea-level) pressure: "));
  DbgPrint(p0,2);
  DbgPrint(F(" mb, "));
  DbgPrint(p0*0.0295333727,2);
  DbgPrintln(F(" inHg"));
  
  // On the other hand, if you want to determine your altitude from the pressure reading,
  // use the altitude function along with a baseline pressure (sea-level or other).
  // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
  // Result: a = altitude in m.
  
  a = pressure.altitude(P, p0);
  DbgPrint(F("computed altitude: "));
  DbgPrint(a,0);
  DbgPrint(F(" meters, "));
  DbgPrint(a*3.28084,0);
  DbgPrintln(F(" feet"));
#endif
}

void collectSensorData(uint8_t sensorIndex){
  DbgPrintln(currentSensorIndex);
  
  switch(sensorIndex){
  case SENSOR_INDEX_TEMP:
    /*  Read DHT11 */
    dht11Read();
    sensorDataInt[SENSOR_INDEX_TEMP] = DHT11.temperature-2;
    sensorDataLength[sensorIndex]=sprintf(sensorData[sensorIndex],"{\"value\":%d}", DHT11.temperature-2);
    break;

  case SENSOR_INDEX_HUMIDITY:
    /*  Read DHT11 */
    dht11Read();
    sensorDataInt[SENSOR_INDEX_HUMIDITY] = DHT11.humidity;
    sensorDataLength[sensorIndex]=sprintf(sensorData[sensorIndex],"{\"value\":%d}",DHT11.humidity);
    break;

  case SENSOR_INDEX_BAROMETER:
    /* Read BMP180 */
    BMP180_Read();
    sensorDataInt[SENSOR_INDEX_BAROMETER] = 0;
    sensorDataLength[sensorIndex]=sprintf(sensorData[sensorIndex],"{\"value\":%d}", int(Pres));
    break;

  case SENSOR_INDEX_LIGHT:
  /* Read BH1750FVI */
    BH1750FVI_Read();
    sensorDataInt[SENSOR_INDEX_LIGHT] = Lux;
    sensorDataLength[sensorIndex]=sprintf(sensorData[sensorIndex],"{\"value\":%d}", Lux);
    break;
  }
  
}

void collectSensorAllData(void)
{

}

