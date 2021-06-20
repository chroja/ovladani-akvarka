/*
--------------------------NOTES---------------------------------
Use LIB
LED DRIVER                                    https://github.com/MrKrabat/LED-Strip-Driver-Module for RGB Strip
RTClib                                        https://navody.arduino-shop.cz/docs/texty/0/7/rtclib.zip
DS18B20                                       https://navody.arduino-shop.cz/docs/texty/0/14/dallastemperature.zip
OneWire                                       https://navody.arduino-shop.cz/docs/texty/0/15/onewire.zip
DS3231                                        https://navody.dratek.cz/docs/texty/0/243/arduino_ds3231_master.zip

pumpa 520ms/ml

COM13 - meduino
com12 - mega akvarko

int numRows = sizeof(LightCurve)/sizeof(LightCurve[0]);
int numCols = sizeof(LightCurve[0])/sizeof(LightCurve[0][0]);




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
#define TEMP_OFFSET
#define RESTART
//#define CUSTOM_BOARD
//#define MESAURE_LED_TEMP
//#define LIGHT_CURVE_TEST

bool SET_RTC_SETUP = false;
bool MESAURE_LIGHT_TEMP = false;
bool SEARCH_ADDRESS_DS18B20 = true;
bool TEST_RGB = false;
bool USE_GAMMA_RGB = false;
bool USE_GAMMA_WHITE = true;
bool GET_TEMP = true;
bool FLOODING_PUMP = false;
bool RANDOM_TEMP = false;



bool DEBUG_RELAY = false;
bool DEBUG_INIT_PIN = true;
bool DEBUG_GET_TIME = true;
bool DEBUG_SERIAL_INFO_SETUP = true;
bool DEBUG_SERIAL_INFO = true;
bool DEBUG_RESTART = true;
bool DEBUG_GET_TEMP = true;
bool DEBUG_DISCOVER_ONE_WIRE_DEVICES = false;
bool DEBUG_HEAT = true;
bool DEBUG_I2C_SCANNER = true;
bool DEBUG_OLED_DRAW_PAGES = true;
bool DEBUG_LIGHT_MODE = true;
bool DEBUG_LIGHT_BTN_READ = true;
bool DEBUG_PREPARE_SHOW_LIGHT = true;
bool DEBUG_SHOW_LIGHT = true;
bool DEBUG_TEST_RGB = true;
bool DEBUG_FERTILIZATION = true;
bool DEBUG_FLOODING_PUMP = true;






//water sensor
#ifdef CUSTOM_BOARD
    uint8_t WaterSensorAddress[8] = {0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10}; //water sensor used on desk
    uint8_t T1SensorAddress[8] = {0x28, 0x0A, 0x23, 0x79, 0xA2, 0x19, 0x03, 0x59}; //led sensor used on desk
#else
/*
    uint8_t WaterSensorAddress[8] = {0x28, 0xA5, 0x6A, 0x56, 0xB5, 0x01, 0x3C, 0x84}; //water sensor used in aquarium
    uint8_t T1SensorAddress[8] = {0x28, 0x77, 0x2B, 0x56, 0xB5, 0x01, 0x3C, 0x0A}; //water sensor used in aquarium
    uint8_t T2SensorAddress[8] = {0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10}; //water sensor used in aquarium
    */
    uint8_t WaterSensorAddress[][8] = {
        {0x28, 0xA5, 0x6A, 0x56, 0xB5, 0x01, 0x3C, 0x84}, //water sensor used in aquarium
        {0x28, 0x77, 0x2B, 0x56, 0xB5, 0x01, 0x3C, 0x0A}, //water sensor used in aquarium
        {0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10} //water sensor used in aquarium
        };
#endif

