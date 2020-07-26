/*
NOTES





*/


// defines
#define DEBUG
#define SEARCH_ADDRESS_DS18B20
#define I2C_SCAN
//#define SET_RTC
#define GET_TEMP_OFFSET
//#define TEMP_OFFSET

//#define DRY_RUN
//#define RESTART
#define SERIAL_INFO
//#define FERTILIZATION


//librlies
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DS3231.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

//i2c address
#define addrLed 0x01
#define addrPCA9634Restart 0x03
#define addrPump 0x20
#define addrSSR 0x30
#define addrEEPROM 0x57
#define addrRTC 0x68
#define addrPCA9634reserved 0x70


//temp
uint8_t T0SensorAddress[8] = { 0x28, 0xC4, 0xF7, 0x79, 0xA2, 0x16, 0x03, 0x8C }; //water sensor
uint8_t T1SensorAddress[8] = { 0x28, 0x0A, 0x23, 0x79, 0xA2, 0x19, 0x03, 0x59 }; //cable sensor - test!
uint8_t T2SensorAddress[8] = { 0x28, 0x75, 0x3F, 0x79, 0xA2, 0x16, 0x03, 0xA0 }; //cable sensor
uint8_t T3SensorAddress[8] = { 0x28, 0xFF, 0x1A, 0x62, 0xC0, 0x17, 0x05, 0xF0 }; //cable sensor
#define TWaterAddr T0SensorAddress[8]
#define TCableAddr T1SensorAddress[8]
#define TLedAddr T2SensorAddress[8]
#define TAirAddr T3SensorAddress[8]
#define TempPin 39
#define TempPrec 12 //9-12
OneWire oneWireDS (TempPin);
DallasTemperature SensorsDS(&oneWireDS);
//get temp
unsigned long TimeReadTemp;
float TWaterTemp, TWaterTempNoOffset;
float TCableTemp, TCableTempNoOffset;
float TLedTemp, TLedTempNoOffset;
float TAirTemp, TAirTempNoOffset;
float AllOffset = 0;
float TWaterOffset = 0;
float TCableOffset = 19.13-18.69;
float TLedOffset = 19.13-19.06;
float TAirOffset = 19.13-19.19;
byte ErrorTempMax = 10;
byte ErrorTemp = 0;



//time
// inicializace RTC z knihovny
DS3231 rtc;
// vytvoření proměnné pro práci s časem
RTCDateTime DateTime;
unsigned long TimeStamp;
unsigned long CurrentMillis, RTCRead;

//tft
/*
#define ILI9341_BLACK       0x0000  ///<   0,   0,   0
#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
#define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
#define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
#define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
#define ILI9341_BLUE        0x001F  ///<   0,   0, 255
#define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
#define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
#define ILI9341_RED         0xF800  ///< 255,   0,   0
#define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
#define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
#define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK        0xFC18  ///< 255, 130, 198
*/
#define TFT_CLK 52
#define TFT_MISO 50
#define TFT_MOSI 51
#define TFT_DC 43
#define TFT_CS 44
#define TFT_RST 45
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,  TFT_RST);
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);




void setup(){
  Serial.begin(115200);
  SensorsDS.begin();
  rtc.begin();
  tft.begin();

  Serial.println("------Start setup-----");
  discoverOneWireDevices();
  Serial.print("Address T0 sensor: ");   PrintSensorAdress(T0SensorAddress);  Serial.println();
  Serial.print("Address T1 sensor: ");   PrintSensorAdress(T1SensorAddress);  Serial.println();
  Serial.print("Address T2 sensor: ");   PrintSensorAdress(T2SensorAddress);  Serial.println();
  Serial.print("Address T3 sensor: ");   PrintSensorAdress(T3SensorAddress);  Serial.println("\n\r");
  i2cScan();
  setRTC();
  SetResolution();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  GetTime();
  //GetTempOffset()
  GetTemp();
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);


  Serial.println("------End setup-----");
}


void loop(){
  CurrentMillis = millis();
  if (CurrentMillis >= (RTCRead+1000)){
    GetTime();
    TestScreen();
  }
  GetTemp();


}


void discoverOneWireDevices(void) {
  #ifdef SEARCH_ADDRESS_DS18B20
    byte i;
    byte present = 0;
    byte data[12];
    byte addr[8];
    byte num = 0;

    Serial.print("Looking for 1-Wire devices...\n\r");
    while(oneWireDS.search(addr)) {
      //Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
      Serial.println();
      num++;
      for( i = 0; i < 8; i++) {
        Serial.print("0x");
        if (addr[i] < 16) {
          Serial.print('0');
        }
        Serial.print(addr[i], HEX);
        if (i < 7) {
          Serial.print(", ");
        }
      }
      if ( OneWire::crc8( addr, 7) != addr[7]) {
          Serial.print("CRC is not valid!\n");
          return;
      }
    }
    Serial.print("\n\r\n\rI found: " + String(num) + " one wire device(s).");
    Serial.print("\n\rThat's it.\r\n");
    Serial.println();
    oneWireDS.reset_search();

    Serial.print("Parasite power is: ");
      if (SensorsDS.isParasitePowerMode()) Serial.println("ON\r\n");
      else Serial.println("OFF\r\n");

    return;
  #endif
}

void i2cScan(){
  #ifdef I2C_SCAN
  byte error, address;
 int nDevices;

 Serial.println("Scanning I2C bus...");

 nDevices = 0;
 for(address = 1; address < 127; address++ )
 {
   // The i2c_scanner uses the return value of
   // the Write.endTransmisstion to see if
   // a device did acknowledge to the address.
   Wire.beginTransmission(address);
   error = Wire.endTransmission();

   if (error == 0)
   {
     Serial.print("I2C device found at address 0x");
     if (address<16)
       Serial.print("0");
     Serial.print(address,HEX);
     Serial.println("  !");

     nDevices++;
   }
   else if (error==4)
   {
     Serial.print("Unknown error at address 0x");
     if (address<16)
       Serial.print("0");
     Serial.println(address,HEX);
   }
 }
 if (nDevices == 0)
   Serial.println("No I2C devices found\n");
 else
   Serial.println("\n\rI found: " + String(nDevices) + " I2C device(s).\n\r");
  #endif
}

