/*
--------------------------NOTES---------------------------------
Use LIB
LED DRIVER                                    https://github.com/MrKrabat/LED-Strip-Driver-Module for RGB Strip
RTClib                                        https://navody.arduino-shop.cz/docs/texty/0/7/rtclib.zip
DS18B20                                       https://navody.arduino-shop.cz/docs/texty/0/14/dallastemperature.zip
OneWire                                       https://navody.arduino-shop.cz/docs/texty/0/15/onewire.zip

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
#define DEBUG
#define TEMP_OFFSET
#define RESTART
#define SERIAL_INFO
//#define CUSTOM_BOARD
//#define MESAURE_LED_TEMP
#define LIGHT_CURVE_TEST

bool SET_RTC = false;
bool MESAURE_LIGHT_TEMP = true;
bool SEARCH_ADDRESS_DS18B20 = true;
bool TestRGB_T = false;


//water sensor
#ifdef CUSTOM_BOARD
    uint8_t T0SensorAddress[8] = {0x28, 0x75, 0x3F, 0x79, 0xA2, 0x16, 0x03, 0xA0}; //water sensor used on desk
    uint8_t T1SensorAddress[8] = {0x28, 0x0A, 0x23, 0x79, 0xA2, 0x19, 0x03, 0x59}; //led sensor used on desk
#else
    uint8_t T1SensorAddress[8] = {0x28, 0x1E, 0x66, 0xDA, 0x1E, 0x19, 0x01, 0x7F}; //water sensor used in aquarium
    uint8_t T0SensorAddress[8] = {0x28, 0xC7, 0x25, 0x79, 0xA2, 0x19, 0x03, 0x10}; //water sensor used in aquarium
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
    long LightCurve[][5] = {
        // {target time, target red, target green, target blue, target white} - first time must bee 0, last time must bee 86 399 (sec), color 0,0%-100,0% (0-1000)
        {0, 0, 0, 0, 0},                //00:00
        {25200, 0, 0, 0, 0},            //7:00
        {28800, 800, 600, 150, 300},    //8:00
        {30600, 1000, 1000, 500, 1000}, //8:30
        {43200, 1000, 1000, 500, 1000}, //12:00
        {44100, 300, 300, 300, 300},    //12:15
        {53100, 300, 300, 300, 300},    //14:45
        {54000, 1000, 1000, 500, 1000}, //15:00
        {70200, 1000, 1000, 500, 1000}, //19:30
        {72000, 1000, 500, 200, 300},   //20:00 
        {77400, 0, 0, 0, 0},            //21:30 
        {86399, 0, 0, 0, 0}             //23:59:59
    };
#endif

int NumRows;
int IndexRow;
int CurrentRow;
int TargetRow;

U8GLIB_SH1106_128X64 Oled(0x3c);

#define RelayPin1 34
#define RelayPin2 36
#define RelayPin3 38
#define RelayPin4 40

#ifdef CUSTOM_BOARD
    #define TempPin 39
    #define LightBtnPin 2
#else
    #define TempPin 43
    #define LightBtnPin 6
#endif

#define NO 1
#define NC 0

#define ON HIGH
#define OFF LOW

typedef struct
{
    int pin;
    bool type;  // NO = 0, NC = 1
    bool state; // ON = 1, OFF = 0
} rele_t;

rele_t CableHeat;
rele_t Heater;
rele_t Relay3;
rele_t Relay4;

//variales Light
#define LightWPin 10
#define RGBLightNum 4
#define RGBDataPin 3
#define RGBClockPin 4

int RedCurr = 0;       //0-100
int RedPrev = 0;       //0-100
int RedPwm = 0;        //0-255
int RedPwmMax = 255;   //0-255

int GreenCurr = 0;
int GreenPrev = 0;
int GreenPwm = 0;
int GreenPwmMax = 255;

int BlueCurr = 0;
int BluePrev = 0;
int BluePwm = 0;
int BluePwmMax = 255;

int WhiteCurr = 0; 
int WhitePrev = 0;
int WhitePwm = 0;
int WhitePwmMax = 255;

bool LightAuto = true;
byte ModeLight = 1; // 0 = off; 1 = auto; 2 = off
byte PrevModeLight = 0;

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
/*
int NTimeY = 0;
int NTimeMo = 0;
int NTimeDay = 0;
int NTimeD = 0;
int NTimeH = 0;
int NTimeM = 0;
int NTimeS = 0;
*/
unsigned long RtcCurrentMillis = 0;
unsigned long TimeStamp = 0;
unsigned long LenghtDay = 86399;

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