int gamma[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
    37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
    51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
    69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
    90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
    115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

#ifdef LIGHT_CURVE_TEST
    long LightCurve[][5] = {
        // {target time, target red, target green, target blue, target white} - first time must bee 0, last time must bee 86 399 (sec), color 0,0%-100,0% (0-1000)
        {0, 0, 0, 0, 0},                //00:00
        {((6*3600)+(17*60)), 0, 0, 0, 0},
        {((6*3600)+(30*60)), 800, 600, 150, 300},
        {((6*3600)+(40*60)), 1000, 1000, 500, 1000},
        {43200, 1000, 1000, 500, 1000},
        {44100, 300, 300, 300, 300},
        {53100, 300, 300, 300, 300},
        {54000, 1000, 1000, 500, 1000},
        {70200, 1000, 1000, 500, 1000},
        {72000, 1000, 500, 200, 300}, 
        {77400, 0, 0, 0, 0},
        {86399, 0, 0, 0, 0}
    };
#else
    /*
    long LightCurve[][5] = {
        // {target time, target red, target green, target blue, target white} - first time must bee 0, last time must bee 86 399 (sec), color 0,0%-100,0% (0-1000)
        {0, 0, 0, 0, 0},                //00:00
        {36000, 0, 0, 0, 0},            //10:00
        {39600, 800, 600, 150, 300},    //11:00
        {41400, 1000, 1000, 500, 700}, //11:30
        {43200, 1000, 1000, 500, 700}, //12:00
        {44100, 300, 300, 300, 300},    //12:15
        {53100, 300, 300, 300, 300},    //14:45
        {54000, 1000, 1000, 500, 700}, //15:00
        {70200, 1000, 1000, 500, 700}, //19:30
        {72000, 1000, 500, 200, 300},   //20:00 
        {77400, 0, 0, 0, 0},            //21:30 
        {86399, 0, 0, 0, 0}             //23:59:59
    };
    */
   
    long LightCurve[][5] = {
        // {target time, target red, target green, target blue, target white} - first time must bee 0, last time must bee 86 399 (sec), color 0,0%-100,0% (0-1000)
        {0, 0, 0, 0, 0},                //00:00
        {39600, 0, 0, 0, 0},            //11:00
        {45900, 300, 300, 300, 300},    //12:45
        {50400, 950, 950, 475, 950},    //13:00
        {70200, 950, 950, 475, 950},    //19:30
        {72000, 950, 500, 200, 300},    //20:00 
        {75600, 0, 0, 0, 0},            //21:00 
        {86399, 0, 0, 0, 0}             //23:59:59
    };
    
#endif

int NumRows;
int IndexRow;
int CurrentRow;
int TargetRow;

 long FertilizationMap[][4] = {
    // {time, pump, dose (ml), dose into water (l)} - 36000, 0, 4, 100 - 10:00, pump 0, 4ml per 100 l
    {33000, 0, 7, 100},
    {33600, 1, 7, 100}
};

unsigned int AquariumVolume = 300; //(l)

//int Fertilization
int FloodingVolume = 10;


U8GLIB_SH1106_128X64 Oled(0x3c);

#define RelayPin1 34
#define RelayPin2 36
#define RelayPin3 38
#define RelayPin4 40

#ifdef CUSTOM_BOARD
    #define TempPin 39
    #define LightBtnPin 2
    #define LightPumpPim 8
    const int FertilizationPumpDef[][3] = {
        //{pin, calibration value (ml), calibration time (sec)} 32, 500, 285 - pin, calibration 500ml per 285 sec
        {32, 500, 285},
        {33, 500, 225}
    };
#else
    #define TempPin 43
    #define LightBtnPin 2
    #define LightPumpPim 30
    const int FertilizationPumpDef[][3] = {
        //{pin, calibration value (ml), calibration time (sec)} 32, 500, 285 - pin, calibration 500ml per 285 sec
        {31, 500, 285},
        {33, 500, 225}
    };
#endif

#define NO 0
#define NC 1

#define ON 1
#define OFF 0

typedef struct
{
    int pin;
    int type;  // NO = 0, NC = 1
    int state; // ON = 1, OFF = 0
} rele_t;

rele_t CableHeat;
rele_t Heater;
rele_t Relay3;
rele_t Relay4;

//variales Light
#define LightWPin 7
#define RGBLightNum 4

#define RGBDataPin 3
#define RGBClockPin 4

int RedCurr = 0;       //0-100
int RedPrev = 0;       //0-100
int RedPwm = 0;        //0-255
int RedPwmMax = 255;//195;   //0-255

int GreenCurr = 0;
int GreenPrev = 0;
int GreenPwm = 0;
int GreenPwmMax = 255;//195;

int BlueCurr = 0;
int BluePrev = 0;
int BluePwm = 0;
int BluePwmMax = 255;//195;

int WhiteCurr = 0; 
int WhitePrev = 0;
int WhitePwm = 0;
int WhitePwmMax = 255;//205;

bool LightAuto = true;
int ModeLight = 1; // 0 = off; 1 = auto; 2 = on
int PrevModeLight = 0;
bool LightPumState = 0;
bool LightPumStatePrev = 0;

CRGB RBGLights[RGBLightNum];

//declarate variables
// var for date
int TimeY = 0;
int TimeMo = 0;
int TimeDay = 0;
int TimeD = 0;
int TimeH = 0;
int TimeM = 0;
int TimeS = 0;

unsigned long RtcCurrentMillis = 0;
unsigned long TimeStamp = 0;
unsigned long LenghtDay = 86399;

//temp
//DS tem sensors
float WaterTemp [] = {}; //temp on Water with calibration offset
float AvgWaterTemp = 0;
float T1Temp = 0; //temp on T1 with calibration offset
float WaterTempNoOffset [] = {};
float T1TempNoOffset = 0;
#ifdef TEMP_OFFSET
//float WaterOffset = 0;
float WaterOffset [] = {-0.7, -0.8, -1};
float T1Offset = 0;
#else
float WaterOffset [3] = {0, 0, 0};
float T1Offset = 0;
#endif



long NextReadTemp = 0;
int ErrorTempMax = 20;
int ErrorTempCurrent = 0;
int TempReadTime = 1;
int TempReadPeriod = 15000; //15 sec

//heating
float TargetTemp = 24;
float DeltaT = 0.5;
int Heat1SafeTemp = 28;
bool CableHeatState = 0;
bool HeaterState = 0;

float SafeLightTemp = 35;    //degrees
float MaximumLightTemp = 65; //degrees
int PrevLightWOn;

//time variable
unsigned long DEBUG_TimeStamp = 0;
bool FirstRun = true;
unsigned long OledRefresh = 0;
unsigned long OledPageShowTime = 3; //sec
unsigned long OledShowCurrentPage = 0;
byte CurrentPage, PrevPage = 0;

// add RTC instance

DS3231 rtc;
RTCDateTime DateTime;

bool LightBtnState = 0;
bool PrevLightBtnState = 0;
char LightBtnDir = 1;

String SerialCommand;

void RelayOn(rele_t vstup){
    // NO musime zapnout 1
    if(DEBUG_RELAY){
        Serial.println("\n\nRelayOn input data");
        Serial.print("input state ");
        Serial.println(vstup.state);
        Serial.print("input pin state ");
        Serial.println(digitalRead(vstup.pin));
    }
    
    if (vstup.type == NO){
        digitalWrite(vstup.pin, HIGH);
        if(DEBUG_RELAY){
            Serial.println("\nRelayOn NO");
            Serial.print("state ");
            Serial.println(vstup.state);
            Serial.print("pin state ");
            Serial.println(digitalRead(vstup.pin));
        }
    }
    // NC musime zapnout 0
    else{
        digitalWrite(vstup.pin, LOW);
        if(DEBUG_RELAY){
            Serial.println("\nRelayOn NC");
            Serial.print("state ");
            Serial.println(vstup.state);
            Serial.print("pin state ");
            Serial.println(digitalRead(vstup.pin));
        }
    }
    // nastaveni stavove promenne
    vstup.state = 1;
    if(DEBUG_RELAY){
        Serial.println("\nRelayOn output data");
        Serial.print("output state ");
        Serial.println(vstup.state);
        Serial.print("input pin state ");
        Serial.println(digitalRead(vstup.pin));
    }
    
}

void RelayOff(rele_t vstup){
    // NO musime vypnout 0
    if(DEBUG_RELAY){
        Serial.println("\n\nRelayOff input data ");
        Serial.print("input state ");
        Serial.println(vstup.state);
        Serial.print("input pin state ");
        Serial.println(digitalRead(vstup.pin));
    }
    if (vstup.type == NO){
        digitalWrite(vstup.pin, LOW);
        if(DEBUG_RELAY){
            Serial.println("\nRelayOff NO");
            Serial.print("state ");
            Serial.println(vstup.state);
            Serial.print("pin state ");
            Serial.println(digitalRead(vstup.pin));
        }
    }
    // NC musime vypnout 1
    else{
        digitalWrite(vstup.pin, HIGH);
        if(DEBUG_RELAY){
            Serial.println("\nRelayOff NC");
            Serial.print("state ");
            Serial.println(vstup.state);
            Serial.print("pin state ");
            Serial.println(digitalRead(vstup.pin));
        }
    }
    // nastaveni stavove promenne
    vstup.state = 0;
    if(DEBUG_RELAY){
        Serial.println("\nRelayOff output data");
        Serial.print("output state");
        Serial.println(vstup.state);
        Serial.print("input pin state ");
        Serial.println(digitalRead(vstup.pin));
    }
}

void RelaySwitch(rele_t vstup){
    // precti stav a nastav opacny
    if (vstup.state == 1){
        RelayOff(vstup);
    }
    else{
        RelayOn(vstup);
    }
}

//add ds instance
OneWire oneWireDS(TempPin);
DallasTemperature SensorsDS(&oneWireDS);

// days
char DayOfTheWeek[7][8] = {"nedele", "pondeli", "utery", "streda", "ctvrtek", "patek", "sobota"};

//****************************************** SETUP ****************************************

void setup(){

    wdt_enable(WDTO_2S);

    // serial comunication via USB
    Serial.begin(115200);
    Serial.println("------Start setup-----");

    rtc.begin();
    SetRTCSetup();

    FastLED.addLeds<P9813, RGBDataPin, RGBClockPin, RGB>(RBGLights, RGBLightNum); // BGR ordering is typical
    for (int i = 0; i < RGBLightNum; i++){
        RBGLights[i] = CRGB(0, 0, 0);
    }
    FastLED.show();

    initPin(LightPumpPim, 0);
    initPin(LightWPin, 0);
    for(int i = 0; i < (sizeof(FertilizationPumpDef)/sizeof(FertilizationPumpDef[0])); i++){
        initPin(FertilizationPumpDef[i][0], 0);
    }

    pinMode(LightBtnPin, INPUT);
    LightBtnState = digitalRead(LightBtnPin);

    //relaay declaration
    CableHeat.pin = RelayPin1;
    CableHeat.type = NC;
    pinMode(CableHeat.pin, OUTPUT);
    RelayOff(CableHeat);

    Heater.pin = RelayPin2;
    Heater.type = NC;
    pinMode(Heater.pin, OUTPUT);
    RelayOff(Heater);

    Relay3.pin = RelayPin3;
    Relay3.type = NC;
    pinMode(Relay3.pin, OUTPUT);
    RelayOff(Relay3);

    Relay4.pin = RelayPin4;
    Relay4.type = NC;
    pinMode(Relay4.pin, OUTPUT);
    RelayOff(Relay4);

    Wire.begin();

    while (!Serial); // Leonardo: wait for serial monitor
    Serial.println("\nI2C Scanner");
    I2CScanner();
    GetTime();
    DiscoverOneWireDevices();
    SensorsDS.begin();
    SerialInfoSetup();
    TestRGB();

    Serial.print("\nRANDOM_TEMP: ");     Serial.println(RANDOM_TEMP);
    Serial.println("------End setup-----");
    wdt_reset();
    /*
    delay(1000);
    wdt_reset();
    delay(1000);
    wdt_reset();
    delay(1000);
    */
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//****************************************** LOOP ****************************************

void loop(){
    wdt_reset(); // make sure this gets called at least once every 2 seconds!
    if(FirstRun){
        FirstRunFunc();
    }

    if (millis() >= (RtcCurrentMillis + 1000)){
        GetTime();
        RtcCurrentMillis = millis();
    }
    
    LightMode();
    LightBtnRead();
    PrepareShowLight();

    GetTemp();
    //InitRelay();
    Heat();
    FirstRun = false;
    TimeRestart();
    ShowOled();
    Fertilization();
    SerialInfo();
    GetSerial();

   
}

//****************************************** FUNCTION ****************************************
void initPin(int Pin, bool State){
    pinMode(Pin, OUTPUT);
    digitalWrite(Pin, State);
    if(DEBUG_INIT_PIN){
        Serial.print("Init pin: ");     Serial.print(Pin);  Serial.print("\t\tto state: ");     Serial.print(State);    Serial.print("\n");
    }
}

void SetRTCSetup(){
    if (SET_RTC_SETUP){
        rtc.setDateTime(__DATE__, __TIME__);
        SET_RTC_SETUP = false;
        Serial.println();
        Serial.println("---------- Time changed ----------");
    }
}

void GetTime(){
    DateTime = rtc.getDateTime();
    int NTimeY = DateTime.year;
    byte NTimeMo = DateTime.month;
    byte NTimeDay = DateTime.day;
    byte NTimeH = DateTime.hour;
    byte NTimeM = DateTime.minute;
    byte NTimeS = DateTime.second;
    if (((NTimeMo > 0) && (NTimeMo < 13)) && ((NTimeH >= 0) && (NTimeH < 24)) && ((NTimeM >= 0) && (NTimeM < 60))){
        TimeY = NTimeY;
        TimeMo = NTimeMo;
        TimeDay = NTimeDay;
        TimeH = NTimeH;
        TimeM = NTimeM;
        TimeS = NTimeS;
        TimeStamp = (long(TimeH) * 3600) + (long(TimeM) * 60) + long(TimeS);
        if(DEBUG_GET_TIME){
            Serial.println("Correct time read");    
        }
    }
    else{
        if(DEBUG_GET_TIME){
            Serial.println("err time read");
        }
    }
}


void SerialInfoSetup(){
    if(DEBUG_SERIAL_INFO_SETUP){
        Serial.println();
        Serial.println("---------------------SETUP INFO------------------------");
        Serial.println("Version: " + String(__DATE__));
        Serial.println("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
        Serial.println("---------------------END SETUP INFO------------------------");
    }
}

void SerialInfo(){
    if(DEBUG_SERIAL_INFO){
        if (DEBUG_TimeStamp != TimeStamp){
            Serial.println();
            Serial.println("------------------------------------------------------------");
            Serial.println("-------------------Start serial info------------------------");
            Serial.print("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
            Serial.print("\nTime Stamp (sec): " + String(TimeStamp));
            Serial.print("\nWater Temp (water): " + String(AvgWaterTemp));  
            Serial.print("\nHeat cable status: " + String(CableHeatState)); Serial.print(digitalRead(RelayPin1));         Serial.print("\t\tHeater  status: " + String(HeaterState)); Serial.print(digitalRead(RelayPin2));       
            Serial.print("\nindex current row: ");    Serial.print(CurrentRow);     Serial.print("\tindex target row: ");    Serial.print(TargetRow);
            Serial.print("\nUse gama for RGB: " + String(USE_GAMMA_RGB) + "\t\tUse gamma for white: " + String(USE_GAMMA_WHITE));
            Serial.print("\nRed: \t");      Serial.print(map(RedPwm, 0, RedPwmMax, 0, 100));        Serial.print(" % \tPWM: ");     Serial.print(RedPwm);       Serial.print(" \tGAMA PWM: ");  Serial.print(gamma[RedPwm]);
            Serial.print("\nGreen: \t");    Serial.print(map(GreenPwm, 0, GreenPwmMax, 0, 100));    Serial.print(" % \tPWM: ");     Serial.print(GreenPwm);     Serial.print(" \tGAMA PWM: ");  Serial.print(gamma[GreenPwm]);
            Serial.print("\nBlue: \t");     Serial.print(map(BluePwm, 0, BluePwmMax, 0, 100));      Serial.print(" % \tPWM: ");     Serial.print(BluePwm);      Serial.print(" \tGAMA PWM: ");  Serial.print(gamma[BluePwm]);
            Serial.print("\nWhite: \t");    Serial.print(map(WhitePwm, 0, WhitePwmMax, 0, 100));    Serial.print(" % \tPWM: ");     Serial.print(WhitePwm);     Serial.print(" \tGAMA PWM: ");  Serial.print(gamma[WhitePwm]);
            DEBUG_TimeStamp = TimeStamp;
            Serial.print("\nGet temp: ");     Serial.println(GET_TEMP);
            Serial.print("\nRANDOM_TEMP: ");     Serial.println(RANDOM_TEMP);
            Serial.println();
            Serial.println("-------------------End serial info--------------------------");
            Serial.println("------------------------------------------------------------");
        }
    }
}

void Restart(String Message, int Value){
    if(DEBUG_RESTART){
        Serial.println();
        Serial.print("Why restart device: ");   Serial.print(Message);  Serial.print(" Value: ");   Serial.println(Value);
        Serial.println("--------------------------------------------------------------------------------------");
        Serial.println("----------------------------------------RESTART DEVICE--------------------------------");
        Serial.println("--------------------------------------------------------------------------------------");
        Serial.println();   Serial.println();
        delay(500);
    }
    resetFunc(); //call reset
}

void TimeRestart(){
    if ((TimeDay == 1) && (TimeH == 0) && (TimeM == 0) && (RtcCurrentMillis >= 120000)){
        Restart("weekly restart", TimeStamp);
    }
}

void GetTemp(){
    if(GET_TEMP){
        if (NextReadTemp <= millis()){
            ShowLight(); //pokud dojde k chybě rozsvícení ledek, tak při měření teploty se opraví
            if(RANDOM_TEMP != 1){
                if(DEBUG_GET_TEMP){
                    Serial.println("******** Start measure temp *******\n");
                }
                SensorsDS.requestTemperatures();
                int NumWaterSensor = (sizeof(WaterSensorAddress)/sizeof(WaterSensorAddress[0]));
                
                for (byte i = 0; i < NumWaterSensor; i++){
                    WaterTempNoOffset[i] = ReadTemperature(WaterSensorAddress[i]);
                    Serial.println("Water temp without offset is: " + String(WaterTempNoOffset [i]) + "°C. On sensor: " + i);
                    if (( WaterTempNoOffset[i] > -127) && ( WaterTempNoOffset[i] < 85)){
                        ErrorTempCurrent = 0;
                        WaterTemp[i] = WaterTempNoOffset[i] + WaterOffset[i];
                        if(DEBUG_GET_TEMP){
                            Serial.println("\n--- Temp measure is ok. ---\n");
                            Serial.println("Water temp with offset on sensor: " + String(i) + " is " + String(WaterTemp[i]) + "°C\n\n");
                        }
                    }
                    
                    else if (ErrorTempCurrent < ErrorTempMax)        {
                        if(DEBUG_GET_TEMP){
                            Serial.println("Temp measure is FAIL.");
                            Serial.println("Error Temp counter is: " + String(ErrorTempCurrent + 1) + " / " + String(ErrorTempMax));
                        }
                        ErrorTempCurrent++;
                    }
                    else{
                        Restart("Lot of error measure. Total: ", ErrorTempCurrent);
                    }
                }

                AvgWaterTemp = 0;
                for (byte i = 0; i < NumWaterSensor; i++){
                    AvgWaterTemp = AvgWaterTemp + WaterTemp[i];
                    if(DEBUG_GET_TEMP){
                        //Serial.println(WaterTemp[i]);
                        Serial.println("\nAvg water temp is: " + String(AvgWaterTemp) + "°C run: " + i);
                    }
                }
                AvgWaterTemp = AvgWaterTemp / float(NumWaterSensor);
            }
            else{

                
                float min = - 100;
                float max = + 100;
                
                float rand = random(min, max);
                rand = rand/100;
                AvgWaterTemp = TargetTemp + rand;
                if(DEBUG_GET_TEMP){
                    Serial.println("randomtemp");
                }
            }
            NextReadTemp = millis() + TempReadPeriod;
            if(DEBUG_GET_TEMP){
                Serial.println("\nAvg water temp is: " + String(AvgWaterTemp) + "°C");
                //Serial.println(AvgWaterTemp);
                Serial.println("\n******* END measure temp **********");
                Serial.println();
                Serial.println();
            }
        } 
    }
}

void DiscoverOneWireDevices(void){
    if (SEARCH_ADDRESS_DS18B20){
        byte i;
        byte present = 0;
        byte data[12];
        byte addr[8];
        if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
            Serial.print("Looking for 1-Wire devices...\n\r");
        }
        while (oneWireDS.search(addr)){
            if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
                Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
            }
            for (i = 0; i < 8; i++){
                if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
                    Serial.print("0x");
                }
                if (addr[i] < 16){
                    if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
                        Serial.print('0');
                    }
                }
                if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
                    Serial.print(addr[i], HEX);
                }
                if (i < 7){
                    if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
                        Serial.print(", ");
                    }
                }
            }
            if (OneWire::crc8(addr, 7) != addr[7]){
                if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
                    Serial.print("CRC is not valid!\n");
                }
                return;
            }
        }
        if(DEBUG_DISCOVER_ONE_WIRE_DEVICES){
            Serial.print("\n\r\n\rThat's it.\r\n");
        }
        oneWireDS.reset_search();
        delay(500);
        return;
    }
}

