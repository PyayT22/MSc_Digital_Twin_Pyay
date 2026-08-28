#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>
#include <DHT.h>

// ======================================================
// Wi-Fi
// ======================================================
const char* ssid = "PLUSNET-PCC9Q2";
const char* wifi_password = "MLaegF9P36exeg";

// ======================================================
// EMQX MQTT
// ======================================================
const char* mqtt_server = "ta6cdf41.ala.eu-central-1.emqxsl.com";
const int mqtt_port = 8883;

const char* mqtt_username = "esp32_smartbuilding";
const char* mqtt_password = "12345678";

const char* mqtt_topic = "smartbuilding/sensors";

// ======================================================
// DHT11
// ======================================================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ======================================================
// LDR
// ======================================================
#define LDR_PIN 36

// ======================================================
// SCD41
// ======================================================
SensirionI2cScd4x scd4x;

// ======================================================
// MQTT CLIENT
// ======================================================
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// ======================================================
// CONNECT TO WI-FI
// ======================================================
void connectWiFi() {

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ======================================================
// CONNECT TO MQTT
// ======================================================
void connectMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Connecting to EMQX MQTT...");

    String clientId = "ESP32-SmartBuilding-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(
          clientId.c_str(),
          mqtt_username,
          mqtt_password)) {

      Serial.println("CONNECTED!");

    } else {

      Serial.print("FAILED, state = ");
      Serial.println(mqttClient.state());

      delay(3000);
    }
  }
}

// ======================================================
// SETUP
// ======================================================
void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("====================================");
  Serial.println("AI-IoT Smart Building MQTT Prototype");
  Serial.println("====================================");

  // DHT11
  dht.begin();

  // SCD41
  Wire.begin(21, 22);

  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  scd4x.stopPeriodicMeasurement();
  delay(500);

  uint16_t error = scd4x.startPeriodicMeasurement();

  if (error) {
    Serial.print("SCD41 start error: ");
    Serial.println(error);
  } else {
    Serial.println("SCD41 initialized");
  }

  // Wi-Fi
  connectWiFi();

  // ----------------------------------------------------
  // TLS
  // ----------------------------------------------------
  // Prototype configuration.
  // Encryption is still used, but certificate validation
  // is disabled temporarily for initial MQTT testing.
  secureClient.setInsecure();

  // MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);

  // MQTT connection
  connectMQTT();

  Serial.println();
  Serial.println("Waiting for sensor measurements...");
  Serial.println();

  delay(5000);
}

// ======================================================
// LOOP
// ======================================================
void loop() {

  // Reconnect Wi-Fi if necessary
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Reconnect MQTT if necessary
  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();

  // ----------------------------------------------------
  // DHT11
  // ----------------------------------------------------
  float dhtTemp = dht.readTemperature();
  float dhtHumidity = dht.readHumidity();

  // ----------------------------------------------------
  // LDR
  // ----------------------------------------------------
  int lightValue = analogRead(LDR_PIN);

  // ----------------------------------------------------
  // SCD41
  // ----------------------------------------------------
  uint16_t co2 = 0;
  float scdTemp = 0.0;
  float scdHumidity = 0.0;

  bool dataReady = false;

  uint16_t error = scd4x.getDataReadyStatus(dataReady);

  if (error) {
    Serial.print("SCD41 data-ready error: ");
    Serial.println(error);
  }

  if (dataReady) {

    error = scd4x.readMeasurement(
      co2,
      scdTemp,
      scdHumidity
    );
  }

  // ----------------------------------------------------
  // SERIAL OUTPUT
  // ----------------------------------------------------
  Serial.println("------------ SENSOR DATA ------------");

  Serial.print("DHT11 Temperature: ");
  Serial.print(dhtTemp);
  Serial.println(" C");

  Serial.print("DHT11 Humidity: ");
  Serial.print(dhtHumidity);
  Serial.println(" %");

  Serial.print("LDR ADC: ");
  Serial.println(lightValue);

  if (dataReady && error == 0) {

    Serial.print("SCD41 CO2: ");
    Serial.print(co2);
    Serial.println(" ppm");

    Serial.print("SCD41 Temperature: ");
    Serial.print(scdTemp);
    Serial.println(" C");

    Serial.print("SCD41 Humidity: ");
    Serial.print(scdHumidity);
    Serial.println(" %");

    // --------------------------------------------------
    // CREATE JSON MQTT MESSAGE
    // --------------------------------------------------
    char payload[300];

    snprintf(
      payload,
      sizeof(payload),
      "{\"dht_temperature\":%.2f,"
      "\"dht_humidity\":%.2f,"
      "\"co2\":%u,"
      "\"scd_temperature\":%.2f,"
      "\"scd_humidity\":%.2f,"
      "\"light_adc\":%d}",
      dhtTemp,
      dhtHumidity,
      co2,
      scdTemp,
      scdHumidity,
      lightValue
    );

    // --------------------------------------------------
    // PUBLISH
    // --------------------------------------------------
    bool published = mqttClient.publish(
      mqtt_topic,
      payload
    );

    if (published) {
      Serial.println("MQTT: DATA PUBLISHED");
      Serial.print("Topic: ");
      Serial.println(mqtt_topic);

      Serial.print("Payload: ");
      Serial.println(payload);
    } else {
      Serial.println("MQTT: PUBLISH FAILED");
    }

  } else {

    Serial.println("SCD41: waiting for measurement...");
  }

  Serial.println("-------------------------------------");
  Serial.println();

  delay(5000);
}