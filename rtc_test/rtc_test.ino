// RTC Hodiny reálného času DS3231

// připojení potřebných knihoven
#include <Wire.h>
#include <DS3231.h>
// inicializace RTC z knihovny
DS3231 rtc;
// vytvoření proměnné pro práci s časem
RTCDateTime datumCas;

// připojení knihovny
#include <LiquidCrystal.h>
// inicializace LCD displeje
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// nastavení čísla propojovacího pinu
// pro osvětlení LCD displeje
#define lcdSvit 10
unsigned long currentMillis, svitMillis;
int RunMill, RunSec, RunMin, RunHr, RunDay;
unsigned long RunX = 123456789;

void setup() {
  // zahájení komunikace po sériové lince
  // rychlostí 9600 baud
  Serial.begin(9600);
  // zahájení komunikace s RTC obvodem
  rtc.begin();
  // nastavení času v RTC podle času kompilace programu,
  // stačí nahrát jednou
  //rtc.setDateTime(__DATE__, __TIME__);
  // přímé nastavení času pro RTC
  //rtc.setDateTime(__DATE__, "12:34:56");
    lcd.begin(16, 2);
  // nastavení pinu pro řízení osvětlení jako výstupu
  pinMode(lcdSvit, OUTPUT);
  digitalWrite(lcdSvit,HIGH);
}
void loop() {
  currentMillis = millis();
   int analogData = analogRead(0);
  // načtení času z RTC do proměnné
  datumCas = rtc.getDateTime();
  // postupný tisk informací po sériové lince
  Serial.print("Datum a cas DS3231: ");
  char TimeChar[50];
  char TimeCharDate[50];
  char TimeCharTime[50];


  sprintf(TimeChar, "%04d/%02d/%02d %02d:%02d:%02d", datumCas.year, datumCas.month, datumCas.day, datumCas.hour, datumCas.minute, datumCas.second);
  sprintf(TimeCharDate, "%04d/%02d/%02d ", datumCas.year, datumCas.month, datumCas.day);
  sprintf(TimeCharTime, "%02d:%02d:%02d", datumCas.hour, datumCas.minute, datumCas.second);
  Serial.println("Time: " + String(TimeChar));
  lcd.setCursor(0, 0);
  lcd.print(TimeCharTime);
  lcd.setCursor(16-7, 0);
  lcd.print("RTC5.23");
  if((analogData > 600) && (analogData < 750)){
      digitalWrite(lcdSvit,HIGH);
      svitMillis = currentMillis;
    }
  
  if ((lcdSvit != LOW) && (currentMillis >= (svitMillis+15000))){
      digitalWrite(lcdSvit,LOW);
    }

  RunX = currentMillis/1000;
  Serial.print("RunX: ");
  Serial.println(RunX);
  
  RunSec = RunX % 60;
  Serial.print("RunSec: ");
  Serial.println(RunSec);
  RunX = RunX/60;
  RunMin = RunX % 60;
  Serial.print("RunMin: ");
  Serial.println(RunMin);
  RunX = RunX/60;
  RunHr = RunX % 60;
  Serial.print("RunHr: ");
  Serial.println(RunHr);
  RunX = RunX/60;
  RunDay = RunX % 24;
  Serial.print("RunDay: ");
  Serial.println(RunDay);

  char RunTimeChar[99];
  sprintf(RunTimeChar, "%03d:%02d:%02d:%02d", RunDay, RunHr, RunMin, RunSec);
  Serial.println(RunTimeChar);
  lcd.setCursor(0, 1);
  lcd.print(RunTimeChar); 
  lcd.setCursor(16-3, 1);
  lcd.print("RUN");

  /*
  Serial.print(datumCas.year);   Serial.print("-");
  Serial.print(datumCas.month);  Serial.print("-");
  Serial.print(datumCas.day);    Serial.print(" ");
  Serial.print(datumCas.hour);   Serial.print(":");
  Serial.print(datumCas.minute); Serial.print(":");
  Serial.print(datumCas.second); Serial.println("");
  */
  // pauza na konci smyčky loop
  //delay(500);
}