float ReadTemperature(DeviceAddress deviceAddress){
    float tempC = SensorsDS.getTempC(deviceAddress);
    return tempC;
}

void Heat(){
    if (AvgWaterTemp != 0){
        if ((AvgWaterTemp <= TargetTemp) && (CableHeatState != 1)){
            CableHeatState = 1;
            RelayOn(CableHeat);
            if(DEBUG_HEAT){
                Serial.println("Heat cable is on.");
            }
        }
        else if ((AvgWaterTemp >= (TargetTemp + DeltaT)) && (CableHeatState == 1)){
            CableHeatState = 0;
            RelayOff(CableHeat);
            if(DEBUG_HEAT){
                Serial.println("Heat cable is off. Standart turn off.");
            }
        }
        if ((AvgWaterTemp <= (TargetTemp - DeltaT)) && (HeaterState != 1)){
            HeaterState = 1;
            RelayOn(Heater);
            if(DEBUG_HEAT){
                Serial.println("Heater in water is on.");
            }
        }
        else if ((AvgWaterTemp >= TargetTemp) && (HeaterState == 1)){
            HeaterState = 0;
            RelayOff(Heater);
            if(DEBUG_HEAT){
                Serial.println("Heater in water is off.");
            }
        }
    }
    else    {
        if (FirstRun){
            if(DEBUG_HEAT){
                Serial.println("Temp isn´t read.");
            }
        }
    }
}