void setRTC(){
    #ifdef SET_RTC
  //adjus time in RTC module

    rtc.setDateTime(__DATE__, __TIME__);
    Serial.println("Succesfully set time to compilation time.\n\r");
  #endif
}

void GetTime(){
  DateTime = rtc.getDateTime();
  Serial.print(DateTime.year);   Serial.print("-");
  Serial.print(DateTime.month);  Serial.print("-");
  Serial.print(DateTime.day);    Serial.print(" ");
  Serial.print(DateTime.hour);   Serial.print(":");
  Serial.print(DateTime.minute); Serial.print(":");
  Serial.print(DateTime.second); Serial.print("  ");
  TimeStamp = ((DateTime.hour)*3600)+(DateTime.minute*60)+(DateTime.second);
  Serial.print("Timestamp: "); Serial.println(TimeStamp);
  RTCRead = CurrentMillis;
}

void PrintSensorAdress (byte SensorName[8]){
  byte i;
  //byte SensorName[8];
  for( i = 0; i < 8; i++) {
    Serial.print("0x");
    if (SensorName[i] < 16) {
      Serial.print('0');
    }
    Serial.print(SensorName[i], HEX);
    if (i < 7) {
      Serial.print(", ");
    }
  }
}

void GetTemp(){
  if (TimeReadTemp != (TimeStamp/60)){
    #ifdef DEBUG
     Serial.println("Start measure temp....");
    #endif
    SensorsDS.requestTemperatures();
    TWaterTemp = ReadTempWithOffset(T0SensorAddress, TWaterTempNoOffset, TWaterOffset, "Water");
    TCableTemp = ReadTempWithOffset(T1SensorAddress, TCableTempNoOffset, TCableOffset, "Cable");
    TLedTemp = ReadTempWithOffset(T2SensorAddress, TLedTempNoOffset, TLedOffset, "LED");
    TAirTemp = ReadTempWithOffset(T3SensorAddress, TAirTempNoOffset, TAirOffset, "Air");

    TimeReadTemp = TimeStamp/60;

    #ifdef DEBUG
     Serial.println("Done");Serial.println("");
    #endif
  }
}

float ReadTempWithOffset(DeviceAddress Addr, float NameTempNoOffset, float NameTempOffset, String Name){
  float NameTemp;
  NameTempNoOffset = ReadTemperature(Addr);
  if (((NameTempNoOffset > (-127)) && (NameTempNoOffset < (85)))){
    ErrorTemp = 0;
    NameTemp = NameTempNoOffset + NameTempOffset + AllOffset;
    #ifdef DEBUG
      Serial.println(String(Name)+" with offset temp is: " + String(NameTemp) + "C. Offset for " + String(Name) + " is: " + String(NameTempOffset) + ". All offset is: " + String(AllOffset) + ".");
    #endif
    return NameTemp;
  }
  else if (ErrorTemp < ErrorTempMax){
    #ifdef DEBUG
      Serial.println("Temp measure is FAIL.");
      Serial.println("Error Temp counter is: " + String(ErrorTemp+1) + " / " + String(ErrorTempMax));
    #endif
    ErrorTemp ++;
  }
  else{
    Restart("Lot of error measure. Total: ", ErrorTemp);
  }
}

void Restart(String Message, int Value){
  #ifdef RESTART
  Serial.println();
  Serial.print("Why restart device: ");
  Serial.print(Message);
  Serial.print(" Value: ");
  Serial.println(Value);
  Serial.println("--------------------------------------------------------------------------------------");
  Serial.println("----------------------------------------RESTART DEVICE--------------------------------");
  Serial.println("--------------------------------------------------------------------------------------");
  Serial.println();
  Serial.println();
  delay(500);
  //digitalWrite(RestartPin, LOW);
  #endif
}

float ReadTemperature(DeviceAddress deviceAddress){
  float tempC = SensorsDS.getTempC(deviceAddress);
  return tempC;
}

void SetResolution(){
  SensorsDS.setResolution(T0SensorAddress, TempPrec);
  SensorsDS.setResolution(T1SensorAddress, TempPrec);
  SensorsDS.setResolution(T2SensorAddress, TempPrec);
  SensorsDS.setResolution(T3SensorAddress, TempPrec);
}

void TestScreen(){
  unsigned int color = random(0,65535);
  //tft.fillScreen(ILI9341_BLACK);
  //tft.fillRect(0, 0, 240, 80, ILI9341_DARKGREEN);
  tft.setCursor(0, 0);
  tft.setTextColor(color, ILI9341_BLACK ); tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.print(DateTime.year);   tft.print("-");
  tft.print(DateTime.month);  tft.print("-");
  tft.print(DateTime.day);    tft.print(" ");
  tft.print(DateTime.hour);   tft.print(":");
  tft.print(DateTime.minute); tft.print(":");
  tft.print(DateTime.second); tft.println("  ");
  tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK ); tft.setTextSize(2);
  tft.print("Water temp: ");   tft.print(TWaterTemp);   tft.println(" C");
  tft.print("Cable temp: ");   tft.print(TCableTemp);   tft.println(" C");
  tft.print("Led temp: ");   tft.print(TLedTemp);   tft.println(" C");
  tft.print("Air temp: ");   tft.print(TAirTemp);   tft.println(" C");

}
