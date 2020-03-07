
//------------------------------------------------------------------
// inicializace zakladnich knihoven
#include <OneWire.h> // knihovna pro komunikaci 1-wire
#include <EEPROM.h> // knihovna pro zapis do pameti eeprom
#include <SPI.h> // knihovna pro komunikacni rozhrani SP
#include <TFT_ILI9341.h> // knihovna displeje
#include <UTouch.h> // knihovna dotykové části
//------------------------------------------------------------------
// nastaveni nazvu pinu
//#define sclk 13  // Don't change, this is the hardware SPI SCLK line
//#define miso 12  // Don't change, this is the hardware SPI MISO line
//#define mosi 11  // Don't change, this is the hardware SPI MOSI line
#define TFT_CS    10   // Chip select for TFT display, don't change when using F_AS_T
#define TFT_DC     9   // Data/command line, don't change when using F_AS_T
//#define TFT_RST  8   // Reset, you could connect this to the Arduino reset pin
#define TOUCH_CLK  7   // Touch SCLK
#define TOUCH_CS   6   // Touch chip select
#define TOUCH_DIN  5   // Touch MOSI
#define TOUCH_DOUT 4   // Touch MISO
#define DS_18B20   3 // vstup pro cidla DS 18B20
#define TOUCH_IRQ  2   // Touch IRQ
//------------------------------------------------------------------
// pocet cidel
#define numberOfSensors   8 // maximalni pocet cidel DS 18B20
//------------------------------------------------------------------
// Nastaveni komunikace cidel DS 18B20
OneWire ds18B20(DS_18B20); // nastaveni komunikace cidel po sbernici oneWire
//------------------------------------------------------------------
// promenne teploty
float teplota[numberOfSensors];
unsigned long casCteniTeplot; // cas posledniho cteni
boolean povolCteniTeplot = false;
byte ID_teploty; // ID pro aktualizaci teploty v pripade nastaveni teplot
unsigned long prodlevaCteniTeplot = 5000; // nastaveni periody cteni teploty
byte tempResolution; // pomocna promenna pro upravu rozliseni cidla
//------------------------------------------------------------------
// pole adres cidel DS 18B20 - 8 pozic dalsi volna adresa 624
//const int cidlaDS_i[] = {560, 568, 576, 584, 592, 600, 608, 616};
const int resolutionAdd = 559; // adresa EEPROM nastaveni rozliseni cidla
const int cidlaDS_i = 560; // prvni adresa EEPROM pro zapis adres cidel
//------------------------------------------------------------------
// Promenne a nastaveni vstupu displej
TFT_ILI9341 tft = TFT_ILI9341(TFT_CS, TFT_DC);
// UTouch(byte clk, cs, din, dout, irq)
UTouch  touch(TOUCH_CLK, TOUCH_CS, TOUCH_DIN, TOUCH_DOUT, TOUCH_IRQ);
//------------------------------------------------------------------
// Promenne displej
unsigned long casProdlevyDotyk; // cas posledniho dotyku
const byte prodlevaDotyk = 200; // nastaveni prodlevy mezi dotykem
boolean vypniDotyk = true; // povoleni
byte TFT_rotation = 2; // nastaveni otoceni displeje
byte TOUCH_rotation; // nastaveni orientace dotykove plochy
int x; // radek x displej
int y; // radek y displej
byte stranka = 0; // cisla zobrazenych stranek
byte IDmistnosti; // pomocna hodnota pro prirazeni cidla mistnosti
//------------------------------------------------------------------
// Vytvoreni promennych pro tlacitka
struct defineButton {
  int x;
  int y;
  int width;
  int height;
  int color;
  char* text;
  byte font;
};
//------------------------------------------------------------------
// Vytvoreni jednotlivych tlacitek
defineButton buttonNastaveni = {60, 260, 120, 50, TFT_LIGHTGREY, "NASTAVENI", 2};
defineButton buttonZpet = {60, 260, 120, 50, TFT_LIGHTGREY, "ZPET", 2};
defineButton buttonCidlo = {40, 40, 160, 50, TFT_LIGHTGREY, "VYBER CIDLA", 2};
defineButton buttonSmazat = {40, 110, 160, 50, TFT_LIGHTGREY, "SMAZAT CIDLA", 2};
defineButton buttonRozliseni = {40, 180, 160, 50, TFT_LIGHTGREY, "NASTAVENI ROZLISENI", 2};
defineButton buttonStorno = {130, 260, 100, 50, TFT_LIGHTGREY, "STORNO", 2};
defineButton buttonUlozit = {10, 260, 100, 50, TFT_LIGHTGREY, "ULOZIT", 2};
defineButton button1 = {0, 40, 240, 27};
defineButton button2 = {0, 67, 240, 27};
defineButton button3 = {0, 94, 240, 27};
defineButton button4 = {0, 121, 240, 27};
defineButton button5 = {0, 148, 240, 27};
defineButton button6 = {0, 175, 240, 27};
defineButton button7 = {0, 202, 240, 27};
defineButton button8 = {0, 229, 240, 27};
defineButton buttonUP = {10, 100, 60, 60, TFT_GREEN};
defineButton buttonDOWN = {170, 100, 60, 60, TFT_GREEN};
//------------------------------------------------------------------
void setup() {
  tft.init(); // inicializace TFT displeje
  tft.setRotation(TFT_rotation); // nastaveni orientace displeje
  tft.setTextColor(TFT_BLACK); // nastaveni barvy pisma
  //------------------------------------------------------------------
  if (TFT_rotation == 0 || TFT_rotation == 2) TOUCH_rotation = PORTRAIT; // nastaveni otoceni dotykove plochy
  else if (TFT_rotation == 1 || TFT_rotation == 3) TOUCH_rotation = LANDSCAPE; // nastaveni otoceni dotykove plochy
  touch.InitTouch(TOUCH_rotation); // inicializuje dotykovou plochu
  touch.setPrecision(PREC_MEDIUM); // nastavi citlivost dotykove plochy
  //------------------------------------------------------------------
  // pokud neni zadano nastav rozliseni
  if (EEPROM.read(resolutionAdd) < 9 && EEPROM.read(resolutionAdd) > 12) EEPROM.write(resolutionAdd, 9);
    //------------------------------------------------------------------
  // zobrazeni uvodni stranky
  stranka0();
}
//------------------------------------------------------------------
void loop() {
  //------------------------------------------------------------------
  // cteni dotykoveho displeje
  if (touch.dataAvailable()) { // kontrola prijatych dat z dotykoveho panelu
    dotyk(); // spust program dotyk
  }
  //------------------------------------------------------------------
  // cast programu pro cteni teplot
  if (!povolCteniTeplot) { // kdyz je zakazano cteni teplot
    if ((unsigned long)(millis() - casCteniTeplot) >= prodlevaCteniTeplot) { // cekej na uplynuti prodlevz
      spustKonverzi();
    }
  }
  else { // jinak je povoleno cteni teplot
    if (ds18B20.read()) { // zkontroluj zda jsou data k dispozici
      cteniTeplot(); // precist hodnoty cidel teploty
      if (stranka == 0) aktualizujTeploty(); // aktualizuj uvodni stranku s teplotami
    }
  }
}
//------------------------------------------------------------------
void dotyk() {
  //------------------------------------------------------------------
  // omezeni opakovaneho stisku tlacitka
  if (!vypniDotyk) { // kdyz je dotyk vypnuty
    if ((unsigned long)(millis() - casProdlevyDotyk) >= prodlevaDotyk) { // pokud uplynula stanovena doba
      vypniDotyk = true; // zapni dotyk
    }
  }
  //------------------------------------------------------------------
  // cteni hodnot z dotykoveho displeje
  if (vypniDotyk) { // kdyz je dotyk zapnuty
    casProdlevyDotyk = millis(); // nastav novy cas pro vypocet prodlevy
    vypniDotyk = false; // vypni docasne dotyk
    touch.read(); // pokud jsou data precte je a ulozi do promenych
    x = touch.getX();
    y = touch.getY();
    //------------------------------------------------------------------
    // overeni stisku tlacitka a spusteni souvisejici akce
    if ((x != -1) && (y != -1)) { // a když není x a y menší než 0 ...
      if (stranka == 0) {  // kkyz je zobrazena stranka 0
        if (checkTouch(&buttonNastaveni)) {  // kdyz je stisknuto tlacitko nastaveni
          vyberNastaveni(); // nakresli stranku vyber nastaveni
        }
      }
      else if (stranka == 1) {  // kkyz je zobrazena stranka vyber nastaveni
        if (checkTouch(&buttonZpet)) {  // kdyz je stisknuto tlacitko zpet
          stranka0(); // nakresli stranku vyber nastaveni
        }
        else if (checkTouch(&buttonCidlo)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          vyberMistnosti(); // nakresli stranku vyber nastaveni
        }
        else if (checkTouch(&buttonSmazat)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          smazCidla(); // nakresli stranku vyber nastaveni
          spustKonverzi(); // nacti teploty
          povolCteniTeplot = true; // aktualizuj teploty
          stranka0(); // nakresli stranku vyber nastaveni
        }
        else if (checkTouch(&buttonRozliseni)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zmenRozliseni(); // nakresli stranku vyber nastaveni
        }
      }
      else if (stranka == 2) {  // kdyz je zobrazena stranka vyber mistnosti
        if (checkTouch(&buttonZpet)) {  // kdyz je stisknuto tlacitko zpet
          vyberNastaveni(); // nakresli stranku vyber nastaveni

        }
        else if (checkTouch(&button1)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 0; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button2)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 1; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button3)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 2; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button4)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 3; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button5)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 4; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button6)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 5; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button7)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 6; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
        else if (checkTouch(&button8)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          IDmistnosti = 7; // nastaveni ID mistnosti
          vyberCidla(); // nakresli stranku vyber cidla
        }
      }
      else if (stranka == 3) {  // kkyz je zobrazena stranka zmena rozliseni
        if (checkTouch(&buttonStorno)) {  // kdyz je stisknuto tlacitko zpet
          vyberNastaveni(); // nakresli stranku vyber nastaveni
        }
        else if (checkTouch(&buttonUlozit)) {  // kdyz je stisknuto tlacitko ulozit
          EEPROM.write(resolutionAdd, tempResolution); // uloz rozliseni do pameti
          spustKonverzi(); // nacti teploty
          povolCteniTeplot = true; // aktualizuj teploty
          vyberNastaveni(); // nakresli stranku vyber nastaveni
        }
        else if (checkTouch(&buttonUP)) {  // kdyz je stisknuto tlacitko nahoru
          tempResolution++;
          tempResolution = min(tempResolution, 12);
          rozliseni();
        }
        else if (checkTouch(&buttonDOWN)) {  // kdyz je stisknuto tlacitko dolu
          tempResolution--;
          tempResolution = max(tempResolution, 9);
          rozliseni();
        }
      }
      else if (stranka == 4) {  // kkyz je zobrazena stranka vyber cidla
        if (checkTouch(&buttonZpet)) {  // kdyz je stisknuto tlacitko zpet
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button1)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(0); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button2)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(1); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button3)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(2); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button4)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(3); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button5)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(4); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button6)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(5); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button7)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(6); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
        else if (checkTouch(&button8)) {  // kdyz je stisknuto tlacitko prirazeni cidla
          zapisAdresDS18B20(7); // prirad cidlo k mistnosti
          vyberMistnosti(); // nakresli stranku vyber mistnosti
        }
      }
    }
  }
}
//------------------------------------------------------------------
// kontrola zda bylo stisknuto tlacitko
boolean checkTouch (struct defineButton *b) {
  boolean press = false; // hodnota pro potvrzeni stisku
  //------------------------------------------------------------------
  if ((x >= b->x && x <= (b->x + b->width)) && (y >= b->y && y <= (b->y + b->height))) {
    press = true; // nastav novou hodnotu
  }
  return press; // vrat hodnotu
}
//------------------------------------------------------------------
// uvodni stranka
void stranka0() {
  stranka = 0; // nastaveni cisla stranky
  tft.fillScreen(TFT_WHITE); // vycisti plochu
  tft.drawCentreString("TEPLOTY", 120, 10, 4); // zobrazeni textu
  vypisMistnosti(); // zobraz nazvy mistnosti
  drawButton(&buttonNastaveni); // nakresli tlacitko
  aktualizujTeploty(); // aktualizuj teploty
}
//------------------------------------------------------------------
// vypis mistnosti
void vypisMistnosti() {
  int yText = 40; // promenna pro umisteni textu
  int py = 27; // promenna pro odstup textu
  for (int i = 0; i < numberOfSensors; i++) { // vypsani texu
    char text1[20] = "Mistnost "; // vytvor promennou pro text
    char text2[10]; // priprav promennou pro text
    itoa(i + 1, text2, 10); // preved int na char
    strcat(text1, text2); // sluc texty
    if (stranka == 0) { // kdyz je zobrazena hlavni stranka
      tft.drawString(text1, 10, yText + (py * i), 4); // zobrazeni textu
      tft.drawRightString("C", 230, yText + (py * i), 4); // zobrazeni textu
    }
    else if (stranka == 2) { // kdyz je zobrazena stranka vyber mistnosti
      tft.drawCentreString(text1, 120, yText + (py * i), 4); // zobrazeni textu
    }
  }
}
//------------------------------------------------------------------
// aktualizace teplot
void aktualizujTeploty() {
  int yText = 40; // promenna pro umisteni textu
  int py = 27; // promenna pro odstup textu
  for (int i = 0; i < numberOfSensors; i++) { // vypsani texu
    tft.fillRect(140, yText + (py * i), 70, py, TFT_WHITE); // vycistime pole pro zapis teploty
    char text1[10]; // priprav promennou pro text
    if (teplota[i] == NULL) { // kdyz nenni nactena zadna teplota
      tft.drawRightString("NA", 200, yText + (py * i), 4); // zobrazeni textu
    }
    else {
      dtostrf(teplota[i], 7, 1, text1); // jinak preved float na text
      tft.drawRightString(text1, 200, yText + (py * i), 4); // zobrazeni textu
    }
  }
}
//------------------------------------------------------------------
// stranka pro vyber nastaveni
void vyberNastaveni() {
  stranka = 1; // nastaveni cisla stranky
  tft.fillScreen(TFT_WHITE); // vycisti plochu
  tft.drawCentreString("NASTAVENI", 120, 10, 4); // zobrazeni textu
  drawButton(&buttonCidlo); // nakresli tlacitko
  drawButton(&buttonSmazat); // nakresli tlacitko
  drawButton(&buttonRozliseni); // nakresli tlacitko
  drawButton(&buttonZpet); // nakresli tlacitko
}
//------------------------------------------------------------------
// stranka pro vyber mistnosti
void vyberMistnosti() {
  stranka = 2; // nastaveni cisla stranky
  tft.fillScreen(TFT_WHITE); // vycisti plochu
  tft.drawCentreString("VYBER MISTNOSTI", 120, 10, 4); // zobrazeni textu
  vypisMistnosti(); // zobraz nazvy mistnosti
  drawButton(&buttonZpet); // nakresli tlacitko
}
//------------------------------------------------------------------
// stranka pro zmenu rozliseni
void zmenRozliseni() {
  int xb;
  int yb;
  int wy;
  byte bw = 16;
  stranka = 3; // nastaveni cisla stranky
  tft.fillScreen(TFT_WHITE); // vycisti plochu
  tft.drawCentreString("ZMENA ROZLISENI", 120, 10, 4); // zobrazeni textu
  // kresleni sipek
  drawButton(&buttonUP); // nakresli tlacitko
  drawButton(&buttonDOWN); // nakresli tlacitko
  // nastaveni promennych
  xb = buttonUP.x;
  yb = buttonUP.y + 2;
  wy = buttonUP.width;
  // kresleni obrysu sipky
  tft.fillRect(xb + (wy / 2) - (bw / 2) - 2, yb + 20 - 2, bw + 4, 30 + 4, TFT_WHITE);
  tft.fillTriangle(xb + (wy / 2), yb + 5 - 3, xb + 10 - 4, yb + 30 + 2, xb + wy - 10 + 4, yb + 30 + 2, TFT_WHITE);
  // kresleni  sipky
  tft.fillRect(xb + (wy / 2) - (bw / 2), yb + 20, bw, 30, TFT_DARKGREY);
  tft.fillTriangle(xb + (wy / 2), yb + 5, xb + 10, yb + 30, xb + wy - 10, yb + 30, TFT_DARKGREY);
  // nastaveni promennych
  xb = buttonDOWN.x;
  yb = buttonDOWN.y - 2;
  wy = buttonDOWN.width;
  // kresleni obrysu sipky
  tft.fillRect(xb + (wy / 2) - (bw / 2) - 2, yb + 10 - 2, bw + 4, 30 + 4, TFT_WHITE);
  tft.fillTriangle(xb + (wy / 2), yb + wy - 5 + 3, xb + 10 - 4, yb + wy - 30 - 2, xb + wy - 10 + 4, yb + wy - 30 - 2, TFT_WHITE);
  // kresleni  sipky
  tft.fillRect(xb + (wy / 2) - (bw / 2), yb + 10, bw, 30, TFT_DARKGREY);
  tft.fillTriangle(xb + (wy / 2), yb + wy - 5, xb + 10, yb + wy - 30, xb + wy - 10, yb + wy - 30, TFT_DARKGREY);
  // kresleni zbytku tlacitek
  drawButton(&buttonStorno); // nakresli tlacitko
  drawButton(&buttonUlozit); // nakresli tlacitko
  //kresleni rozliseni cidel
  tempResolution = EEPROM.read(resolutionAdd); // naplneni pomocne promenne
  rozliseni(); // zobraz rozliseni
}
//------------------------------------------------------------------
// kresleni rozliseni cidla
void rozliseni() {
  char text[10]; // priprav promennou pro text
  itoa(tempResolution, text, 10); // preved int na char
  tft.fillRect(buttonUP.x + 80, buttonUP.y, buttonUP.width, buttonUP.width, TFT_WHITE); // vycisti plochu pro kresleni
  tft.drawCentreString(text, 120, buttonUP.y + (buttonUP.width / 2) - 10, 4); // zobrazeni textu
}
//------------------------------------------------------------------
// stranka pro vyber cidla
void vyberCidla() { // stranka pro vyber cidla
  stranka = 4; // nastaveni cisla stranky
  tft.fillScreen(TFT_WHITE); // vycisti plochu
  tft.drawCentreString("VYBER CIDLA", 120, 10, 4); // zobrazeni textu
  drawButton(&buttonZpet); // nakresli tlacitko
  hledaniDS18B20(); // vyhledej a zobraz pripojena cidla
}
//------------------------------------------------------------------
// precte teploty ulozenych cidel
void cteniTeplot() {
  byte data[12]; // lokalni prommena
  byte address[8]; // lokalni prommena
  byte count; // lokalni prommena
  byte resolutionDS;; // // lokalni prommena
  uint16_t tmpResolution; // lokalni prommena
  uint16_t temp; // lokalni prommena
  // napln promenne
  resolutionDS = EEPROM.read(resolutionAdd); // nastav rozliseni cidla
  tmpResolution = (resolutionDS - 9) * 32; // nastav promennou
  if (resolutionDS == 12) temp = 127; // 12 bit res, 750 ms
  else if (resolutionDS == 11) temp = 95; // 11 bit res, 375 ms
  else if (resolutionDS == 10) temp = 63; // 10 bit res, 187.5 ms
  else temp = 31; // 9 bit res, 93.75 ms
  // vypni Onewire (pro parazitni napajieni
  ds18B20.depower();
  // precti teploty a uloz je do promennych
  for (int id = 0; id < numberOfSensors; id++) { // nastav odresu EEPROM cidla
    count = 0; // resetuj count
    for (int i = cidlaDS_i + (id * 8); i < cidlaDS_i + (id * 8) + 8; i++) { // precti adresy
      address[count] = EEPROM.read(i); // precti adresy
      count++; // zvys count o 1
    }
    ds18B20.reset(); // resetuj nastaveni oneWire
    ds18B20.select(address); // vyber adresu aktualniho cidla
    ds18B20.write(0xBE); // cti zasobnik cidla
    for (int i = 0; i < 9; i++) { // precti prvnich 9 bitu, bez CRC kodu
      data[i] = ds18B20.read(); // napln promennou
    }
    if (address[0] == 255 || data[0] == 255) teplota[id] = NULL; // kdyz neni cidlo ulozeno, nebo nejsou data k dispozici
    else { // jinak
      if (data[4] != temp) { // kdyz neni nastaveno pozadovane rozliseni
        ds18B20.reset();
        ds18B20.select(address);
        ds18B20.write(0x4E);         // zapis do zasobniku
        ds18B20.write(0x00);            // TL
        ds18B20.write(0x00);            // TH
        ds18B20.write(tmpResolution);         // Nastav nove rozliseni
        ds18B20.write(0x48);         // kopiruj zasobnik
        ds18B20.reset();
      }
      int16_t raw = (data[1] << 8) | data[0];
      teplota[id] = (float)raw / 16;
    }
  }
  povolCteniTeplot = false;
}
//------------------------------------------------------------------
// spusti teplotni konverzi cidel
void spustKonverzi() {
  casCteniTeplot = millis(); // nastav novy cas cteni
  ds18B20.reset(); // resetuj nastaveni oneWire
  ds18B20.write(0xCC); // nastav konverzi pro vsechny cidla
  ds18B20.write(0x44); // start interni konverze teploty
  povolCteniTeplot = true;
}
//------------------------------------------------------------------
// hleda adresy teplotnich cidel
void hledaniDS18B20() { // hleda adresy teplotnich cidel DS 18B20
  byte address[8];
  byte id;
  char text[40]; // pomocna promenna pro zobrazeni textu
  int yText = 40; // promenna pro umisteni textu
  int py = 27; // promenna pro odstup textu
  ds18B20.reset_search(); // resetuj hledani
  if (ds18B20.search(address)) { // hledej adresy
    for (id = 0; id < numberOfSensors; id++) { // zapis data do displeje
      String stringText;
      for (byte i = 0; i < 8; i++) { // precti a zobraz adresy
        stringText += address[i], DEC;
        if (i < 8 - 1) stringText += ",";
      }
      stringText.toCharArray(text, stringText.length() + 2);
      tft.drawString(text, 10, yText + (py * id), 2); // zobrazeni textu
      if (!ds18B20.search(address)) {
        ds18B20.reset_search();
        break;
      }
    }
  }
}
//------------------------------------------------------------------
// zapise vybranou adresu do EEPROM
void zapisAdresDS18B20(byte count) {
  byte address[8];
  if (ds18B20.search(address)) { // ksyz je adresa k dispozici
    for (byte id = 0; id < 8; id++) { // projdi data do count
      if (id == count) { // kdyz je id adresy rovno stisknutemu tlacitku
        for (byte i = 0; i < 8; i++) { // zapis adresy
          EEPROM.write(cidlaDS_i + (IDmistnosti * 8) + i, address[i]); // uloz hodnotu do EEPROM
        }
        break; // ukonci hledani
      }
      if (!ds18B20.search(address)) { // kdyz nejsou k dispozici zadne adresy
        ds18B20.reset_search(); // resetuj hledani
        break; // ukonci program
      }
    }
  }
}
//------------------------------------------------------------------
// funkce pro smazani adres cidel
void smazCidla() { // smazat vsechna cidla z pameti EEPROM
  for (byte id = 0; id < (numberOfSensors * 8) + 8; id++) { // projdi vsechny adresydata do count
    EEPROM.write(cidlaDS_i + id, 255); // uloz hodnotu do EEPROM
  }
}
//------------------------------------------------------------------
// funkce kresleni tlacitka
void drawButton(struct defineButton *b) {
  int xText; // pozice textu
  int yText; // pozice textu
  //------------------------------------------------------------------
  tft.fillRoundRect(b->x - 1, b->y - 1, b->width + 2, b->height + 2, 10, TFT_BLACK); // zobrazeni obrysu
  tft.fillRoundRect(b->x, b->y, b->width, b->height, 10, b->color); // zobrazeni tlacitka
  //------------------------------------------------------------------
  xText = b->x + (b->width/2); // urceni pozice x pro zobrazeni textu
  //------------------------------------------------------------------
  if (b->font == 4 ) yText = (b->y + (b->height / 2)) - 10; // urceni pozice y pro font 4
  else if (b->font == 2 ) yText = (b->y + (b->height / 2)) - 8; // urceni pozice y pro font 2
  else yText = (b->y + (b->height / 2)) - 3; // urceni pozice y pro font 1
  //------------------------------------------------------------------
  tft.drawCentreString(b->text, xText, yText, b->font); // zobrazeni textu
}
