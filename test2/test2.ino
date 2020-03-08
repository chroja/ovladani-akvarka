#include <LEDStripDriver.h>

// use two available GPIO pins from your board
// DIN=GPIO16, CIN=GPIO14 in this example
LEDStripDriver led = LEDStripDriver(10, 11);

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  led.setColor(255, 0, 0); // RGB
  delay(100);
  led.setColor(0,255,0); // String
  delay(100);
  led.setColor(0,0,255); // HEX
  delay(100);
  led.setColor(); // turn off
  delay(1000);
}
