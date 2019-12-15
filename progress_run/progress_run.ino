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
int SetRtcH = 20;
int SetRtcM = 40;
int SetRtcS = 0;

// var for date
int TimeY = 0;
int TimeMo = 0;
int TimeDay = 0;
int TimeD = 0;
int TimeH = 0;
int TimeM = 0;
int TimeS = 0;

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
}

void loop () {

  Serial.print(':');

  GetTime();

}

void SetRTC(){
  //adjus time in RTC module
  DS1307.adjust(DateTime(SetRtcY, SetRtcMo, SetRtcD, SetRtcH, SetRtcM, SetRtcS));
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
