/*
--------------------------NOTES---------------------------------
Use LIB https://github.com/MrKrabat/LED-Strip-Driver-Module for RGB Strip



*/
// defines
#define DEBUG
#define SET_RTC
#define DEBUG_W_LED

//librlies
#include <Wire.h>
#include "RTClib.h"

//declarate variables
//for RTC
int SetRtcY = 2019;
int SetRtcMo = 12;
int SetRtcD = 21;
int SetRtcH = 12;
int SetRtcM = 14;
int SetRtcS = 50;

// var for date
int TimeY = 0;
int TimeMo = 0;
int TimeDay = 0;
int TimeD = 0;
int TimeH = 0;
int TimeM = 0;
int TimeS = 0;
int TimeHM = 0;

//var for LEDs
int StartLedHourW = 15; // rozsviti se prni LED, postupne se budou zapinat dalsi
int StartLedMinuteW = 0;
int StartLedW = (StartLedHourW * 100) + StartLedMinuteW;
int EndLedHourW = 16;
int EndLedMinuteW = 0; //zhasne poslední LED, postupnw zhasnou vsechny
int EndLedW = (EndLedHourW * 100) + EndLedMinuteW;
int SpeedLedW = 3; //in minutes
int NumLedW = 6;
int NumLedWOn = 0;
byte StatusLedStrip = 0; //status 0 = unknown, 1 = min, 2 = step; 3 = max
int OldNumLedWOffset;

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

  #ifdef DEBUG_W_LED
  DebugWLED();
  #endif

}

void loop () {

  GetTime();
  LedWOn();
  LedWOff();
  #ifdef DEBUG_W_LED
  if (TimeHM == 1620){
    SetRTC();
  }
  #endif

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

void initOutput(){ //inicializace output ninu, nastavení na vychozí hodnoty

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

void DebugWLED(){
  #ifdef DEBUG_W_LED
  Serial.println();
  Serial.println("---------- Start test white led ----------");
  digitalWrite(LedW1, HIGH);
  delay (300);
  digitalWrite(LedW2, HIGH);
  delay (300);
  digitalWrite(LedW3, HIGH);
  delay (300);
  digitalWrite(LedW4, HIGH);
  delay (300);
  digitalWrite(LedW5, HIGH);
  delay (300);
  digitalWrite(LedW6, HIGH);
  delay (500);
  digitalWrite(LedW1, LOW);
  digitalWrite(LedW2, LOW);
  digitalWrite(LedW3, LOW);
  digitalWrite(LedW4, LOW);
  digitalWrite(LedW5, LOW);
  digitalWrite(LedW6, LOW);
  delay(300);
  digitalWrite(LedW1, HIGH);
  digitalWrite(LedW2, HIGH);
  digitalWrite(LedW3, HIGH);
  digitalWrite(LedW4, HIGH);
  digitalWrite(LedW5, HIGH);
  digitalWrite(LedW6, HIGH);
  delay(500);
  digitalWrite(LedW1, LOW);
  digitalWrite(LedW2, LOW);
  digitalWrite(LedW3, LOW);
  digitalWrite(LedW4, LOW);
  digitalWrite(LedW5, LOW);
  digitalWrite(LedW6, LOW);
  Serial.println();
  Serial.println("---------- End test white led ----------");
  #endif
}

void GetTime(){
  DateTime DateTime = DS1307.now();
  TimeY = DateTime.year();
  TimeMo = DateTime.month();
  TimeDay = DateTime.day();
  TimeH = DateTime.hour();
  TimeM = DateTime.minute();
  TimeS = DateTime.second();
  //TimeHM = (TimeH * 100) + TimeM;
  TimeHM = (TimeM * 100) + TimeS;

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
      Serial.print(" hodiny a minty (TimeHM): ");
      Serial.print(TimeHM);
      Serial.println();
      DEBUG_TimeS = TimeS;
    }
  #endif
}

void LedWOn(){
  int LedWOffset;
  int LedWOffsetMinute;
  int LedWOffseHour;
  int LedWoffsetTime;
  if((NumLedWOn <= NumLedW) && (StatusLedStrip != 3)){
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
  if((LedWoffsetTime <= TimeHM) && (EndLedW >= TimeHM) && (StatusLedStrip != 3)){
    if(NumLedWOn < NumLedW){
      NumLedWOn = NumLedWOn + 1;

      #ifdef DEBUG
      Serial.print("NumLedWOn: ");
      Serial.println(NumLedWOn);
      #endif
    }
    else if (NumLedWOn == NumLedW){
      StatusLedStrip = 3;

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
  if((NumLedWOn > 0) && (StatusLedStrip != 1)){
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
  if((LedWoffsetTime <= TimeHM) /*&& (StartLedW >= TimeHM)*/ && (StatusLedStrip != 1)){
    if(NumLedWOn > 0){
      NumLedWOn = NumLedWOn - 1;

      #ifdef DEBUG
      Serial.print("NumLedWOn: ");
      Serial.println(NumLedWOn);
      #endif
    }
    else if (NumLedWOn == 0){
      StatusLedStrip = 1;

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
