#define Rellay1 34
#define Rellay2 36
#define Rellay3 38
#define Rellay4 40
#define Rellay5 35
#define Rellay6 37
#define Rellay7 39
#define Rellay8 41

void setup(){
  Serial.begin(115200);
  initRellay(Rellay1);
  initRellay(Rellay2);
  initRellay(Rellay3);
  initRellay(Rellay4);
  initRellay(Rellay5);
  initRellay(Rellay6);
  initRellay(Rellay7);
  initRellay(Rellay8);
}
void loop (){
  testRellay(Rellay1, "Rellay1");
  testRellay(Rellay2, "Rellay2");
  testRellay(Rellay3, "Rellay3");
  testRellay(Rellay4, "Rellay4");
  testRellay(Rellay5, "Rellay5");
  testRellay(Rellay6, "Rellay6");
  testRellay(Rellay7, "Rellay7");
  testRellay(Rellay8, "Rellay8");
  Serial.println("end testing loop");
  delay(2000);


}

void initRellay(int Rellay){
  pinMode(Rellay, OUTPUT);
  digitalWrite(Rellay, LOW);
}

void testRellay(int rellay, String name){
  digitalWrite(rellay, !LOW);
  Serial.println("tested rellay: " + String(rellay) + " name: " + name);
  delay(100);
  digitalWrite(rellay, LOW);
  delay(1000);

}







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
  rele1.pin = 10;
  rele1.typ = NO;

  rele2.pin = 11;
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
