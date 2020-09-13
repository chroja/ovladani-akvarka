/*
--------------------------NOTES---------------------------------
Use LIB
LED DRIVER                                    https://github.com/MrKrabat/LED-Strip-Driver-Module for RGB Strip
RTClib                                        https://navody.arduino-shop.cz/docs/texty/0/7/rtclib.zip
DS18B20                                       https://navody.arduino-shop.cz/docs/texty/0/14/dallastemperature.zip
OneWire                                       https://navody.arduino-shop.cz/docs/texty/0/15/onewire.zip



sensor adress 0x28, 0x25, 0xC5, 0xF7, 0x08, 0x00, 0x00, 0x61 vnejsi dlouhy kabel
              0x28, 0xFF, 0x1A, 0x62, 0xC0, 0x17, 0x05, 0xF0 vnejsi kratky kabel
              0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10 v akva
Set

pumpa 520ms/ml


*/


//librlies
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <FastLED.h>
#include <DallasTemperature.h>
#include <DS3231.h>
#include "U8glib.h"
#include <avr/wdt.h>

// defines
#define DEBUG
//#define SET_RTC
#define TEMP_OFFSET
#define SEARCH_ADDRESS_DS18B20
#define RESTART
#define SERIAL_INFO
#define DS3231_USE
#define DRY_RUN
#define CUSTOM_BOARD
#define MESAURE_LED_TEMP

//water sensor
#ifdef DRY_RUN
  //uint8_t T0SensorAddress[8] = { 0x28, 0x25, 0xC5, 0xF7, 0x08, 0x00, 0x00, 0x61 }; //water sensor used on desk - 2wire! red - sign, white - gnd, 4K7 sign -vcc
  uint8_t T0SensorAddress[8] = { 0x28, 0x75, 0x3F, 0x79, 0xA2, 0x16, 0x03, 0xA0 }; //water sensor used on desk
#else
  uint8_t T0SensorAddress[8] = { 0x28, 0xDA, 0xDD, 0xC0, 0x1E, 0x19, 0x01, 0x20 }; //water sensor used in aquarium
#endif

//led sensor
#ifdef DRY_RUN
  //uint8_t T1SensorAddress[8] = { 0x28, 0xC7, 0x25, 0x79, 0xA2, 0x19, 0x03, 0x10 }; //led sensor used on desk
  uint8_t T1SensorAddress[8] = { 0x28, 0x0A, 0x23, 0x79, 0xA2, 0x19, 0x03, 0x59 }; //led sensor used on desk
#else
  uint8_t T1SensorAddress[8] = { 0x28, 0x1E, 0x66, 0xDA, 0x1E, 0x19, 0x01, 0x7F }; //water sensor used in aquarium
#endif



U8GLIB_SH1106_128X64 Oled(0x3c);

//variales led pin (W D22-D27)
#define LedW1 22
#define LedW2 23
#define LedW3 24
#define LedW4 25
#define LedW5 26
#define LedW6 27

#define LightBtnPin 2

#define RelayPin1 34
#define RelayPin2 36
#define RelayPin3 38
#define RelayPin4 40

#ifdef CUSTOM_BOARD
  #define TempPin 39
#else
  #define TempPin 43
#endif

#define NO 1
#define NC 0

#define ON  HIGH
#define OFF LOW
typedef struct {
  int  pin;
  bool type;  // NO = 0, NC = 1
  bool state; // ON = 1, OFF = 0
} rele_t;

rele_t CableHeat;
rele_t Heater;
rele_t Relay3;
rele_t Relay4;





//Led RGB PINOUT
#define RGBLedNum 4
#define RGBDataPin 2
#define RGBClockPin 3
byte Red = 255;
byte Green = 25;
byte Blue = 255;

CRGB RBGLeds[RGBLedNum];

