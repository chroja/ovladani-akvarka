



#define NO 0
#define NC 1

#define ON  HIGH
#define OFF LOW
typedef struct {
  int  pin;
  bool typ;  // NO = 0, NC = 1
  bool stav; // ON = 1, OFF = 0
} rele_t;

rele_t rele1;
rele_t rele2;

void zapniRele(rele_t vstup) {
  // NO musime zapnout 1
  if (vstup.typ == NO) {
    digitalWrite(vstup.pin, HIGH);
  }
  // NC musime zapnout 0
  else {
    digitalWrite(vstup.pin, LOW);
  }
  // nastaveni stavove promenne
  vstup.stav = ON;
}

void vypniRele(rele_t vstup) {
  // NO musime vypnout 0
  if (vstup.typ == NO) {
    digitalWrite(vstup.pin, LOW);
  }
  // NC musime vypnout 1
  else {
    digitalWrite(vstup.pin, HIGH);
  }
  // nastaveni stavove promenne
  vstup.stav = OFF;
}

void prepniStavRele(rele_t vstup) {
  // precti stav a nastav opacny
  if (vstup.stav == ON) {
    vypniRele(vstup);
  }
  else {
    zapniRele(vstup);
  }
}

void setup() {
  // prirazeni cisla pinu a typu rele
  rele1.pin = 34;
  rele1.typ = NO;

  rele2.pin = 36;
  rele2.typ = NC;
  // nastaveni vystupu
  pinMode(rele1.pin, OUTPUT);
  pinMode(rele2.pin, OUTPUT);
}

void loop() {
  zapniRele(rele1);
  zapniRele(rele2);
  delay(1000);
  vypniRele(rele1);
  vypniRele(rele2);
  delay(1000);
}
