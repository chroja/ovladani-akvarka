#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 40

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Addresses of 3 DS18B20s
uint8_t sensor1[8] = { 0x28, 0x25, 0xC5, 0xF7, 0x08, 0x00, 0x00, 0x61 };
uint8_t sensor2[8] = { 0x28, 0xFF, 0x1A, 0x62, 0xC0, 0x17, 0x05, 0xF0 };
uint8_t sensor3[8] = { 0x28, 0x06, 0x3B, 0xF8, 0x08, 0x00, 0x00, 0x10 };


void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
}

void loop(void)
{
  sensors.requestTemperatures();

  Serial.print("Sensor 1: ");
  printTemperature(sensor1);

  Serial.print("Sensor 2: ");
  printTemperature(sensor2);

  Serial.print("Sensor 3: ");
  printTemperature(sensor3);

  Serial.println();
  delay(1000);
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(tempC);
  Serial.print((char)176);
  Serial.print("C  |  ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
  Serial.print((char)176);
  Serial.println("F");
}