//declarate variables
//for RTC
int SetRtcY = 2020;
int SetRtcMo = 05;
int SetRtcD = 31;
int SetRtcH = 21;
int SetRtcM = 15;
int SetRtcS = 30;

// var for date
int TimeY = 0;
int TimeMo = 0;
int TimeDay = 0;
int TimeD = 0;
int TimeH = 0;
int TimeM = 0;
int TimeS = 0;
int TimeHM = 0;


int NTimeY = 0;
int NTimeMo = 0;
int NTimeDay = 0;
int NTimeD = 0;
int NTimeH = 0;
int NTimeM = 0;
int NTimeS = 0;
int NTimeHM = 0;


unsigned long RtcCurrentMillis = 0;


//var for LEDs
int StartLedHourW = 8; // rozsviti se prni LED, postupne se budou zapinat dalsi
int StartLedMinuteW = 00;
int StartLedW = (StartLedHourW * 100) + StartLedMinuteW;
int EndLedHourW = 21;
int EndLedMinuteW = 30; //zhasne poslední LED, postupnw zhasnou vsechny
int EndLedW = (EndLedHourW * 100) + EndLedMinuteW;
int SpeedLedW = 5; //in minutes
int NumLedW = 6;
int NumLedWOn = 0;
byte StatusLedStrip = 0;
byte ModeLed = 1; // 0 = off; 1 = auto; 3 = off 
byte PrevModeLed = 0;

int OldNumLedWOffset;

//RGB val
int RLedValue = 255;
int GLedValue = 255;
int BLedValue = 255;
int RLedValueOld;
int GLedValueOld;
int BLedValueOld;

//temp
//DS tem sensors
float T0Temp = 0; //temp on T0 with calibration offset
float T1Temp = 0; //temp on T1 with calibration offset
float T0TempNoOffset = 0;
float T1TempNoOffset = 0;
#ifdef TEMP_OFFSET
  float T0Offset = 0;
  float T1Offset = 0;
#else
  float T0Offset = 0;
  float T1Offset = 0;
#endif
/*
int DSCountSensor = 0;
int DSUseSensor = 4;
int DSSetupConnectAttemp = 0;
*/
unsigned long NextReadTepmMs = 0;
int NextReadTepmMin = 0;
int LastReadTemp;
int ErrorTempMax = 1000;
int ErrorTempCurrent = 0;
int TempReadTime = 1;
int TempReadPeriod = 15000; //15 sec
//heat
float TargetTemp = 25;
float DeltaT = 0.5;
int Heat1SafeTemp = 28;
bool CableHeatState = 0;
bool HeaterState = 0;
//unsigned long LastT0HeatStart = 0;
//unsigned long LastT1HeatStart = 0;
//unsigned long LastT0HeatEnd = 0;
//unsigned long LastT1HeatEnd = 0;

float SafeLedTemp = 35;  //degrees
float MaximumLedTemp = 65; //degrees
int PrevLedWOn;


//time variable
int DEBUG_TimeS = 0;
bool FirstRun = 0;
unsigned long  OledRefresh = 0;

// add RTC instance
#ifdef DS3231_USE
DS3231 rtc;
RTCDateTime DateTime;
#else
RTC_DS1307 DS1307;
#endif

bool LightBtnState = 0;
bool PrevLightBtnState = 0;
char LightBtnDir = 1;


void RelayOn(rele_t vstup) {
  // NO musime zapnout 1
  if (vstup.type == NO) {
    digitalWrite(vstup.pin, HIGH);
  }
  // NC musime zapnout 0
  else {
    digitalWrite(vstup.pin, LOW);
  }
  // nastaveni stavove promenne
  vstup.state = ON;
}

void RelayOff(rele_t vstup) {
  // NO musime vypnout 0
  if (vstup.type == NO) {
    digitalWrite(vstup.pin, LOW);
  }
  // NC musime vypnout 1
  else {
    digitalWrite(vstup.pin, HIGH);
  }
  // nastaveni stavove promenne
  vstup.state = OFF;
}

