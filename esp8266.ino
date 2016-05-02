/* Be Careful with Hardware,
 * old ESP8266(512KB), should use the 0.9xxx AT firmware(9600 Baud Rate)
 * New ESP8266(1MB), can use >=1.xxx AT Firmware */

/* use Software Serial for sending data */
#define _ESP8266_RXPIN_ 10
#define _ESP8266_TXPIN_ 11

SoftwareSerial Esp8266SoftSerial(_ESP8266_RXPIN_, _ESP8266_TXPIN_);

#define Esp8266Serial  Esp8266SoftSerial
#define DbgSerial Serial

#define DbgPrintln(...)  DbgSerial.println(__VA_ARGS__)
#define DbgPrint(...)  DbgSerial.print(__VA_ARGS__)

#define ESP8266_SERIAL_BEGIN(...)  Esp8266Serial.begin(__VA_ARGS__)
#define ESP8266_SERIAL_PRINT(...)  Esp8266Serial.print(__VA_ARGS__)
#define ESP8266_SERIAL_PRINTLN(...) Esp8266Serial.println(__VA_ARGS__)
#define ESP8266_SERIAL_FLUSH()  Esp8266Serial.flush()
#define ESP8266_SERIAL_FIND(...)  Esp8266Serial.find(__VA_ARGS__)

#define HOME_SSID "TP-LINK_ZPY"
#define HOME_PASSWORD "zpy810304?"

#define SENSOR_NR 4
#define SENSOR_INDEX_TEMP           0
#define SENSOR_INDEX_HUMIDITY       1
#define SENSOR_INDEX_BAROMETER      2
#define SENSOR_INDEX_LIGHT          3

#define yeeWebsiteIp F("42.96.164.52")
#define yeeWebsite "api.yeelink.net"
#define yeeAPIKey F("13529a2762b65b3ce5b08fa107255b53")

#define DeviceID F("347255")
const long SensorID[SENSOR_NR] = {
  387677,387678,387679,387680};

uint8_t currentSensorIndex = 0;
int sensorDataInt[SENSOR_NR];
char sensorData[SENSOR_NR][30];
uint8_t sensorDataLength[SENSOR_NR];

/* Connection interval define */
uint32_t lastConnectionTime = 0;
const uint16_t PostingInterval = 10000; /*ms, 10 seconds */

boolean connectWiFi(){
  ESP8266_SERIAL_PRINTLN(F("AT+CWMODE=1"));
  delay(2000);
  
  String cmd="AT+CWJAP=\"";
  cmd+=HOME_SSID;
  cmd+="\",\"";
  cmd+=HOME_PASSWORD;
  cmd+="\"";
  ESP8266_SERIAL_PRINTLN(cmd);
  
  delay(8000);
//  if( ESP8266_SERIAL_FIND("OK") ){
  if( Esp8266Serial.find("OK") ){
    DbgPrintln(F("RECEIVED: OK"));
    return true;
  }else{
    DbgPrintln(F("RECEIVED: Error"));
    return false;
  }
}

void Esp8266Setup()
{
  ESP8266_SERIAL_BEGIN(9600);
  ESP8266_SERIAL_FLUSH();
  ESP8266_SERIAL_PRINTLN(F("AT+RST"));

  delay(6000);
  connectWiFi();
}

bool esp8266IpConfig(const __FlashStringHelper * webIp)
{  
  ESP8266_SERIAL_PRINTLN(F("AT+CIPMUX=1"));
  Esp8266Serial.flush();
  delay(1000);
  
  ESP8266_SERIAL_PRINT(F("AT+CIPSTART=0,\"TCP\",\""));
  ESP8266_SERIAL_PRINT(webIp);
  ESP8266_SERIAL_PRINTLN(F("\",80"));
  delay(2000);

  if( Esp8266Serial.find("Error") ){
   return false; 
  }
  
  return true;
}

int getIntLength(int someValue) {
  // there's at least one byte:
  int digits = 1;

  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue /10;

  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}

int esp8266SendF(const __FlashStringHelper * str, bool isSend)
{
  if(isSend){
    ESP8266_SERIAL_PRINT(str);
  }
  return strlen_P((const char PROGMEM *)(str));
}