void I2CScanner(){
    byte error, address;
    int nDevices;
    if(DEBUG_I2C_SCANNER){
        Serial.println("Scanning...");
    }
    nDevices = 0;
    for (address = 1; address < 127; address++){
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0){
            if(DEBUG_I2C_SCANNER){
                Serial.print("I2C device found at address 0x");
            }
            if (address < 16){
                if(DEBUG_I2C_SCANNER){
                    Serial.print("0");
                }
            }
            if(DEBUG_I2C_SCANNER){ 
                Serial.print(address, HEX);
                Serial.println("  !");
            }

            nDevices++;
        }
        else if (error == 4){
            if(DEBUG_I2C_SCANNER){
                Serial.print("Unknown error at address 0x");
            }
            if (address < 16){
                if(DEBUG_I2C_SCANNER){
                    Serial.print("0");
                }
            }
            if(DEBUG_I2C_SCANNER){
                Serial.println(address, HEX);
            }
        }
    }
    if (nDevices == 0){
        if(DEBUG_I2C_SCANNER){
            Serial.println("No I2C devices found\n");
        }
    }
    else{
        if(DEBUG_I2C_SCANNER){
            Serial.println("done\n");
        }
    }
}
void OledDrawPages(){
    if(((OledShowCurrentPage + OledPageShowTime) > LenghtDay) && (TimeStamp < 15)){
        OledShowCurrentPage = TimeStamp;
    }
    if (TimeStamp >= (OledShowCurrentPage + OledPageShowTime)){
        CurrentPage++;
        if(DEBUG_OLED_DRAW_PAGES){
            Serial.print("Oled - print next page. Print page no. "); Serial.println(CurrentPage);
        }
        OledShowCurrentPage = TimeStamp;
    }

    switch (CurrentPage){
    case 0:
        OledTimePage();
        break;
    case 1:
        OledTempPage();
        break;
    case 2:
        OledLightModePage();
        break;
    case 3:
        OledLightColorPage();
        break;
    default:
        CurrentPage = 0;
        if(DEBUG_OLED_DRAW_PAGES){
            Serial.println("Oled - maximum count for pages overflow. Now set current page to 0.");
        }
        break;
    }
}