void RelaySwitch(rele_t vstup) {
  // precti stav a nastav opacny
  if (vstup.state == ON) {
    RelayOff(vstup);
  }
  else {
    RelayOn(vstup);
  }
}


//add ds instance
OneWire oneWireDS (TempPin);
DallasTemperature SensorsDS(&oneWireDS);

// days
char DayOfTheWeek[7][8] = {"nedele", "pondeli", "utery", "streda", "ctvrtek", "patek", "sobota"};


//****************************************** SETUP ****************************************

void setup () {

  wdt_enable(WDTO_2S);
//  #ifdef DEBUG
  // serial comunication via USB
  Serial.begin(115200);
  Serial.println("------Start setup-----");
//  #endif
  #ifdef DS3231_USE
    rtc.begin();
  #else
    // chceck if RTC connected
    if (! DS1307.begin()) {
      Serial.println("Hodiny nejsou pripojeny!");
      while (1);
    }
    // kontrolu spuštění obvodu reálného času
    if (! DS1307.isrunning()) {
      Serial.println("Hodiny nejsou spusteny! Spoustim nyni..");
    }
  #endif

  #ifdef SET_RTC
  SetRTC();
  #endif



  FastLED.addLeds<P9813, RGBDataPin, RGBClockPin, RGB>(RBGLeds, RGBLedNum);  // BGR ordering is typical
  for (int i = 0; i < RGBLedNum; i++) {
    RBGLeds[i] = CRGB(0,0,0);
  }
  FastLED.show();

  initPin(LedW1);
  initPin(LedW2);
  initPin(LedW3);
  initPin(LedW4);
  initPin(LedW5);
  initPin(LedW6);

  pinMode(LightBtnPin, INPUT);



  CableHeat.pin = RelayPin1;
  CableHeat.type = NC;
  pinMode(CableHeat.pin, OUTPUT);
  RelayOff(CableHeat);


  Heater.pin = RelayPin2;
  Heater.type = NC;
  pinMode(Heater.pin, OUTPUT);
  RelayOff(Heater);


  Relay3.pin = RelayPin3;
  Relay3.type = NO;
  pinMode(Relay3.pin, OUTPUT);
  RelayOff(Relay3);


  Relay4.pin = RelayPin4;
  Relay4.type = NO;
  pinMode(Relay4.pin, OUTPUT);
  RelayOff(Relay4);


  Wire.begin();


  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\nI2C Scanner");
  I2CScanner();



  GetTimeSetup();

  DiscoverOneWireDevices();
  SensorsDS.begin();
  //SensorsDSRun();
  NextReadTepmMin = TimeM;
  SerialInfoSetup();
  delay (500);
  Serial.println("------End setup-----");
}


void(* resetFunc) (void) = 0; //declare reset function @ address 0

//****************************************** LOOP ****************************************

void loop () {
  wdt_reset();// make sure this gets called at least once every 8 seconds!


  if (millis() >= (RtcCurrentMillis+1000)){
    GetTime();
    RtcCurrentMillis = millis();

  }

  LightBtnRead();


  Led();
  LedWOn();
  LedWOff();

  #ifdef SERIAL_INFO
    SerialInfo();
  #endif
  GetTemp();
  Heat();
  FirstRun = 1;
  TimeRestart();
  CheckLedTemp();
  ShowOled();


}

//****************************************** FUNCTION ****************************************
void initPin(int Pin){
  pinMode(Pin, OUTPUT);
  digitalWrite(Pin, LOW);
}