int esp8266SendlnF(const __FlashStringHelper * str, bool isSend)
{
  if(isSend){
    ESP8266_SERIAL_PRINTLN(str);
  }
  return strlen_P((const char PROGMEM *)(str));
}

int esp8266SendStr(char * str, bool isSend)
{
  if(isSend){
    ESP8266_SERIAL_PRINT(str);
  }
  return strlen(str);
}

int esp8266SendInt(int val, bool isSend)
{
  String intStr = String(val);

  if(isSend){
    ESP8266_SERIAL_PRINT(intStr);
  }
  
  return intStr.length();
}

int esp8266SendLong(long val, bool isSend)
{
  String longStr = String(val);

  if(isSend){
    ESP8266_SERIAL_PRINT(longStr);
  }
  
  return longStr.length();
}

/* have to optimize codes like this, rather than using String aggregate it,
 * which might generate enormous SRAM memory usage! */

/* SEND or GetString Length*/
int esp8266SendGetLenYeelink(bool isSend, const __FlashStringHelper * deviceId, long sensorId, int Data)
{
  int httpCmdLen = 0;
  int cmdLength = 10 + getIntLength(Data);

  httpCmdLen = esp8266SendF(F("POST /v1.0/device/"), isSend);
  httpCmdLen += esp8266SendF(deviceId, isSend);
  httpCmdLen += esp8266SendF(F("/sensor/"), isSend);
  httpCmdLen += esp8266SendLong(sensorId, isSend);
  httpCmdLen += esp8266SendF(F("/datapoints"), isSend);
  httpCmdLen += esp8266SendF(F( " HTTP/1.1\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("Host: api.yeelink.net\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("Accept: *"), isSend);
  httpCmdLen += esp8266SendF(F("/"), isSend);
  httpCmdLen += esp8266SendF(F("*\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("U-ApiKey: "), isSend);
  httpCmdLen += esp8266SendF(yeeAPIKey, isSend);
  httpCmdLen += esp8266SendF(F("\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("Content-Length: "), isSend);
  httpCmdLen += esp8266SendInt(cmdLength, isSend);
  httpCmdLen += esp8266SendF(F("\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("Content-Type: application/x-www-form-urlencoded\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("Connection: close\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("\r\n"), isSend);
  httpCmdLen += esp8266SendF(F("{\"value\":"), isSend);
  httpCmdLen += esp8266SendInt(Data, isSend);
  httpCmdLen += esp8266SendlnF(F("}\r\n"), isSend);
  
  return httpCmdLen;
}

void esp8266SendData(const __FlashStringHelper * deviceId, long sensorId, int Data)
{
  int httpCmdLen = 0;
  
  DbgPrintln(deviceId);
  DbgPrint(F("sensorId:"));
  DbgPrintln(sensorId);
  
  if (!esp8266IpConfig(yeeWebsiteIp)) {
    ESP8266_SERIAL_PRINTLN(F("AT+CIPCLOSE=0"));
    return;
  }
  
  httpCmdLen = esp8266SendGetLenYeelink(false, deviceId, sensorId, Data);

  ESP8266_SERIAL_PRINT(F("AT+CIPSEND=0,"));
  ESP8266_SERIAL_PRINTLN(httpCmdLen);

  if( Esp8266Serial.find(">") ){
    /* Real Send */
    esp8266SendGetLenYeelink(true, deviceId, sensorId, Data);;
  }else{
    ESP8266_SERIAL_PRINTLN(F("AT+CIPCLOSE"));
  }
  
}
  
void inc_current_sensor_index()
{
  currentSensorIndex ++;
  if(currentSensorIndex == SENSOR_NR){
    currentSensorIndex = 0;
  }
}

void collectAndSendEsp8266(void)
{
  char recvMessage[400];
  uint32_t currentTime = 0;

  currentTime = millis();
  if( (currentTime > lastConnectionTime) &&
  ( (currentTime - lastConnectionTime) <= PostingInterval) ){
    /* still in post range, collect data and return */
     collectSensorAllData();
     delay(500);

     return;
  }

  collectSensorData(currentSensorIndex);
  esp8266SendData(DeviceID, SensorID[currentSensorIndex], sensorDataInt[currentSensorIndex]);
  inc_current_sensor_index();
  lastConnectionTime = currentTime;
  
  delay(1000);
}