void OledLeftText(unsigned char Row){
    Oled.setFont(u8g_font_unifont);
    Oled.setPrintPos(0, (Row * 15) + 10);
}

void ShowOled(){
    if (millis() - OledRefresh > 100){
        Oled.firstPage();
        do{
            OledDrawPages();
        } 
        while (Oled.nextPage());
        OledRefresh = millis();
    }
}

void OledTimePage(){
    OledLeftText(0);
    Oled.print("Time: ");
    Oled.print(TimeH);
    Oled.print(":");
    Oled.print(TimeM);
    Oled.print(":");
    Oled.print(TimeS);
    OledLeftText(1);
    Oled.print("Stamp: ");
    Oled.print(TimeStamp);
    OledLeftText(2);
    Oled.print("Run: ");
    Oled.print((millis() / 1000));
    Oled.print("s");
    OledLeftText(3);
    Oled.print("Run: ");
    Oled.print((millis() / (3600000)));
    Oled.print(":");
    Oled.print((millis() / (60000)) % 60);
    Oled.print(":");
    Oled.print((millis() / 1000) % 60);
    Oled.print(":");
    Oled.print((millis()) % 1000);
}

void OledTempPage(){
    OledLeftText(0);
    Oled.print("Avg Water: ");
    Oled.print(AvgWaterTemp);
    Oled.print(" C");
    /*
    OledLeftText(1);
    Oled.print("Led: ");
    Oled.print(T1Temp);
    Oled.print(" C");
    */
    OledLeftText(1);
    Oled.print("Heat cable: ");
    if (CableHeatState == 0){
        Oled.print("OFF");
    }
    else if (CableHeatState == 1){
        Oled.print("ON");
    }
    else{
        Oled.print("ERR");
    }

    OledLeftText(2);
    Oled.print("Heater: ");
    if (HeaterState == 0){
        Oled.print("OFF");
    }
    else if (HeaterState == 1){
        Oled.print("ON");
    }
    else{
        Oled.print("ERR");
    }
}

void OledLightModePage(){
    OledLeftText(0);
    Oled.print("Led mode: ");
    if (ModeLight == 0){
        Oled.print("Off");
    }
    else if (ModeLight == 1){
        Oled.print("Auto");
    }
    else if (ModeLight == 2){
        Oled.print("On");
    }
    else{
        Oled.print("ERR");
    }
}

void OledLightColorPage(){
    OledLeftText(0);
    Oled.print("RED: ");    Oled.print(float(RedCurr)/10);    Oled.print("%");

    OledLeftText(1);
    Oled.print("GREEN: ");  Oled.print(float(GreenCurr)/10);  Oled.print("%");

    OledLeftText(2);
    Oled.print("BLUE: ");   Oled.print(float(BlueCurr)/10);   Oled.print("%");
    
    OledLeftText(3);
    Oled.print("White: ");  Oled.print(float(WhiteCurr)/10);  Oled.print("%");
}


void LightMode(){
    if (ModeLight == 0){
        if (PrevModeLight != 0){
            PrevModeLight = ModeLight;
            if(DEBUG_LIGHT_MODE){
                Serial.println("Light set to off mode.");
            }
            LightAuto = false;
            RedPwm = 0;
            GreenPwm = 0;
            BluePwm = 0;
            WhitePwm = 0;
            RedPrev = 0;
            GreenPrev = 0;
            BluePrev = 0;
            WhitePrev = 0;
            ShowLight();
        }
    }
    else if (ModeLight == 2){
        if (PrevModeLight != 2){
            PrevModeLight = ModeLight;
            if(DEBUG_LIGHT_MODE){
                Serial.println("Light set to on mode.");
            }
            LightAuto = false;
            RedPwm = RedPwmMax;
            GreenPwm = GreenPwmMax;
            BluePwm = BluePwmMax;
            WhitePwm = WhitePwmMax;
            RedPrev = 100;
            GreenPrev = 100;
            BluePrev = 100;
            WhitePrev = 100;
            ShowLight();
        }
    }
    else if (ModeLight == 1){
        if (PrevModeLight != 1){
            PrevModeLight = ModeLight;
            if(DEBUG_LIGHT_MODE){
                Serial.println("Light set to auto mode.");
            }
        }
        LightAuto = true;
    }
    else{
        if(DEBUG_LIGHT_MODE){
            Serial.println("invalid set Led mode");
        }
    }
}