void SetRTC(){
  #ifdef DS3231_USE
    rtc.setDateTime(__DATE__, __TIME__);
  #else
    DS1307.adjust(DateTime(SetRtcY, SetRtcMo, SetRtcD, SetRtcH, SetRtcM, SetRtcS));
  #endif

  Serial.println();
  Serial.println("---------- Time changed ----------");

  //adjus time in RTC module
  //DS1307.adjust(DateTime(SetRtcY, SetRtcMo, SetRtcD, SetRtcH, SetRtcM, SetRtcS));
/*  #ifdef DEBUG
    Serial.println();
    Serial.println("---------- Time changed ----------");
    Serial.print("New time is: ");
    Serial.print(SetRtcH);
    Serial.print(':');
    Serial.print(SetRtcM);
    Serial.print(':');
    Serial.println(SetRtcS);
    delay (2000);
  #endif*/
}

void GetTimeSetup(){
  #ifdef DS3231_USE
    DateTime = rtc.getDateTime();
    TimeY = DateTime.year;
    TimeMo = DateTime.month;
    TimeDay = DateTime.day;
    TimeH = DateTime.hour;
    TimeM = DateTime.minute;
    TimeS = DateTime.second;
  #else
    DateTime DateTime = DS1307.now();
    TimeY = DateTime.year();
    TimeMo = DateTime.month();
    TimeDay = DateTime.day();
    TimeH = DateTime.hour();
    TimeM = DateTime.minute();
    TimeS = DateTime.second();
  #endif

  TimeHM = (TimeH * 100) + TimeM;

}

