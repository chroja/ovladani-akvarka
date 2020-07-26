#include <pca9634.h>


#include <Wire.h>

// I2C address 0x20: A0-A5->gnd; A6->Vcc
//Pca9634 led( 0x01 );
Pca9634 pump( 0x02 );
//Pca9634 ssr( 0x30 );

void setup() {
  Wire.begin();
  Serial.begin(9600);

  //led.begin();
  pump.begin();
  //ssr.begin();
}

void loop(){

 //Set Leds 0 and 1 to ON
 for (int i=0; i<=7; i++){
 pump.setBrightness(i, 250);
 Serial.print("on: ");
 Serial.println(i);
 delay(500);

 //Set Leds 0 and 1 to OFF
 pump.setBrightness(i, 0);
 Serial.print("off: ");
 Serial.println(i);


 }
}
