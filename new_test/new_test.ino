/*
--------------------------NOTES---------------------------------
Use LIB
RTClib                                        https://navody.arduino-shop.cz/docs/texty/0/7/rtclib.zip
DS18B20                                       https://navody.arduino-shop.cz/docs/texty/0/14/dallastemperature.zip
OneWire                                       https://navody.arduino-shop.cz/docs/texty/0/15/onewire.zip



sensor adress 0x28, 0x25, 0xC5, 0xF7, 0x08, 0x00, 0x00, 0x61 vnejsi dlouhy kabel
              0x28, 0xFF, 0x1A, 0x62, 0xC0, 0x17, 0x05, 0xF0 vnejsi kratky kabel
              0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10 v akva
Set

pumpa 520ms/ml


*/
#define DEBUG
//#define SET_RTC

//librlies
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//declarate variables
//for RTC
int SetRtcY = 2020;
byte SetRtcMo = 01;
byte SetRtcD = 04;
byte SetRtcH = 13;
byte SetRtcM = 03;
byte SetRtcS = 00;

// var for date
int TimeY = 0;
byte TimeMo = 0;
byte TimeDay = 0; //day of week
byte TimeD = 0;
byte TimeH = 0;
byte TimeM = 0;
byte TimeS = 0;
byte TimeHM = 0;
int TimeStampDay = 0;

//millis
unsigned long CurrentMillis = 0;
unsigned long GetTimeMillis = 0;

RTC_DS1307 DS1307;

char DayOfTheWeek[7][8] = {"nedele", "pondeli", "utery", "streda", "ctvrtek", "patek", "sobota"};

void setup () {
  Wire.begin();
  // serial comunication via USB
  Serial.begin(115200);
  Serial.println("------Start setup-----");
  //i2c scanner
  Serial.println("\nI2C Scanner");
  i2cScann();



  // chceck if RTC connected
  if (! DS1307.begin()) {
    Serial.println("Hodiny nejsou pripojeny!");
    while (1);
  }
  // kontrolu spuštění obvodu reálného času
  if (! DS1307.isrunning()) {
    Serial.println("Hodiny nejsou spusteny! Spoustim nyni..");
  }
  #ifdef SET_RTC
  SetRTC();
  #endif
  GetTime();
  Serial.println();
  Serial.println("---------------------SETUP INFO------------------------");
  char verChar[8];
  sprintf(verChar, "%04d%02d%02d", SetRtcY, SetRtcMo, SetRtcD);
  Serial.println("Version: " + String(verChar));
  Serial.println("------End setup-----");
}

void loop(){
  CurrentMillis = millis();
  if (CurrentMillis >= (GetTimeMillis + 1000)) {
    GetTimeMillis = CurrentMillis;
    GetTime();

  }


}

void i2cScann(){
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

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
    Serial.println("\nTotal device found: " + String(nDevices));
    Serial.println("\nDone\n");
    Serial.println("Exit I2C Scanner\n");
}

void GetTime(){
  DateTime DateTime = DS1307.now();
  int NTimeY ;
  byte NTimeMo, NTimeDay, NTimeD, NTimeH, NTimeM, NTimeS;
  NTimeY = DateTime.year();
  NTimeMo = DateTime.month();
  NTimeDay = DateTime.day();
  NTimeH = DateTime.hour();
  NTimeM = DateTime.minute();
  NTimeS = DateTime.second();
    if((NTimeMo > 0) && (NTimeMo < 13)){
      if((NTimeH >= 0) && (NTimeH < 24)){
        if((NTimeM >= 0) && (NTimeM < 60)){
          TimeY = NTimeY;
          TimeMo = NTimeMo;
          TimeDay = NTimeDay;
          TimeH = NTimeH;
          TimeM = NTimeM;
          TimeS = NTimeS;
          TimeStampDay = (NTimeH*60) + NTimeM;
          #ifdef DEBUG
          Serial.print("Correct time read --- ");
          SerialPrintTime();
          #endif
        }
        else{
          #ifdef DEBUG
          Serial.println("err minute read");
          #endif
        }
      }
      else{
        #ifdef DEBUG
        Serial.println("err hour read");
        #endif
      }
    }
    else{
      #ifdef DEBUG
      Serial.println("err mounth read");
      #endif
    }


}

void SetRTC(){
  //adjus time in RTC module
  DS1307.adjust(DateTime(SetRtcY, SetRtcMo, SetRtcD, SetRtcH, SetRtcM, SetRtcS));
  #ifdef DEBUG
    Serial.println();
    Serial.println("---------- Time changed ----------");
    Serial.print("New time is: ");
    Serial.print(SetRtcH);
    Serial.print(':');
    Serial.print(SetRtcM);
    Serial.print(':');
    Serial.println(SetRtcS);
  #endif
}

void SerialPrintTime(){
  #ifdef DEBUG
  char TimeChar[50];
  sprintf(TimeChar, "%04d/%02d/%02d %02d:%02d:%02d", TimeY, TimeMo, TimeDay, TimeH, TimeM, TimeS);
  Serial.print("Time: " + String(TimeChar));
  Serial.println(" TimeStampDay: " + String(TimeStampDay));
  #endif
}