void GetTime(){
  #ifdef DS3231_USE
    DateTime = rtc.getDateTime();
    NTimeY = DateTime.year;
    NTimeMo = DateTime.month;
    NTimeDay = DateTime.day;
    NTimeH = DateTime.hour;
    NTimeM = DateTime.minute;
    NTimeS = DateTime.second;
  #else
    DateTime DateTime = DS1307.now();
    NTimeY = DateTime.year();
    NTimeMo = DateTime.month();
    NTimeDay = DateTime.day();
    NTimeH = DateTime.hour();
    NTimeM = DateTime.minute();
    NTimeS = DateTime.second();
  #endif
    if((NTimeMo > 0) && (NTimeMo < 13)){
      if((NTimeH >= 0) && (NTimeH < 24)){
        if((NTimeM >= 0) && (NTimeM < 60)){
          TimeY = NTimeY;
          TimeMo = NTimeMo;
          TimeDay = NTimeDay;
          TimeH = NTimeH;
          TimeM = NTimeM;
          TimeS = NTimeS;
          #ifdef DEBUG
          Serial.println("Correct time read");
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
  TimeHM = (TimeH * 100) + TimeM;

}




void LedWOn(){
  int LedWOffset;
  int LedWOffsetMinute;
  int LedWOffseHour;
  int LedWoffsetTime;
  if((NumLedWOn <= NumLedW) && (StatusLedStrip != 1)){
    LedWOffset = ((NumLedWOn) * SpeedLedW);
    LedWOffsetMinute =  (StartLedMinuteW + LedWOffset) % 60;
    LedWOffseHour  = ((StartLedMinuteW + LedWOffset) / 60) + StartLedHourW;
    LedWoffsetTime = (LedWOffseHour * 100) + LedWOffsetMinute;

    #ifdef DEBUG
    if(OldNumLedWOffset != NumLedWOn){
      Serial.print("Next strip in time: ");
      Serial.println(LedWoffsetTime);
      OldNumLedWOffset = NumLedWOn;
    }
    #endif
  }
  if((LedWoffsetTime <= TimeHM) && (EndLedW >= TimeHM) && (StatusLedStrip != 1)){
    if(NumLedWOn < NumLedW){
      NumLedWOn = NumLedWOn + 1;

      #ifdef DEBUG
      Serial.print("NumLedWOn: ");
      Serial.println(NumLedWOn);
      #endif
    }
    else if (NumLedWOn == NumLedW){
      StatusLedStrip = 1;



      #ifdef DEBUG
      Serial.print("Status led strip W change to: ");
      Serial.println(StatusLedStrip);
      #endif
    }
    else{
      Serial.println("Error in function LedWOn. Loop is broken :(");
    }

    LedWSwitch();
  }
}

void LedWOff(){
  int LedWOffset;
  int LedWOffsetMinute;
  int LedWOffseHour;
  int LedWoffsetTime;
  if((NumLedWOn > 0) && (StatusLedStrip != 0)){
    LedWOffset = ((NumLedW - NumLedWOn) * SpeedLedW);
    LedWOffsetMinute =  (EndLedMinuteW + LedWOffset) % 60;
    LedWOffseHour  = ((EndLedMinuteW + LedWOffset) / 60) + EndLedHourW;
    LedWoffsetTime = (LedWOffseHour * 100) + LedWOffsetMinute;

    #ifdef DEBUG
    if(OldNumLedWOffset != NumLedWOn){
      Serial.print("Next strip in time: ");
      Serial.println(LedWoffsetTime);
      OldNumLedWOffset = NumLedWOn;
    }
    #endif
  }
  if((LedWoffsetTime <= TimeHM) && (StatusLedStrip != 0)){
    if(NumLedWOn > 0){
      NumLedWOn = NumLedWOn - 1;

      #ifdef DEBUG
      Serial.print("NumLedWOn: ");
      Serial.println(NumLedWOn);
      #endif
    }
    else if (NumLedWOn == 0){
      StatusLedStrip = 0;

      #ifdef DEBUG
      Serial.print("Status led strip W change to: ");
      Serial.println(StatusLedStrip);
      #endif
    }
    else{
      Serial.println("Error in function LedWOff. Loop is broken :(");
    }

    LedWSwitch();
  }
}

void LedWSwitch() {
  switch(NumLedWOn){
    case 0:
      digitalWrite(LedW1, LOW);
      digitalWrite(LedW2, LOW);
      digitalWrite(LedW3, LOW);
      digitalWrite(LedW4, LOW);
      digitalWrite(LedW5, LOW);
      digitalWrite(LedW6, LOW);
      for (int i = 0; i < RGBLedNum; i++) { //RGBLedNum
        RBGLeds[i] = CRGB(0,0,0);
      }
      FastLED.show();
      #ifdef DEBUG
      Serial.println("All white LED strip are off.");
      #endif
      break;
    case 1:
      digitalWrite(LedW1, HIGH);
      digitalWrite(LedW2, LOW);
      digitalWrite(LedW3, LOW);
      digitalWrite(LedW4, LOW);
      digitalWrite(LedW5, LOW);
      digitalWrite(LedW6, LOW);
      for (int i = 0; i < 2; i++) { //RGBLedNum
        RBGLeds[i] = CRGB(Red,0,0);
      }
      FastLED.show();
      #ifdef DEBUG
      Serial.println("1 white LED strip is on");
      #endif
      break;
    case 2:
      digitalWrite(LedW1, HIGH);
      digitalWrite(LedW2, HIGH);
      digitalWrite(LedW3, LOW);
      digitalWrite(LedW4, LOW);
      digitalWrite(LedW5, LOW);
      digitalWrite(LedW6, LOW);
      RBGLeds[2] = CRGB(Red,0,0);
      FastLED.show();
      #ifdef DEBUG
      Serial.println("1-2 white LED strip are on");
      #endif
      break;
    case 3:
      digitalWrite(LedW1, HIGH);
      digitalWrite(LedW2, HIGH);
      digitalWrite(LedW3, HIGH);
      digitalWrite(LedW4, LOW);
      digitalWrite(LedW5, LOW);
      digitalWrite(LedW6, LOW);
      RBGLeds[3] = CRGB(Red,0,0);
      FastLED.show();
      #ifdef DEBUG
      Serial.println("1-3 white LED strip are on");
      #endif
      break;
    case 4:
      digitalWrite(LedW1, HIGH);
      digitalWrite(LedW2, HIGH);
      digitalWrite(LedW3, HIGH);
      digitalWrite(LedW4, HIGH);
      digitalWrite(LedW5, LOW);
      digitalWrite(LedW6, LOW);
      #ifdef DEBUG
      Serial.println("1-4 white LED strip are on");
      #endif
      break;
    case 5:
      digitalWrite(LedW1, HIGH);
      digitalWrite(LedW2, HIGH);
      digitalWrite(LedW3, HIGH);
      digitalWrite(LedW4, HIGH);
      digitalWrite(LedW5, HIGH);
      digitalWrite(LedW6, LOW);
      for (int i = 0; i < RGBLedNum; i++) {
        RBGLeds[i] = CRGB(Red,0,0);
      }
      FastLED.show();
      #ifdef DEBUG
      Serial.println("1-5 white LED strip are on");
      #endif
      break;
    case 6:
      digitalWrite(LedW1, HIGH);
      digitalWrite(LedW2, HIGH);
      digitalWrite(LedW3, HIGH);
      digitalWrite(LedW4, HIGH);
      digitalWrite(LedW5, HIGH);
      digitalWrite(LedW6, HIGH);
      for (int i = 0; i < RGBLedNum; i++) {
        RBGLeds[i] = CRGB(Red,Green,Blue);
      }
      FastLED.show();
      #ifdef DEBUG
      Serial.println("1-6 white LED strip are on");
      #endif
      break;
    default:
      digitalWrite(LedW1, LOW);
      digitalWrite(LedW2, LOW);
      digitalWrite(LedW3, LOW);
      digitalWrite(LedW4, LOW);
      digitalWrite(LedW5, LOW);
      digitalWrite(LedW6, LOW);
      #ifdef DEBUG
      Serial.println("All white LED strip are off. Case default!!");
      #endif
      break;
  }
}


void SerialInfoSetup(){
  #ifdef DEBUG
    Serial.println();
    Serial.println("---------------------SETUP INFO------------------------");
    Serial.println("Version: " + String(SetRtcY) + String(SetRtcMo) + String(SetRtcD));
    Serial.println("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
    Serial.println("White led start time (HH:MM): " + String(StartLedHourW) + ":" + String(StartLedMinuteW) + " White led end time (HH:MM): " + String(EndLedHourW) + ":" + String(EndLedMinuteW) + " Offset for each strip (in minutes): " + String(SpeedLedW) + " Maximum white led strip (num): " + String(NumLedW));
    //Serial.println("Red value: " + String(RLedValue) + " Green value: " + String(GLedValue) + " Blue value: " + String(BLedValue));
    Serial.println("---------------------END SETUP INFO------------------------");



  #endif
}

void SerialInfo(){
  #ifdef DEBUG
    if(TimeS != DEBUG_TimeS){
      Serial.println();
      Serial.println("------------------------------------------------------------");
      Serial.println("-------------------Start serial info------------------------");
      Serial.println("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
      Serial.println("Numbers LED stip white on: " + String(NumLedWOn));
      Serial.println("Heat cable status: " + String(CableHeatState));
      Serial.println("Heater  status: " + String(HeaterState));
      Serial.println("T0 Temp (water): " + String(T0Temp));
      Serial.println("T1 Temp (LED): " + String(T1Temp));

      DEBUG_TimeS = TimeS;
      Serial.println();
      Serial.println("-------------------End serial info--------------------------");
      Serial.println("------------------------------------------------------------");
    }


  #endif
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
  resetFunc();  //call reset

  #endif
}

void TimeRestart(){
  if ((TimeDay == 1) && (TimeH == 0) && (TimeM == 0) && (RtcCurrentMillis >= 120000)){
    Serial.println("resetting");
  resetFunc();  //call reset
  }
}



void GetTemp(){
  if (NextReadTepmMs <= millis()){
      LedWSwitch(); //pokud dojde k chybě rozsvícení ledek, tak při měření teploty se opraví
    #ifdef DEBUG
     Serial.println("******** Start measure temp *******\n");
    #endif
    SensorsDS.requestTemperatures();
    T0TempNoOffset = ReadTemperature(T0SensorAddress);

    #ifdef MESAURE_LED_TEMP
      T1TempNoOffset = ReadTemperature(T1SensorAddress);
    #else 
      T1TempNoOffset = T0TempNoOffset;
    #endif
    
    NextReadTepmMs = NextReadTepmMs + TempReadPeriod;
    LastReadTemp = TimeHM;
    #ifdef DEBUG
      Serial.println("Temp read.");
      Serial.println("T0 read temp is: " + String(T0TempNoOffset) + "°C");

      Serial.println("Temp read.");
      Serial.println("T1 read temp is: " + String(T1TempNoOffset) + "°C");

      Serial.println("Current measure in time: " + String(LastReadTemp));
    #endif
    if ((T0TempNoOffset > -127) && (T0TempNoOffset < 85)){
      if ((T1TempNoOffset > -127) && (T1TempNoOffset < 85)){
        ErrorTempCurrent = 0;
        T0Temp = T0TempNoOffset + T0Offset;
        T1Temp = T1TempNoOffset + T1Offset;

        #ifdef DEBUG
          Serial.println("\n--- Temp measure is ok. ---\n");
          Serial.println("T0 with offset temp is: " + String(T0Temp) + "°C");
          Serial.println("T1 with offset temp is: " + String(T1Temp) + "°C");
        #endif
      }
    }
    else if (ErrorTempCurrent < ErrorTempMax){
      #ifdef DEBUG
        Serial.println("Temp measure is FAIL.");
        Serial.println("Error Temp counter is: " + String(ErrorTempCurrent+1) + " / " + String(ErrorTempMax));
      #endif
      ErrorTempCurrent ++;
    }
    else{
      Restart("Lot of error measure. Total: ", ErrorTempCurrent);
    }
    Serial.println("\n******* END measure temp **********");
    Serial.println(); Serial.println();
  }
}

void DiscoverOneWireDevices(void) {
  #ifdef SEARCH_ADDRESS_DS18B20
    byte i;
    byte present = 0;
    byte data[12];
    byte addr[8];

    Serial.print("Looking for 1-Wire devices...\n\r");
    while(oneWireDS.search(addr)) {
      Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
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
    Serial.print("\n\r\n\rThat's it.\r\n");
    oneWireDS.reset_search();
    delay (500);
    return;
  #endif
}

float ReadTemperature(DeviceAddress deviceAddress){
  float tempC = SensorsDS.getTempC(deviceAddress);
  return tempC;
}


void Heat(){
  if(T0Temp != 0){
    if((T0Temp <= TargetTemp) && (CableHeatState != 1)){
      CableHeatState = 1;
      RelayOn(CableHeat);
      #ifdef DEBUG
        Serial.println("Heat cable is on.");
      #endif
    }
    else if((T0Temp >= (TargetTemp + DeltaT)) && (CableHeatState == 1)){
      CableHeatState = 0;
      RelayOff(CableHeat);
      #ifdef DEBUG
        Serial.println("Heat cable is off. Standart turn off.");
      #endif
    }

    if ((T0Temp <= (TargetTemp - DeltaT)) && (HeaterState != 1)){
      HeaterState = 1;
      RelayOn(Heater);
      #ifdef DEBUG
        Serial.println("Heater in water is on.");
      #endif
    }
    else if((T0Temp >= TargetTemp) && (HeaterState == 1)){
      HeaterState = 0;

      RelayOff(Heater);
      #ifdef DEBUG
        Serial.println("Heater in water is off.");
      #endif
    }

  }
  else{
    if(FirstRun == 0){
      #ifdef DEBUG
        Serial.println("Temp isn´t read.");
      #endif
    }
  }
}

void I2CScanner(){
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
   Serial.println("done\n");
}

void ShowOled(){
  if (millis()-OledRefresh > 100) {
    Oled.firstPage();
    do {
      TempTimeStatusPage();
    }
  while( Oled.nextPage());
    OledRefresh = millis();
  }
}

void TempTimeStatusPage(void){
  Oled.setFont(u8g_font_unifont);
  Oled.setPrintPos(0, 10);
  Oled.print("Time: "); Oled.print(TimeH);  Oled.print(":");  Oled.print(TimeM);  Oled.print(":");  Oled.print(TimeS);
  Oled.setPrintPos(0, 25);
  Oled.print("Water: "); Oled.print(T0Temp);  Oled.print(" C");
  Oled.setPrintPos(0, 40);
  Oled.print("Heat cable: ");
  if(CableHeatState == 0){
    Oled.print("OFF");
  }
  else if (CableHeatState == 1){
    Oled.print("ON");
  }
  else {
    Oled.print("ERR");
  }
  Oled.setPrintPos(0, 55);
  Oled.print("Heater: ");
  if(HeaterState == 0){
    Oled.print("OFF");
  }
  else if (HeaterState == 1){
    Oled.print("ON");
  }
  else {
    Oled.print("ERR");
  }
}

void CheckLedTemp(){
  if(StatusLedStrip == 1){
    if (T1Temp > MaximumLedTemp){
      PrevLedWOn = NumLedWOn;
      NumLedWOn = 0;
      if (NumLedWOn != PrevLedWOn){
        Serial.println("\n\n********** Temperature led light exceed maximum operating temperature **********\n");
        LedWSwitch();
      }
    }
    if (T1Temp > SafeLedTemp){
      PrevLedWOn = NumLedWOn;
      NumLedWOn = map(T1Temp, SafeLedTemp, MaximumLedTemp, NumLedW-1, 0);
      if (NumLedWOn != PrevLedWOn){
        Serial.println("\n\n***** Temperature led light exceed safety operating temperature *****\n");
        Serial.print("Now lighting: " + String(NumLedWOn) + " white led strip(s)\n");
        LedWSwitch();
      }
    }
  }
}


void Led(){
  if (ModeLed == 0){
    if (PrevModeLed != 0)  {
      PrevModeLed = ModeLed;
      Serial.println("Led set to off mode.");
      NumLedWOn = 0;
      LedWSwitch();
    }
  }
  else if (ModeLed == 2) {
    if (PrevModeLed != 2) {
      PrevModeLed = ModeLed;
      Serial.println("Led set to on mode.");
      NumLedWOn = 6;
      LedWSwitch();
    }
  }
  else if (ModeLed == 1){
    if (PrevModeLed != 1){
      PrevModeLed = ModeLed;
      Serial.println("Led set to auto mode.");
    }
    LedWOn();
    LedWOff();
  }
  else
  {
    //Serial.println("invalid set Led mode");
  }
 }

 void LightBtnRead (){
   LightBtnState = digitalRead(LightBtnPin);
   if ((digitalRead(LightBtnPin) == 1) && (PrevLightBtnState != 1)){
     delay(20);
     if ((digitalRead(LightBtnPin) == 1) && (PrevLightBtnState != 1)){
      ModeLed = ModeLed + LightBtnDir;
      LightBtnState = digitalRead(LightBtnPin);
      Serial.println("func LightBtnRead -- ModeLed: " + String(ModeLed));
       if((ModeLed == 0) || (ModeLed == 2)){
        LightBtnDir = LightBtnDir * (-1);
        if(LightBtnDir > 0){
          Serial.println("func LightBtnRead -- next LightBtnDir: +");
        }
        if(LightBtnDir < 0){
          Serial.println("func LightBtnRead -- next LightBtnDir: -");
        }
        
      }
     }
   }
  PrevLightBtnState = LightBtnState;



 }