void LightBtnRead (){
    LightBtnState = digitalRead(LightBtnPin);
    if ((digitalRead(LightBtnPin) == 1) && (PrevLightBtnState != 1)){
        delay(20);
        if ((digitalRead(LightBtnPin) == 1) && (PrevLightBtnState != 1)){
            if(FLOODING_PUMP){
                FloodingPump();
            }
            if (!FLOODING_PUMP){
                ModeLight = ModeLight + LightBtnDir;
                LightBtnState = digitalRead(LightBtnPin);
                if(DEBUG_LIGHT_BTN_READ){
                    Serial.println("func LightBtnRead -- ModeLed: " + String(ModeLight));
                }
                if((ModeLight == 0) || (ModeLight == 2)){
                    LightBtnDir = LightBtnDir * (-1);
                    if(LightBtnDir > 0){
                        if(DEBUG_LIGHT_BTN_READ){
                            Serial.println("func LightBtnRead -- next LightBtnDir: +");
                        }
                    }
                    if(LightBtnDir < 0){
                        if(DEBUG_LIGHT_BTN_READ){
                            Serial.println("func LightBtnRead -- next LightBtnDir: -");
                        }
                    }
                    
                }
            }
        }    
    }
    PrevLightBtnState = LightBtnState;
}

void PrepareShowLight (){
    if(!LightAuto){
        return;
    }
    NumRows = sizeof(LightCurve)/sizeof(LightCurve[0]);
    IndexRow = NumRows - 1;
    for(int i = 0; i < IndexRow; i++){
        if(TimeStamp >= LightCurve[i][0]){
            CurrentRow = i;
            TargetRow = CurrentRow + 1;
            
        }
    }

    RedCurr = map(TimeStamp, LightCurve[CurrentRow][0], LightCurve[TargetRow][0], LightCurve[CurrentRow][1], LightCurve[TargetRow][1]);
    RedPwm = map(RedCurr, 0, 1000, 0, RedPwmMax);

    GreenCurr = map(TimeStamp, LightCurve[CurrentRow][0], LightCurve[TargetRow][0], LightCurve[CurrentRow][2], LightCurve[TargetRow][2]);
    GreenPwm = map(GreenCurr, 0, 1000, 0, GreenPwmMax);

    BlueCurr = map(TimeStamp, LightCurve[CurrentRow][0], LightCurve[TargetRow][0], LightCurve[CurrentRow][3], LightCurve[TargetRow][3]);
    BluePwm = map(BlueCurr, 0, 1000, 0, BluePwmMax);

    WhiteCurr = map(TimeStamp, LightCurve[CurrentRow][0], LightCurve[TargetRow][0], LightCurve[CurrentRow][4], LightCurve[TargetRow][4]);
    WhitePwm = map(WhiteCurr, 0, 1000, 0, WhitePwmMax);

    if((RedPrev != RedCurr) || (GreenPrev != GreenCurr) || (BluePrev != BlueCurr) || (WhitePrev != WhiteCurr)){
        
        ShowLight();
        if(DEBUG_PREPARE_SHOW_LIGHT){
            Serial.print("\n********** Color Changed **********");
            Serial.print("\nRed   from: ");     Serial.print(float(RedPrev)/10);       Serial.print(" % \tto: ");   Serial.print(float(RedCurr)/10);      Serial.print(" % \tPWM: ");  Serial.print(RedPwm);    Serial.print(" \tGammma PWM: ");  Serial.print(gamma[RedPwm]);    
            Serial.print("\nGreen from: ");     Serial.print(float(GreenPrev)/10);     Serial.print(" % \tto: ");   Serial.print(float(GreenCurr)/10);    Serial.print(" % \tPWM: ");  Serial.print(GreenPwm);  Serial.print(" \tGammma PWM: ");  Serial.print(gamma[GreenPwm]);
            Serial.print("\nBlue  from: ");     Serial.print(float(BluePrev)/10);      Serial.print(" % \tto: ");   Serial.print(float(BlueCurr)/10);     Serial.print(" % \tPWM: ");  Serial.print(BluePwm);   Serial.print(" \tGammma PWM: ");  Serial.print(gamma[BluePwm]);
            Serial.print("\nWhite from: ");     Serial.print(float(WhitePrev)/10);     Serial.print(" % \tto: ");   Serial.print(float(WhiteCurr)/10);    Serial.print(" % \tPWM: ");  Serial.print(WhitePwm);  Serial.print(" \tGammma PWM: ");  Serial.print(gamma[WhitePwm]);
            Serial.print("\n********** Color Changed **********");
        }
        RedPrev = RedCurr;
        GreenPrev = GreenCurr;
        BluePrev = BlueCurr;
        WhitePrev = WhiteCurr;
    }
}


void ShowLight(){
    for (int i = 0; i < RGBLightNum; i++) {
        if(USE_GAMMA_RGB){
            RBGLights[i] = CRGB(gamma[RedPwm], gamma[GreenPwm], gamma[BluePwm]);
        }
        else{
            RBGLights[i] = CRGB(RedPwm, GreenPwm, BluePwm);
        }
    }
    FastLED.show();
    if(USE_GAMMA_WHITE){
        analogWrite(LightWPin, gamma[WhitePwm]);
    }
    else{
        analogWrite(LightWPin, WhitePwm);
    }
    if(DEBUG_SHOW_LIGHT){
        Serial.print("\nWrite color\n");
    }

    if((RedPwm > 0) || (GreenPwm > 0) || (BluePwm > 0) || (WhitePwm > 0)){
        LightPumState = 1;
        if(LightPumStatePrev != 1){
            digitalWrite(LightPumpPim, HIGH);
            LightPumStatePrev = LightPumState;
            if(DEBUG_SHOW_LIGHT){
                Serial.println("Light pump turned on");
            }
        }
    }
    else if((RedPwm == 0) || (GreenPwm == 0) || (BluePwm == 0) || (WhitePwm == 0)){
        LightPumState = 0;
        if(LightPumStatePrev != 0){
            digitalWrite(LightPumpPim, LOW);
            LightPumStatePrev = LightPumState;
            if(DEBUG_SHOW_LIGHT){
                Serial.println("Light pump turned off");
            }
        }
    }
}

