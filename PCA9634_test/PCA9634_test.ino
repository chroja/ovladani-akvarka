#include <pca9634.h>


#include <Wire.h>

// I2C address 0x20: A0-A5->gnd; A6->Vcc
Pca9634 led( 0x30 );

void setup() {
  Wire.begin();
  Serial.begin(9600);

  led.begin();
}

void loop(){

 //Set Leds 0 and 1 to ON
 for (int i=0; i<=7; i++){
 led.setBrightness(i, 255);
 Serial.print("on: ");
 Serial.println(i);
 delay(500);

 //Set Leds 0 and 1 to OFF
 led.setBrightness(i, 0);
 Serial.print("off: ");
 Serial.println(i);
 i2c();

 }
}
