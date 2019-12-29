/*
--------------------------NOTES---------------------------------
Use LIB https://github.com/MrKrabat/LED-Strip-Driver-Module for RGB Strip


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
#define TEMP_OFFSET
//#define SEARCH_ADDRESS_DS18B20
#define DRY_RUN
#define RESTART
#define SERIAL_INFO

uint8_t T0SensorAddress[8] = { 0x28, 0x25, 0xC5, 0xF7, 0x08, 0x00, 0x00, 0x61 }; //water sensor
#ifdef DRY_RUN
  uint8_t T1SensorAddress[8] = { 0x28, 0xFF, 0x1A, 0x62, 0xC0, 0x17, 0x05, 0xF0 }; //cable sensor - test!
#else
  uint8_t T1SensorAddress[8] = { 0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10 }; //cable sensor
#endif


//variales led pin (W D22-D27)
#define LedW1 22
#define LedW2 23
#define LedW3 24
#define LedW4 25
#define LedW5 26
#define LedW6 27

//Led RGB PINOUT
#define RLedPwmPin 44
#define GLedPwmPin 45
#define BLedPwmPin 46

//temp
#define Heat0  37//heat water rellay
#define Heat1  39//heat cable rellay
#define TempPin 40
#define RestartSensorPin 41
#define RestartPin 53



//librlies
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//declarate variables
//for RTC
int SetRtcY = 2019;
int SetRtcMo = 12;
int SetRtcD = 25;
int SetRtcH = 18;
int SetRtcM = 33;
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

//var for LEDs
int StartLedHourW = 12; // rozsviti se prni LED, postupne se budou zapinat dalsi
int StartLedMinuteW = 0;
int StartLedW = (StartLedHourW * 100) + StartLedMinuteW;
int EndLedHourW = 20;
int EndLedMinuteW = 00; //zhasne poslední LED, postupnw zhasnou vsechny
int EndLedW = (EndLedHourW * 100) + EndLedMinuteW;
int SpeedLedW = 5; //in minutes
int NumLedW = 6;
int NumLedWOn = 0;
byte StatusLedStrip = 0; //status 0 = unknown, 1 = min, 2 = step; 3 = max
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
  float T0Offset = -0.8;
  float T1Offset = -0.54;
#else
  float T0Offset = 0;
  float T1Offset = 0;
#endif
int DSCountSensor = 0;
int DSUseSensor = 2;
int DSSetupConnectAttemp = 0;
int NextReadTepmMin = 0;
int LastReadTemp;
int ErrorTempMax = 10;
int ErrorTempCurrent = 0;
int TempReadTime = 1;
//heat
float TargetTemp = 23;
float DeltaT = 0.5;
int Heat1SafeTemp = 28;
bool Heat0State = 0;
bool Heat1State = 0;




//time variable
int DEBUG_TimeS = 0;
bool FirstRun = 0;

// add RTC instance
RTC_DS1307 DS1307;

//add ds instance
OneWire oneWireDS (TempPin);
DallasTemperature SensorsDS(&oneWireDS);

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

  initOutput();

  #ifdef DEBUG_LED
  DebugLED();
  #endif
  GetTime();
  DiscoverOneWireDevices();
  SensorsDS.begin();
  SensorsDSRun();

  SerialInfoSetup();

  NextReadTepmMin = TimeM;
  if(TimeS > 45){
    NextReadTepmMin ++;
  }
  delay (500);
  Serial.println("------End setup-----");
}

//****************************************** LOOP ****************************************

void loop () {

  GetTime();
  GetTemp();
  LedWOn();
  LedWOff();
  LedRGB();
  Heat();

  #ifdef SERIAL_INFO
    SerialInfo();
  #endif

  FirstRun = 1;
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
  #endif
}

void initOutput(){ //inicializace output ninu, nastavení na vychozí hodnoty
  //white led
  digitalWrite(RestartPin, HIGH);
  pinMode(RestartPin, OUTPUT);
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

  //GRB led;
  pinMode(RLedPwmPin, OUTPUT);
  analogWrite(RLedPwmPin, 0);
  pinMode(GLedPwmPin, OUTPUT);
  analogWrite(GLedPwmPin, 0);
  pinMode(GLedPwmPin, OUTPUT);
  analogWrite(GLedPwmPin, 0);

  //
  pinMode(RestartSensorPin, OUTPUT);
  digitalWrite(RestartSensorPin, LOW);
  //heating
  pinMode(Heat0, OUTPUT);
  digitalWrite(Heat0, !Heat0State);
  pinMode(Heat1, OUTPUT);
  digitalWrite(Heat1, !Heat1State);

}

void SensorsDSRun(){
  DSCountSensor = SensorsDS.getDeviceCount();
  delay(1000);
  while(DSCountSensor != DSUseSensor){
    if(DSSetupConnectAttemp < 2){

      digitalWrite(RestartSensorPin, HIGH);
      delay(100);
      digitalWrite(RestartSensorPin, LOW);
      delay(100);
      DSCountSensor = SensorsDS.getDeviceCount();
      //delay(1000);
      Serial.println("Detected " + String(DSCountSensor) + " sensor(s). Attemp connected: " + String(DSSetupConnectAttemp + 1));
      DSSetupConnectAttemp ++;
    }
    else{
      Restart("Detected different number connected sensors!", DSCountSensor);
    }
  }
}

void DebugLED(){
  #ifdef DEBUG_LED
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
  delay(300);
  RGB_color(255, 0, 0);
  delay(500);
  RGB_color(0, 255, 0);
  delay(500);
  RGB_color(0, 0, 255);
  delay(500);
  RGB_color(0, 0, 0);

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
  TimeHM = (TimeH * 100) + TimeM;

}

void GetTemp(){
  if (NextReadTepmMin == TimeM){
    #ifdef DEBUG
     Serial.println("Start measure temp T0 + T1");
    #endif
    SensorsDS.requestTemperatures();
    T0TempNoOffset = ReadTemperature(T0SensorAddress);
    T1TempNoOffset = ReadTemperature(T1SensorAddress);
    NextReadTepmMin = (NextReadTepmMin + TempReadTime) % 60;
    LastReadTemp = TimeHM;
    #ifdef DEBUG
      Serial.println("Temp read.");
      Serial.println("T0 read temp is: " + String(T0TempNoOffset) + "C");
      Serial.println("T1 read temp is: " + String(T1TempNoOffset) + "C");
      Serial.println("Current measure in time: " + String(LastReadTemp) + " Next Read in minute: " + String(NextReadTepmMin));
    #endif
    if (((T0TempNoOffset > (-127)) && (T0TempNoOffset < (85))) && ((T1TempNoOffset > (-127)) && (T1TempNoOffset < (85)))){
      ErrorTempCurrent = 0;
      T0Temp = T0TempNoOffset + T0Offset;
      T1Temp = T1TempNoOffset + T1Offset;

      #ifdef DEBUG
        Serial.println("Temp measure is ok.");
        Serial.println("T0 with offset temp is: " + String(T0Temp) + "C");
        Serial.println("T1 with offset temp is: " + String(T1Temp) + "C");
      #endif
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
    Serial.println(); Serial.println();
  }
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
  if((LedWoffsetTime <= TimeHM) && (StatusLedStrip != 1)){
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

void LedRGB(){
  if(NumLedWOn == NumLedW){
    RGB_color(RLedValue, GLedValue, BLedValue);
    }
  else if (NumLedWOn < NumLedW){
    RGB_color(0, 0, 0);
    }
  else{

  }
}

void RGB_color(int red, int green, int blue){

  if((red != RLedValueOld) || (green != GLedValueOld) || (blue != BLedValueOld)){
    analogWrite(RLedPwmPin, red);
    analogWrite(GLedPwmPin, green);
    analogWrite(BLedPwmPin, blue);


    #ifdef DEBUG
    Serial.println("RGB color set to (RGB 0-255): " + String(red) + ", " + String(green) + ", " + String(blue));
    RLedValueOld = red;
    GLedValueOld = green;
    BLedValueOld = blue;
    #endif
  }
}

void SerialInfoSetup(){
  #ifdef DEBUG
    Serial.println();
    Serial.println("---------------------SETUP INFO------------------------");
    Serial.println("Version: " + String(SetRtcY) + String(SetRtcMo) + String(SetRtcD));
    Serial.println("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
    Serial.println("White led start time (HH:MM): " + String(StartLedHourW) + ":" + String(StartLedMinuteW) + " White led end time (HH:MM): " + String(EndLedHourW) + ":" + String(EndLedMinuteW) + " Offset for each strip (in minutes): " + String(SpeedLedW) + " Maximum white led strip (num): " + String(NumLedW));
    Serial.println("Red value: " + String(RLedValue) + " Green value: " + String(GLedValue) + " Blue value: " + String(BLedValue));
    Serial.print("Address T0 (water) sensor: ");
    PrintSensorAdress(T0SensorAddress);
    Serial.println();
    Serial.print("Address T1 (cable) sensor: ");
    PrintSensorAdress(T1SensorAddress);
    Serial.println();
    Serial.println("DS18B20 conected sensors: " + String(DSCountSensor));
    Serial.println("Temp set to: " + String(TargetTemp) + "°C with delta temp +- " + String(DeltaT) + "°C (" + String(TargetTemp - DeltaT) + "°C - " + String(TargetTemp + DeltaT) + "°C)");
    Serial.println("Safety temp for Heat cable is: " + String(Heat1SafeTemp) + "°C");

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
      Serial.println("Water temp is: " + String(T0Temp) + "°C. Cable temp is: " + String(T1Temp) + "°C.");
      //Serial.print("The last temperature measured was at: " + String(LastReadTemp,3) + ". Measure every " + String(TempReadTime) + " minutes.");
      Serial.print("The last temperature measured was at: ");
      if (LastReadTemp<100){Serial.print("00");} else if (LastReadTemp<1000){Serial.print("0");}
      Serial.println(String(LastReadTemp) + ". Measure every " + String(TempReadTime) + " minutes.");
      Serial.println("Heater in water have state: " + String(Heat0State));
      Serial.println("Heat cable have state: " + String(Heat1State));



      DEBUG_TimeS = TimeS;
      Serial.println();
      Serial.println("-------------------End serial info--------------------------");
      Serial.println("------------------------------------------------------------");
    }


  #endif
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

void PrintSensorAdress (byte  SensorName[8]){
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

void RellayInvertSwitch(int Pin, bool State, String Description){
  digitalWrite(Pin, !State);
  #ifdef DEBUG
    Serial.println("Switch rellay " + Description + "(" + String(Pin) + ")"+ " to state " + String(State));
  #endif
}

void Heat(){
  if(T0Temp != 0){
    if((T0Temp <= TargetTemp) && (T1Temp < Heat1SafeTemp) && (Heat1State != 1)){
      Heat1State = 1;
      RellayInvertSwitch(Heat1, Heat1State, "Heat1 - cable heater");
      #ifdef DEBUG
        Serial.println("Heat cable is on.");
      #endif
    }
    else if((T0Temp >= (TargetTemp + DeltaT)) && (Heat1State == 1)){
      Heat1State = 0;
      RellayInvertSwitch(Heat1, Heat1State, "Heat1 - cable heater");
      #ifdef DEBUG
        Serial.println("Heat cable is off. Standart turn off.");
      #endif
    }

    if ((T1Temp >= Heat1SafeTemp) && (Heat1State == 1)){
      Heat1State = 0;
      RellayInvertSwitch(Heat1, Heat1State, "Heat1 - cable heater");
      #ifdef DEBUG
        Serial.println("Heat cable is off. Dangerous temperature reached.");
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

  if ((T0Temp <= (TargetTemp - DeltaT)) && (Heat0State != 1)){
    Heat0State = 1;
    RellayInvertSwitch(Heat0, Heat0State, "Heat0 - water heater");
    #ifdef DEBUG
      Serial.println("Heater in water is on.");
    #endif
  }
  else if((T0Temp >= TargetTemp) && (Heat0State == 1)){
    Heat0State = 0;
    RellayInvertSwitch(Heat0, Heat0State, "Heat0 - water heater");
    #ifdef DEBUG
      Serial.println("Heater in water is off.");
    #endif
  }
}