void TestRGB(){
    if(TEST_RGB){
        if(DEBUG_TEST_RGB){
            Serial.println("testig rgb");
            Serial.println("r");
        }
        for (int i = 0; i < RGBLightNum; i++) {
            RBGLights[i] = CRGB(25, 0, 0);
            FastLED.show();
            wdt_reset();
            delay(1000/RGBLightNum);
        }
        for (int i = 0; i < RGBLightNum; i++) {
            RBGLights[i] = CRGB(0, 0, 0);
        }
        FastLED.show();
        if(DEBUG_TEST_RGB){
            Serial.println("g");
        }
        for (int i = 0; i < RGBLightNum; i++) {
            RBGLights[i] = CRGB(0, 25, 0);
            FastLED.show();
            wdt_reset();
            delay(1000/RGBLightNum);
        }
        for (int i = 0; i < RGBLightNum; i++) {
            RBGLights[i] = CRGB(0, 0, 0);
        }
        FastLED.show();
        if(DEBUG_TEST_RGB){
            Serial.println("b");
        }
        for (int i = 0; i < RGBLightNum; i++) {
            RBGLights[i] = CRGB(0, 0, 25);
            FastLED.show();
            wdt_reset();
            delay(1000/RGBLightNum);
        }
        for (int i = 0; i < RGBLightNum; i++) {
            RBGLights[i] = CRGB(0, 0, 0);
        }
        FastLED.show();
   }  
}

void Fertilization (){
    int FertilizationIndexRowMax = (sizeof(FertilizationMap)/sizeof(FertilizationMap[0]));
    for(int i = 0; i < FertilizationIndexRowMax; i++) {
        if(((TimeStamp+5) >= FertilizationMap[i][0]) && ((TimeStamp) <= FertilizationMap[i][0])){
            if(GET_TEMP){
                GET_TEMP = false;
            }
        }
        if(((TimeStamp+1) >= FertilizationMap[i][0]) && ((TimeStamp) <= FertilizationMap[i][0])){
            if(DEBUG_FERTILIZATION){
                Serial.print("\n\t\t-----Start fertilization-----");
            }
            unsigned long FertilizationStartMillis = millis();
            float FertilizationVolume = (AquariumVolume / FertilizationMap[i][3]) * FertilizationMap[i][2];
            unsigned int FertilizationCalibration =  float(FertilizationPumpDef[FertilizationMap[i][1]][2]) / float(FertilizationPumpDef[FertilizationMap[i][1]][1]) * float(1000);
            unsigned long FertilizationTime = FertilizationCalibration * FertilizationVolume;
            int PumpPin = FertilizationPumpDef[FertilizationMap[i][1]][0];
            if(DEBUG_FERTILIZATION){
                Serial.print("\nPump: \t\t\t\t\t");                     Serial.print(FertilizationMap[i][1]);            
                Serial.print("\nFertilization volume: \t\t");           Serial.print(FertilizationVolume);              Serial.print(" ml"); 
                Serial.print("\nFertilization Calibration: \t");        Serial.print(FertilizationCalibration);         Serial.print(" ms/ml");
                Serial.print("\nTotal time fertilization: \t");         Serial.print(float(FertilizationTime)/1000);    Serial.print(" s\n");
            }
            while (millis() < (FertilizationStartMillis + FertilizationTime)){
                //code tento while to nesmí opustit  dokud nebude pohnojeno
                digitalWrite(PumpPin, HIGH);
                if(DEBUG_FERTILIZATION){
                    Serial.print(".");
                }
                delay(100);
                wdt_reset(); 
            }

            digitalWrite(PumpPin, LOW);
            GET_TEMP = true;
            if(DEBUG_FERTILIZATION){
                Serial.print("\n\t\t-----End fertilization-----");
                Serial.print("\n");
            }
        }
    }
}

void FloodingPump(){
    int FertilizationIndexRowMax = (sizeof(FertilizationMap)/sizeof(FertilizationMap[0]));
    for(int i = 0; i < FertilizationIndexRowMax; i++) {
        if(GET_TEMP){
            GET_TEMP = false;
        }
        if(DEBUG_FLOODING_PUMP){
            Serial.print("\n\t\t-----Start flooding fertilization-----");
        }
        unsigned long FertilizationStartMillis = millis();
        unsigned int FertilizationCalibration =  float(FertilizationPumpDef[FertilizationMap[i][1]][2]) / float(FertilizationPumpDef[FertilizationMap[i][1]][1]) * float(1000);
        unsigned long FertilizationTime = FertilizationCalibration * FloodingVolume;
        int PumpPin = FertilizationPumpDef[FertilizationMap[i][1]][0];
        if(DEBUG_FLOODING_PUMP){
            Serial.print("\nPump: \t\t\t\t\t\t");                   Serial.print(FertilizationMap[i][1]);            
            Serial.print("\nFlooding volume: \t\t");                Serial.print(FloodingVolume);                   Serial.print(" ml"); 
            Serial.print("\nFertilization Calibration: \t");        Serial.print(FertilizationCalibration);         Serial.print(" ms/ml");
            Serial.print("\nTotal time fertilization: \t");         Serial.print(float(FertilizationTime)/1000);    Serial.print(" s\n");
        }
        while (millis() < (FertilizationStartMillis + FertilizationTime)){
            //code tento while to nesmí opustit  dokud nebude pohnojeno
            digitalWrite(PumpPin, HIGH);
            if(DEBUG_FLOODING_PUMP){
                Serial.print(".");
            }
            delay(100);
            wdt_reset(); 
        }

        digitalWrite(PumpPin, LOW);
        if(DEBUG_FLOODING_PUMP){
            Serial.print("\n\t\t-----End fertilization-----");
            Serial.print("\n");
        }
    }
    GET_TEMP = true;
    FLOODING_PUMP = false;
}

void FirstRunFunc(){
    if(FirstRun){
        Serial.println("*********FirstRunFunc*********\n");
        RelayOn(CableHeat);
        delay(50);
        wdt_reset();
        RelayOn(Heater);
        delay(50);
        wdt_reset();
        RelayOn(Relay3);
        delay(50);
        wdt_reset();
        RelayOn(Relay4);
        delay(50);
        wdt_reset();
        RelayOff(CableHeat);
        delay(50);
        wdt_reset();
        RelayOff(Heater);
        delay(50);
        wdt_reset();
        RelayOff(Relay3);
        delay(50);
        wdt_reset();
        RelayOff(Relay4);

        Serial.print("RANDOM_TEMP "); Serial.println(RANDOM_TEMP);
    


        Serial.println("*********FirstRunFunc*********");
    }
}

void GetSerial(){
    // read from port 1, send to port 0:
    if (Serial.available()) {
        char c = Serial.read(); {
        if (c ==  '\n') {
            ParseSerial(SerialCommand);
            SerialCommand = "";
        }
        else {
            SerialCommand += c;

        }
    }
  }
}

