#define LDR_PIN 36

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== LDR INDIVIDUAL SENSOR TEST ===");
  Serial.println("Testing light sensor...");
}

void loop() {
  int lightValue = analogRead(LDR_PIN);

  Serial.print("Light ADC: ");
  Serial.println(lightValue);

  delay(1000);
}
