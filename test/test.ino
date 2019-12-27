int Reset = 53;

void setup() {
  digitalWrite(Reset, HIGH);
  delay(200);
  pinMode(Reset, OUTPUT);
  Serial.begin(115200);
  Serial.println("How to Reset Arduino Programmatically");
  Serial.println("www.TheEngineeringProjects.com");
  delay(200);
}
void loop()
{
  Serial.println("A");
  delay(1000);
  Serial.println("B");
  delay(1000);
  Serial.println("Now we are Resetting Arduino Programmatically");
  Serial.println();
  delay(1000);
  digitalWrite(Reset, LOW);
  Serial.println("Arduino will never reach there.");
}