void ParseSerial(String com){
    String Variable; 
    String Value;
    Variable = com.substring(0, com.indexOf("="));
    Value = com.substring(com.indexOf("=") + 1);
    
    if(Variable == "SEARCH_ADDRESS_DS18B20"){
        Serial.println("Current value for " + Variable + " is: " + String(SEARCH_ADDRESS_DS18B20));
        SEARCH_ADDRESS_DS18B20 = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(SEARCH_ADDRESS_DS18B20));
    }
    else if(Variable == "TEST_RGB"){
        Serial.println("Current value for " + Variable + " is: " + String(TEST_RGB));
        TEST_RGB = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(TEST_RGB));
    }
    else if(Variable == "USE_GAMMA_RGB"){
        Serial.println("Current value for " + Variable + " is: " + String(USE_GAMMA_RGB));
        USE_GAMMA_RGB = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(USE_GAMMA_RGB));
    }
    else if(Variable == "USE_GAMMA_WHITE"){
        Serial.println("Current value for " + Variable + " is: " + String(USE_GAMMA_WHITE));
        USE_GAMMA_WHITE = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(USE_GAMMA_WHITE));
    }
    else if(Variable == "GET_TEMP"){
        Serial.println("Current value for " + Variable + " is: " + String(GET_TEMP));
        GET_TEMP = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(GET_TEMP));
    }
    else if(Variable == "FLOODING_PUMP"){
        Serial.println("Current value for " + Variable + " is: " + String(FLOODING_PUMP));
        FLOODING_PUMP = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(FLOODING_PUMP));
    }
    else if(Variable == "RANDOM_TEMP"){
        Serial.println("Current value for " + Variable + " is: " + String(RANDOM_TEMP));
        RANDOM_TEMP = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(RANDOM_TEMP));
    }/* *********** debugs *************
    else if(Variable == "DEBUG_RELAY"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_RELAY));
        DEBUG_RELAY = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_RELAY));
    }
    else if(Variable == "DEBUG_INIT_PIN"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_INIT_PIN));
        DEBUG_INIT_PIN = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_INIT_PIN));
    }
    else if(Variable == "DEBUG_GET_TIME"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_GET_TIME));
        DEBUG_GET_TIME = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_GET_TIME));
    }
    else if(Variable == "DEBUG_SERIAL_INFO_SETUP"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_SERIAL_INFO_SETUP));
        DEBUG_SERIAL_INFO_SETUP = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_SERIAL_INFO_SETUP));
    }
    else if(Variable == "DEBUG_SERIAL_INFO"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_SERIAL_INFO));
        DEBUG_SERIAL_INFO = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_SERIAL_INFO));
    }
    else if(Variable == "DEBUG_RESTART"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_RESTART));
        DEBUG_RESTART = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_RESTART));
    }
    else if(Variable == "DEBUG_GET_TEMP"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_GET_TEMP));
        DEBUG_GET_TEMP = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_GET_TEMP));
    }
    else if(Variable == "DEBUG_DISCOVER_ONE_WIRE_DEVICES"){
        Serial.println("Current value for " + Variable + " is: " + String(DEBUG_DISCOVER_ONE_WIRE_DEVICES));
        DEBUG_DISCOVER_ONE_WIRE_DEVICES = Value.toInt();
        Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_DISCOVER_ONE_WIRE_DEVICES));
    }
    else if(Variable == "DEBUG_HEAT"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_HEAT));
        DEBUG_HEAT = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_HEAT));
    }
    else if(Variable == "DEBUG_I2C_SCANNER"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_I2C_SCANNER));
        DEBUG_I2C_SCANNER = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_I2C_SCANNER));
    }
    else if(Variable == "DEBUG_OLED_DRAW_PAGES"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_OLED_DRAW_PAGES));
        DEBUG_OLED_DRAW_PAGES = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_OLED_DRAW_PAGES));
    }
    else if(Variable == "DEBUG_LIGHT_MODE"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_LIGHT_MODE));
        DEBUG_LIGHT_MODE = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_LIGHT_MODE));
    }
    else if(Variable == "DEBUG_LIGHT_BTN_READ"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_LIGHT_BTN_READ));
        DEBUG_LIGHT_BTN_READ = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_LIGHT_BTN_READ));
    }
    else if(Variable == "DEBUG_PREPARE_SHOW_LIGHT"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_PREPARE_SHOW_LIGHT));
        DEBUG_PREPARE_SHOW_LIGHT = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_PREPARE_SHOW_LIGHT));
    }
    else if(Variable == "DEBUG_SHOW_LIGHT"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_SHOW_LIGHT));
        DEBUG_SHOW_LIGHT = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_SHOW_LIGHT));
    }
    else if(Variable == "DEBUG_TEST_RGB"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_TEST_RGB));
        DEBUG_TEST_RGB = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_TEST_RGB));
    }
    else if(Variable == "DEBUG_FERTILIZATION"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_FERTILIZATION));
        DEBUG_FERTILIZATION = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_FERTILIZATION));
    }
    else if(Variable == "DEBUG_FLOODING_PUMP"){
            Serial.println("Current value for " + Variable + " is: " + String(DEBUG_FLOODING_PUMP));
        DEBUG_FLOODING_PUMP = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(DEBUG_FLOODING_PUMP));
    }*/




    else if(Variable == "FloodingVolume"){
            Serial.println("Current value for " + Variable + " is: " + String(FloodingVolume));
        FloodingVolume = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(FloodingVolume));
    }
    else if(Variable == "RedPwmMax"){
            Serial.println("Current value for " + Variable + " is: " + String(RedPwmMax));
        RedPwmMax = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(RedPwmMax));
    }
    else if(Variable == "GreenPwmMax"){
            Serial.println("Current value for " + Variable + " is: " + String(GreenPwmMax));
        GreenPwmMax = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(GreenPwmMax));
    }
    else if(Variable == "BluePwmMax"){
            Serial.println("Current value for " + Variable + " is: " + String(BluePwmMax));
        BluePwmMax = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(BluePwmMax));
    }
    else if(Variable == "WhitePwmMax"){
            Serial.println("Current value for " + Variable + " is: " + String(WhitePwmMax));
        WhitePwmMax = Value.toInt();
            Serial.println("Value for " + Variable + " changed to: " + String(WhitePwmMax));
    }
    else if(Variable == "ModeLight"){
            Serial.println("Current value for " + Variable + " is: " + String(ModeLight));
        ModeLight = Value.toInt();
        ModeLight == 1 ? LightAuto = true : LightAuto = false;
            Serial.println("Value for " + Variable + " changed to: " + String(ModeLight));
    }




    else{

    }

}

