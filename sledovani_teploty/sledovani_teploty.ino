#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define OLED_RESET 5
Adafruit_SH1106 display(OLED_RESET);

int potPin = A6;
int potVal = 0;
int ohm = 10000;
int potOhm;
float desetinyStupne;

float teplotaC;
const int pinCidlaDS = 4;
OneWire oneWireDS(pinCidlaDS);
DallasTemperature senzoryDS(&oneWireDS);


void setup()   {
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);
  display.clearDisplay();
  display.display();/*
  display.drawPixel(10, 10, WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Hello word");
  display.display();
  delay(5000);*/
  pinMode(potPin, INPUT);
  senzoryDS.begin();



}

void loop(){
  potRead();

  getTemp();
  displayShow();
  delay(2000);

}


void potRead(){
  potVal = analogRead(potPin);
  potOhm = map(potVal, 0, 1023, 0, ohm);
  Serial.println("potVal: " + String(potVal) + " potOhm: " + String(potOhm));
}

void getTemp(){
senzoryDS.requestTemperatures();
Serial.print("Teplota cidla DS18B20: ");
teplotaC = senzoryDS.getTempCByIndex(0);
Serial.print(teplotaC);
Serial.println(" stupnu Celsia");


}


void displayShow(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(String(potOhm) + " Ohm");
  display.println();
  display.println();
  display.setTextSize(3);
  display.print(String(teplotaC) + " C");




  display.display();

}
