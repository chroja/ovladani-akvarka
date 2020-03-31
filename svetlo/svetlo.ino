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
// defines
#define DEBUG
//#define SET_RTC
//#define DEBUG_LED
//#define TEMP_OFFSET
//#define SEARCH_ADDRESS_DS18B20
//#define DRY_RUN
//#define RESTART
#define SERIAL_INFO
//#define FERTILIZATION



//variales led pin (W D22-D27)
#define LedW1 22
#define LedW2 23
#define LedW3 24
#define LedW4 25
#define LedW5 26
#define LedW6 27

//Led RGB PINOUT
#define RGBLedNum 4
#define RGBDataPin 2
#define RGBClockPin 3
byte Red = 255;
byte Green = 25;
byte Blue = 255;



//librlies
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <FastLED.h>

CRGB RBGLeds[RGBLedNum];

//declarate variables
//for RTC
int SetRtcY = 2020;
int SetRtcMo = 03;
int SetRtcD = 29;
int SetRtcH = 20;
int SetRtcM = 16;
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
int StartLedHourW = 9; // rozsviti se prni LED, postupne se budou zapinat dalsi
int StartLedMinuteW = 30;
int StartLedW = (StartLedHourW * 100) + StartLedMinuteW;
int EndLedHourW = 21;
int EndLedMinuteW = 30; //zhasne poslední LED, postupnw zhasnou vsechny
int EndLedW = (EndLedHourW * 100) + EndLedMinuteW;
int SpeedLedW = 5; //in minutes
int NumLedW = 6;
int NumLedWOn = 0;
byte StatusLedStrip = 0;
int OldNumLedWOffset;

//RGB val
int RLedValue = 255;
int GLedValue = 255;
int BLedValue = 255;
int RLedValueOld;
int GLedValueOld;
int BLedValueOld;




//time variable
int DEBUG_TimeS = 0;
bool FirstRun = 0;

// add RTC instance
RTC_DS1307 DS1307;

//add ds instance


// days
char DayOfTheWeek[7][8] = {"nedele", "pondeli", "utery", "streda", "ctvrtek", "patek", "sobota"};


//****************************************** SETUP ****************************************

void setup () {
//  #ifdef DEBUG
  // serial comunication via USB
  Serial.begin(115200);
  Serial.println("------Start setup-----");
//  #endif

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

  FastLED.addLeds<P9813, RGBDataPin, RGBClockPin, RGB>(RBGLeds, RGBLedNum);  // BGR ordering is typical
  for (int i = 0; i < RGBLedNum; i++) {
    RBGLeds[i] = CRGB(0,0,0);
  }
  FastLED.show();

  initOutput();

  GetTimeSetup();

  SerialInfoSetup();
  delay (500);
  Serial.println("------End setup-----");
}


void(* resetFunc) (void) = 0; //declare reset function @ address 0

//****************************************** LOOP ****************************************

void loop () {
  if (millis() >= (RtcCurrentMillis+3000)){
    #ifdef DEBUG
    Serial.println("time read");
  //  Serial.println(RtcCurrentMillis);
    #endif
    GetTime();
    RtcCurrentMillis = millis();

  }

  //GetTime();
  //LedW();

  LedWOn();
  LedWOff();

  #ifdef SERIAL_INFO
    SerialInfo();
  #endif

  FirstRun = 1;
  TimeRestart();
}

//****************************************** FUNCTION ****************************************

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
    delay (2000);
  #endif
}

void initOutput(){ //inicializace output ninu, nastavení na vychozí hodnoty
  //white led

  pinMode(LedW1, OUTPUT);
  digitalWrite(LedW1, LOW);
  pinMode(LedW2, OUTPUT);
  digitalWrite(LedW2, LOW);
  pinMode(LedW3, OUTPUT);
  digitalWrite(LedW3, LOW);
  pinMode(LedW4, OUTPUT);
  digitalWrite(LedW4, LOW);
  pinMode(LedW5, OUTPUT);
  digitalWrite(LedW5, LOW);
  pinMode(LedW6, OUTPUT);
  digitalWrite(LedW6, LOW);

}

void GetTimeSetup(){
  DateTime DateTime = DS1307.now();
  TimeY = DateTime.year();
  TimeMo = DateTime.month();
  TimeDay = DateTime.day();
  TimeH = DateTime.hour();
  TimeM = DateTime.minute();
  TimeS = DateTime.second();
  TimeHM = (TimeH * 100) + TimeM;

}

void GetTime(){
  DateTime DateTime = DS1307.now();
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


void LedW(){
  if ((TimeHM >= StartLedW) && (TimeHM < EndLedW) && (NumLedWOn != NumLedW)){
    NumLedWOn = NumLedW;
    LedWSwitch();
    for (int i = 0; i < RGBLedNum; i++) {
      RBGLeds[i] = CRGB(Red,Green,Blue);
    }
    FastLED.show();

  }
  else if(((TimeHM < StartLedW) || (TimeHM >= EndLedW)) && (NumLedWOn != 0)) {
    NumLedWOn = 0;
    LedWSwitch();
    for (int i = 0; i < RGBLedNum; i++) {
      RBGLeds[i] = CRGB(0,0,0);
    }
    FastLED.show();
    }
    else{

    }
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
  digitalWrite(RestartPin, LOW);
  #endif
}

void TimeRestart(){
  if ((TimeDay == 1) && (TimeH == 0) && (TimeM == 0) && (RtcCurrentMillis >= 120000)){
    Serial.println("resetting");
  resetFunc();  //call reset
  }
}
