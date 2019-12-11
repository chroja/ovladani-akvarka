// Displej dotykový 240x320px

// připojení knihoven
#include "SPI.h"
#include "Adafruit_ILI9340.h"
#include "XPT2046_Touchscreen.h"

// nastavení propojovacích pinů
#define LCD_cs 10
#define LCD_dc 8//9
#define LCD_rst 9//18
#define DOTYK_cs  7

// inicializace LCD displeje z knihovny
Adafruit_ILI9340 displej = Adafruit_ILI9340(LCD_cs, LCD_dc, LCD_rst);
// inicializace řadiče dotykové vrstvy z knihovny
XPT2046_Touchscreen dotyk(DOTYK_cs);

void setup() {
  // zahájení komunikace s displejem a dotykovou vrstvou
  displej.begin();
  dotyk.begin();
  // pro otočení displeje stačí změnit číslo
  // v závorce v rozmezí 0-3
  displej.setRotation(0);
  // vyplnění displeje černou barvou
  displej.fillScreen(ILI9340_BLACK);
  // nastavení kurzoru na souřadnice x, y
  displej.setCursor(0, 0);
  displej.setTextColor(ILI9340_WHITE);
  // velikost textu lze nastavit změnou čísla v závorce
  displej.setTextSize(1);
  // funkce pro výpis textu na displej,
  // print tiskne na řádek stále za sebou,
  // println na konci textu přeskočí na nový řádek
  displej.println("Dotykovy displej");
  displej.setTextSize(3);
  displej.println("2.4 TFT LCD");
  displej.setTextSize(2);
  displej.println("navody.");
  displej.println("arduino-shop.cz");
}

void loop() {
  // pokud je detekován dotyk na displeji,
  // proveď následující
  if (dotyk.touched()) {
    // načti do proměnné bod souřadnice dotyku
    TS_Point bod = dotyk.getPoint();
    // funkce pro vykreslení plného obdélníku,
    // zadání (výchozí bod x, výchozí bod y,
    // velikost hrany na ose x, velikost hrany na ose y,
    // barva obdélníku) 
    displej.fillRect(115, 100, 100, 50, ILI9340_BLACK);
    // funkce pro vykreslení obrysu obdélníku,
    // zadání (výchozí bod x, výchozí bod y,
    // velikost hrany na ose x, velikost hrany na ose y,
    // barva obsysu obdélníku)
    displej.drawRect(20, 200, 100, 100, ILI9340_RED);
    // funkce pro vykreslení vodorovné čáry,
    // zadání (výchozí bod x, výchozí bod y,
    // délka čáry, barva čáry)
    displej.drawFastHLine(10, 90, 220, ILI9340_BLUE);
    // funkce pro vykreslení svislé čáry,
    // zadání (výchozí bod x, výchozí bod y,
    // délka čáry, barva čáry)
    displej.drawFastVLine(110, 90, 100, ILI9340_GREEN);
    displej.setCursor(0, 100);
    displej.setTextColor(ILI9340_WHITE);
    displej.setTextSize(2);
    // vypsání informací o souřadnicích posledního dotyku
    // včetně tlaku dotyku - tlak není přiliš přesný
    displej.print("   Tlak = ");
    displej.print(bod.z);
    displej.println(",");
    displej.print("Sour. x = ");
    displej.print(bod.x);
    displej.println(",");
    displej.print("Sour. y = ");
    displej.print(bod.y);
    displej.println();
    // pokud je detekován stisk uvnitř červeného obdélníku,
    // vypiš do obdélníku text Stisk!,
    // jinak vyplň obdélník černou barvou
    if (bod.x > 580 & bod.x < 1660 & bod.y > 700 & bod.y < 2000 ) {
      displej.setCursor(35, 240);
      displej.print("Stisk!");
    } else {
      displej.fillRect(35, 210, 80, 80, ILI9340_BLACK);
    }
  }
  delay(1);

}
