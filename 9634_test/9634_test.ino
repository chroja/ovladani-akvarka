#include <pca9634.h>


#include <Wire.h>

// I2C address 0x20: A0-A5->gnd; A6->Vcc
Pca9634 led( 0x02 );
//Pca9634 pump( 0x20 );
//Pca9634 ssr( 0x30 );

#define DEBUGPIN
//#define i2cscan


int speed = 250;
/*
unsigned long averageSpeed = 0;
unsigned long SumSpeed;
unsigned long countLoop = 0;
*/
void setup() {
  Wire.begin();
  Serial.begin(9600);

  led.begin();
  led.wakeup();
  /*
  pump.begin();
  pump.wakeup();
  ssr.begin();
  ssr.wakeup();
  */
}

void loop(){
  //speed = random(0, 25);
  Serial.print("Current speed: ");
  Serial.print(speed);
/*
  SumSpeed = SumSpeed + speed;
  averageSpeed = SumSpeed / (countLoop+1);
  Serial.print(" - Average speed: ");
  Serial.print(averageSpeed);
  Serial.print(" -  Loop Num: ");
  Serial.print(countLoop+1);
  Serial.print(" -  Sum Speed: ");
  Serial.println(SumSpeed);
*/
 //Set Leds 0 and 1 to ON
 for (int i=3; i<=3; i++){
 led.setBrightness(i, 255);
 #ifdef DEBUGPIN
 Serial.print("on: ");
 Serial.println(i);
 #endif
 delay(speed);}

/*
  //Set Leds 0 and 1 to ON
 for (int i=0; i<=7; i++){
 pump.setBrightness(i, 255);
 #ifdef DEBUGPIN
 Serial.print("on: ");
 Serial.println(i);
  #endif

 delay(speed);}

  //Set Leds 0 and 1 to ON
 for (int i=0; i<=7; i++){
 ssr.setBrightness(i, 255);
 #ifdef DEBUGPIN
 Serial.print("on: ");
 Serial.println(i);
  #endif

 delay(speed);}
*/
 //Set Leds 0 and 1 to OFF
  for (int i=3; i<=3; i++){
 led.setBrightness(i, 0);
 #ifdef DEBUGPIN
 Serial.print("off: ");
 Serial.println(i);
  #endif

 delay(speed);
 }

/*
  //Set Leds 0 and 1 to OFF
  for (int i=0; i<=7; i++){
  pump.setBrightness(i, 0);
  #ifdef DEBUGPIN
 Serial.print("off: ");
 Serial.println(i);
  #endif

 delay(speed);
 }

  //Set Leds 0 and 1 to OFF
  for (int i=0; i<=7; i++){
 ssr.setBrightness(i, 0);
 #ifdef DEBUGPIN
 Serial.print("off: ");
 Serial.println(i);
  #endif

 delay(speed);

 }*/
  //countLoop ++;
  #ifdef i2cscan
 i2c();
 #endif
}

  void i2c(){
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  //delay(5000);           // wait 5 seconds for next scan
}