long NextReadTemp = 0;
int ErrorTempMax = 10;
int ErrorTempCurrent = 0;
int TempReadTime = 1;
int TempReadPeriod = 15000; //15 sec
//heat
float TargetTemp = 25;
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
int OledPageShowTime = 3; //sec
unsigned long OledShowCurrentPage = 0;
byte CurrentPage, PrevPage = 0;

// add RTC instance

DS3231 rtc;
RTCDateTime DateTime;

bool LightBtnState = 0;
bool PrevLightBtnState = 0;
char LightBtnDir = 1;

void RelayOn(rele_t vstup){
    // NO musime zapnout 1
    if (vstup.type == NO){
        digitalWrite(vstup.pin, HIGH);
    }
    // NC musime zapnout 0
    else{
        digitalWrite(vstup.pin, LOW);
    }
    // nastaveni stavove promenne
    vstup.state = ON;
}

void RelayOff(rele_t vstup){
    // NO musime vypnout 0
    if (vstup.type == NO){
        digitalWrite(vstup.pin, LOW);
    }
    // NC musime vypnout 1
    else{
        digitalWrite(vstup.pin, HIGH);
    }
    // nastaveni stavove promenne
    vstup.state = OFF;
}

void RelaySwitch(rele_t vstup){
    // precti stav a nastav opacny
    if (vstup.state == ON){
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

    //wdt_enable(WDTO_2S);

    // serial comunication via USB
    Serial.begin(115200);
    Serial.println("------Start setup-----");

    rtc.begin();
    SetRTC();

    FastLED.addLeds<P9813, RGBDataPin, RGBClockPin, RGB>(RBGLights, RGBLightNum); // BGR ordering is typical
    for (int i = 0; i < RGBLightNum; i++){
        RBGLights[i] = CRGB(0, 0, 0);
    }
    FastLED.show();

    pinMode(LightWPin, OUTPUT);
    digitalWrite(LightWPin, 0);
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
    Relay3.type = NO;
    pinMode(Relay3.pin, OUTPUT);
    RelayOff(Relay3);

    Relay4.pin = RelayPin4;
    Relay4.type = NO;
    pinMode(Relay4.pin, OUTPUT);
    RelayOff(Relay4);

    Wire.begin();

   

    while (!Serial); // Leonardo: wait for serial monitor
    Serial.println("\nI2C Scanner");
    I2CScanner();
    GetTimeSetup();
    DiscoverOneWireDevices();
    SensorsDS.begin();
    SerialInfoSetup();
    delay(500);
    TestRGB();
    Serial.println("------End setup-----");
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//****************************************** LOOP ****************************************

void loop(){
    wdt_reset(); // make sure this gets called at least once every 8 seconds!

    if (millis() >= (RtcCurrentMillis + 1000)){
        GetTime();
        RtcCurrentMillis = millis();
    }

    LightBtnRead();
    PrepareShowLight();


    GetTemp();
    Heat();
    FirstRun = false;
    TimeRestart();
    ShowOled();
    #ifdef SERIAL_INFO
        SerialInfo();
    #endif
   
}

//****************************************** FUNCTION ****************************************
void initPin(int Pin){
    pinMode(Pin, OUTPUT);
    digitalWrite(Pin, LOW);
}

void SetRTC(){
    if (SET_RTC){
        rtc.setDateTime(__DATE__, __TIME__);
        SET_RTC = false;
        Serial.println();
        Serial.println("---------- Time changed ----------");
    }
}

void GetTimeSetup(){
    DateTime = rtc.getDateTime();
    TimeY = DateTime.year;
    TimeMo = DateTime.month;
    TimeDay = DateTime.day;
    TimeH = DateTime.hour;
    TimeM = DateTime.minute;
    TimeS = DateTime.second;
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
        #ifdef DEBUG
            Serial.println("Correct time read");
        #endif
    }
    else{
        #ifdef DEBUG
            Serial.println("err time read");
        #endif
    }
}


void SerialInfoSetup(){
    #ifdef DEBUG
        Serial.println();
        Serial.println("---------------------SETUP INFO------------------------");
        Serial.println("Version: " + String(__DATE__));
        Serial.println("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
        Serial.println("---------------------END SETUP INFO------------------------");
    #endif
}

void SerialInfo(){
    #ifdef DEBUG
        if (DEBUG_TimeStamp != TimeStamp){
            Serial.println();
            Serial.println("------------------------------------------------------------");
            Serial.println("-------------------Start serial info------------------------");
            Serial.print("Actual date and time " + String(TimeDay) + '/' + String(TimeMo) + '/' + String(TimeY) + ' ' + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS));
            Serial.print("\nTime Stamp (sec): " + String(TimeStamp));
            Serial.print("\nHeat cable status: " + String(CableHeatState));
            Serial.print("\t\tHeater  status: " + String(HeaterState));
            Serial.print("\nT0 Temp (water): " + String(T0Temp));
            Serial.print("\t\tT1 Temp (light): " + String(T1Temp));
            Serial.print("\nindex current row: ");    Serial.print(CurrentRow);
            Serial.print("\tindex target row: ");    Serial.print(TargetRow);
            Serial.print("\nRed: ");        Serial.print(map(RedPwm, 0, RedPwmMax, 0, 100));        Serial.print(" % \tPWM: ");     Serial.print(RedPwm);
            Serial.print("\t\tGreen: ");    Serial.print(map(GreenPwm, 0, GreenPwmMax, 0, 100));    Serial.print(" % \tPWM: ");     Serial.print(GreenPwm);
            Serial.print("\t\tBlue: ");     Serial.print(map(BluePwm, 0, BluePwmMax, 0, 100));      Serial.print(" % \tPWM: ");     Serial.print(BluePwm);
            Serial.print("\t\tWhite: ");    Serial.print(map(WhitePwm, 0, WhitePwmMax, 0, 100));    Serial.print(" % \tPWM: ");     Serial.print(WhitePwm);
            DEBUG_TimeStamp = TimeStamp;
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
        resetFunc(); //call reset
    #endif
}

void TimeRestart(){
    if ((TimeDay == 1) && (TimeH == 0) && (TimeM == 0) && (RtcCurrentMillis >= 120000)){
        Serial.println("resetting");
        resetFunc(); //call reset
    }
}

void GetTemp(){
    if (NextReadTemp <= millis()){
        ShowLight(); //pokud dojde k chybě rozsvícení ledek, tak při měření teploty se opraví
        #ifdef DEBUG
            Serial.println("******** Start measure temp *******\n");
        #endif
        SensorsDS.requestTemperatures();
        T0TempNoOffset = ReadTemperature(T0SensorAddress);
        #ifdef MESAURE_LIGHT_TEMP
            T1TempNoOffset = ReadTemperature(T1SensorAddress);
        #else
            T1TempNoOffset = T0TempNoOffset;
        #endif
        NextReadTemp = millis() + TempReadPeriod;
                #ifdef DEBUG
            Serial.println("Temp read.");
            Serial.println("T0 read temp is: " + String(T0TempNoOffset) + "°C");
            Serial.println("Temp read.");
            Serial.println("T1 read temp is: " + String(T1TempNoOffset) + "°C");
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
        else if (ErrorTempCurrent < ErrorTempMax)        {
            #ifdef DEBUG
                Serial.println("Temp measure is FAIL.");
                Serial.println("Error Temp counter is: " + String(ErrorTempCurrent + 1) + " / " + String(ErrorTempMax));
            #endif
            ErrorTempCurrent++;
        }
        else{
            Restart("Lot of error measure. Total: ", ErrorTempCurrent);
        }
        Serial.println("\n******* END measure temp **********");
        Serial.println();
        Serial.println();
    }
}

void DiscoverOneWireDevices(void){
    if (SEARCH_ADDRESS_DS18B20){
        byte i;
        byte present = 0;
        byte data[12];
        byte addr[8];

        Serial.print("Looking for 1-Wire devices...\n\r");
        while (oneWireDS.search(addr)){
            Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
            for (i = 0; i < 8; i++){
                Serial.print("0x");
                if (addr[i] < 16){
                    Serial.print('0');
                }
                Serial.print(addr[i], HEX);
                if (i < 7){
                    Serial.print(", ");
                }
            }
            if (OneWire::crc8(addr, 7) != addr[7]){
                Serial.print("CRC is not valid!\n");
                return;
            }
        }
        Serial.print("\n\r\n\rThat's it.\r\n");
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
    if (T0Temp != 0){
        if ((T0Temp <= TargetTemp) && (CableHeatState != 1)){
            CableHeatState = 1;
            RelayOn(CableHeat);
            #ifdef DEBUG
                Serial.println("Heat cable is on.");
            #endif
        }
        else if ((T0Temp >= (TargetTemp + DeltaT)) && (CableHeatState == 1)){
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
        else if ((T0Temp >= TargetTemp) && (HeaterState == 1)){
            HeaterState = 0;
            RelayOff(Heater);
            #ifdef DEBUG
                Serial.println("Heater in water is off.");
            #endif
        }
    }
    else    {
        if (FirstRun){
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
    for (address = 1; address < 127; address++){
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0){
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        }
        else if (error == 4){
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
}

void OledDrawPages(){
    if(TimeStamp > LenghtDay){
        OledShowCurrentPage = 0;
    }
    if (TimeStamp >= (OledShowCurrentPage + OledPageShowTime)){
        CurrentPage++;
        Serial.print("Oled - print next page. Print page no. "); Serial.println(CurrentPage);
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
        Serial.println("Oled - maximum count for pages overflow. Now set current page to 0.");
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
    Oled.print("Water: ");
    Oled.print(T0Temp);
    Oled.print(" C");
    OledLeftText(1);
    Oled.print("Led: ");
    Oled.print(T1Temp);
    Oled.print(" C");
    OledLeftText(2);
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

    OledLeftText(3);
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
            Serial.println("Light set to off mode.");
            RedPwm = 0;
            GreenPwm = 0;
            BluePwm = 0;
            WhitePwm = 0;
            ShowLight();
        }
    }
    else if (ModeLight == 2){
        if (PrevModeLight != 2){
            PrevModeLight = ModeLight;
            Serial.println("Light set to on mode.");
            RedPwm = RedPwmMax;
            GreenPwm = GreenPwmMax;
            BluePwm = BluePwmMax;
            WhitePwm = WhitePwmMax;
            ShowLight();
        }
    }
    else if (ModeLight == 1){
        if (PrevModeLight != 1){
            PrevModeLight = ModeLight;
            Serial.println("Light set to auto mode.");
        }
        LightAuto = true;
    }
    else{
        Serial.println("invalid set Led mode");
    }
}

void LightBtnRead (){
    LightBtnState = digitalRead(LightBtnPin);
    if ((digitalRead(LightBtnPin) == 1) && (PrevLightBtnState != 1)){
        delay(20);
        if ((digitalRead(LightBtnPin) == 1) && (PrevLightBtnState != 1)){
            ModeLight = ModeLight + LightBtnDir;
            LightBtnState = digitalRead(LightBtnPin);
            Serial.println(digitalRead(LightBtnPin));
            Serial.println("func LightBtnRead -- ModeLed: " + String(ModeLight));
            if((ModeLight == 0) || (ModeLight == 2)){
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
        Serial.print("\n********** Color Changed **********");
        Serial.print("\nRed   from: ");     Serial.print(float(RedPrev)/10);       Serial.print(" % \tto: ");   Serial.print(float(RedCurr)/10);      Serial.print(" % \tPWM: ");  Serial.print(RedPwm);    Serial.print(" \tGammma PWM: ");  Serial.print(gamma[RedPwm]);    
        Serial.print("\nGreen from: ");     Serial.print(float(GreenPrev)/10);     Serial.print(" % \tto: ");   Serial.print(float(GreenCurr)/10);    Serial.print(" % \tPWM: ");  Serial.print(GreenPwm);  Serial.print(" \tGammma PWM: ");  Serial.print(gamma[GreenPwm]);
        Serial.print("\nBlue  from: ");     Serial.print(float(BluePrev)/10);      Serial.print(" % \tto: ");   Serial.print(float(BlueCurr)/10);     Serial.print(" % \tPWM: ");  Serial.print(BluePwm);   Serial.print(" \tGammma PWM: ");  Serial.print(gamma[BluePwm]);
        Serial.print("\nWhite from: ");     Serial.print(float(WhitePrev)/10);     Serial.print(" % \tto: ");   Serial.print(float(WhiteCurr)/10);    Serial.print(" % \tPWM: ");  Serial.print(WhitePwm);  Serial.print(" \tGammma PWM: ");  Serial.print(gamma[WhitePwm]);
        Serial.print("\n********** Color Changed **********");
        RedPrev = RedCurr;
        GreenPrev = GreenCurr;
        BluePrev = BlueCurr;
        WhitePrev = WhiteCurr;
    }
}


void ShowLight(){
    
    for (int i = 0; i < RGBLightNum; i++) {
        RBGLights[i] = CRGB(gamma[RedPwm], gamma[GreenPwm], gamma[BluePwm]);
    }
    FastLED.show();
    analogWrite(LightWPin, gamma[WhitePwm]);
    Serial.print("\nWrite color");
    /*
    for (int i = 0; i < RGBLightNum; i++) {
        RBGLights[i] = CRGB(RedPwm, GreenPwm, BluePwm);
    }
    FastLED.show();
    analogWrite(LightWPin, WhitePwm);
    */
}

void TestRGB(){
    if(TestRGB_T){
        Serial.println("testig rgb");
        Serial.println("r");
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
        Serial.println("g");
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
        Serial.println("b");
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