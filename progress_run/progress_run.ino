/*
--------------------------NOTES---------------------------------
Use LIB https://github.com/MrKrabat/LED-Strip-Driver-Module for RGB Strip



*/
// defines
#define DEBUG
//#define SET_RTC

//librlies
#include <Wire.h>
#include "RTClib.h"

//declarate variables
//for RTC
int SetRtcY = 2019;
int SetRtcMo = 12;
int SetRtcD = 15;
int SetRtcH = 21;
int SetRtcM = 8;
int SetRtcS = 0;

// var for date
int TimeY = 0;
int TimeMo = 0;
int TimeDay = 0;
int TimeD = 0;
int TimeH = 0;
int TimeM = 0;
int TimeS = 0;

//var for LEDs
int StartLedHourW = 14; // rozsviti se prni LED, postupne se budou zapinat dalsi
int StartLedMinuteW = 0;
int EndLedHourW = 20;
int EndLedMinuteW = 0; //zhasne poslední LED, postupnw zhasnou vsechny
int SpeedLedW = 5; //in minutes
int NumLedW = 6
int NumLedWOn = 0;


//variales led pin (W D22-D27)
#define LedW1 22
#define LedW2 23
#define LedW3 24
#define LedW4 25
#define LedW5 26
#define LedW6 27




//time variable
int DEBUG_TimeS = 0;

//DS1307.adjust(DateTime(SetRtcY, SetRtcMo, SetRtcD, SetRtcH, SetRtcM, SetRtcS));

// add RTC instance
RTC_DS1307 DS1307;

// days
char DayOfTheWeek[7][8] = {"nedele", "pondeli", "utery", "streda", "ctvrtek", "patek", "sobota"};

void setup () {
  #ifdef DEBUG
  // serial comunication via USB
  Serial.begin(9600);
  #endif
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

  initOutput();

}

void loop () {

  GetTime();
  LedWOn();
  //LedWOff();

}

void SetRTC(){
  //adjus time in RTC module
  DS1307.adjust(DateTime(SetRtcY, SetRtcMo, SetRtcD, SetRtcH, SetRtcM, SetRtcS));
}

void initOutput(){ //inicializace output ninu, nastavení na vychozí hodnoty

  pinMode(LedW1, OUTPUT);
  digitlWrite(LedW1, LOW);
  pinMode(LedW2, OUTPUT);
  digitlWrite(LedW2, LOW);
  pinMode(LedW3, OUTPUT);
  digitlWrite(LedW3, LOW);
  pinMode(LedW4, OUTPUT);
  digitlWrite(LedW4, LOW);
  pinMode(LedW5, OUTPUT);
  digitlWrite(LedW5, LOW);
  pinMode(LedW6, OUTPUT);
  digitlWrite(LedW6, LOW);

}

void GetTime(){
  DateTime DateTime = DS1307.now();
  TimeY = DateTime.year();
  TimeMo = DateTime.month();
  TimeDay = DateTime.day();
  TimeH = DateTime.hour();
  TimeM = DateTime.minute();
  TimeS = DateTime.second();

  #ifdef DEBUG
    if(TimeS != DEBUG_TimeS){
      Serial.println();
      Serial.print("Aktualni cas ");
      Serial.print(DateTime.hour());
      Serial.print(':');
      Serial.print(DateTime.minute());
      Serial.print(':');
      Serial.print(DateTime.second());
      Serial.print(", ");
      Serial.print(DayOfTheWeek[DateTime.dayOfTheWeek()]);
      Serial.print(" ");
      Serial.print(DateTime.day());
      Serial.print('.');
      Serial.print(DateTime.month());
      Serial.print('.');
      Serial.print(DateTime.year());
      Serial.println();
      DEBUG_TimeS = TimeS;
    }
  #endif
}

void LedWOn(){
  if(NumLedWOn < NumLedW){
    int LedWOffset = ((NumLedWOn) * SpeedLedW);
    int LedWOffsetMinute =  (StartLedMinuteW + LedWOffset) % 60;
    int LedWOffseHour  = ((StartLedMinuteW + LedWOffset) / 60) + StartLedHourW;
  }
  if(((LedWOffseHour >= TimeH) && (LedWOffsetMinute >= TimeM)) && (((EndLedHourW*100) + EndLedMinuteW) <= ((TimeH * 100) + TimeM)))
}
