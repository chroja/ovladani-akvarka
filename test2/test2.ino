#define RGBLedNum 4
#define RGBDataPin 2
#define RGBClockPin 3
int speed = 30;

#include <FastLED.h>

CRGB RBGLeds[RGBLedNum];

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<P9813, RGBDataPin, RGBClockPin, RGB>(RBGLeds, RGBLedNum);  // BGR ordering is typical

}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < RGBLedNum; i++) {
      RBGLeds[i] = CRGB(255,0,0);
      FastLED.show();
      delay ;
      RBGLeds[i] = CRGB(0,255,0);
      FastLED.show();
      delay (speed);
      RBGLeds[i] = CRGB(0,0,255);
      FastLED.show();
      delay (speed);
      RBGLeds[i] = CRGB(255,255,255);
      FastLED.show();
      delay (speed);
      RBGLeds[i] = CRGB(0,0,0);
      FastLED.show();
      delay (speed);
    }
}
