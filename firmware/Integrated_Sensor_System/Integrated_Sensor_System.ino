#include <Wire.h>
#include <DHT.h>
#include <SensirionI2cScd4x.h>

// -------------------- PIN DEFINITIONS --------------------
#define DHTPIN 4
#define DHTTYPE DHT11

#define LDR_PIN 36

// -------------------- SENSOR OBJECTS --------------------
DHT dht(DHTPIN, DHTTYPE);
SensirionI2cScd4x scd41;

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== SMART BUILDING SENSOR SYSTEM ===");

  // DHT11
  dht.begin();

  // I2C for SCD41
  Wire.begin(21, 22);   // SDA = GPIO21, SCL = GPIO22

  scd41.begin(Wire, SCD41_I2C_ADDR_62);

  uint16_t error;

  // Stop any previous measurement session
  error = scd41.stopPeriodicMeasurement();
  delay(500);

  // Reinitialise sensor
  error = scd41.reinit();
  delay(500);

  // Start periodic measurements
  error = scd41.startPeriodicMeasurement();

  if (error != 0) {
    Serial.print("SCD41 START ERROR: ");
    Serial.println(error);
  } else {
    Serial.println("SCD41 started successfully.");
  }

  Serial.println("--------------------------------");
}

// -------------------- MAIN LOOP --------------------
void loop() {

  // =====================================================
  // DHT11
  // =====================================================
  float tempDHT = dht.readTemperature();
  float humDHT = dht.readHumidity();

  if (isnan(tempDHT) || isnan(humDHT)) {
    Serial.println("DHT11 | READ ERROR");
  } else {
    Serial.print("DHT11 | Temp: ");
    Serial.print(tempDHT, 1);
    Serial.print(" C | Humidity: ");
    Serial.print(humDHT, 1);
    Serial.println(" %");
  }

  // =====================================================
  // LDR
  // =====================================================
  int lightValue = analogRead(LDR_PIN);

  Serial.print("LDR   | Light ADC: ");
  Serial.println(lightValue);

  // =====================================================
  // SCD41
  // =====================================================
  uint16_t error;
  bool dataReady = false;

  error = scd41.getDataReadyStatus(dataReady);

  if (error != 0) {
    Serial.print("SCD41 | STATUS ERROR: ");
    Serial.println(error);
  }

  else if (dataReady) {

    uint16_t co2 = 0;
    float tempSCD = 0.0;
    float humSCD = 0.0;

    error = scd41.readMeasurement(
      co2,
      tempSCD,
      humSCD
    );

    if (error == 0) {

      Serial.print("SCD41 | CO2: ");
      Serial.print(co2);
      Serial.print(" ppm | Temp: ");
      Serial.print(tempSCD, 1);
      Serial.print(" C | Humidity: ");
      Serial.print(humSCD, 1);
      Serial.println(" %");

    } else {

      Serial.print("SCD41 | READ ERROR: ");
      Serial.println(error);
    }
  }

  Serial.println("--------------------------------");

  // SCD41 produces a new measurement about every 5 seconds
  delay(5000);
}
