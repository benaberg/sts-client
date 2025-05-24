#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SENSOR_PIN 8
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Seconds to sleep

#define WIFI_SSID "FIXME"
#define WIFI_PASSWORD "FIXME"
#define URL "FIXME"

HTTPClient client;
WiFiMulti wifiMulti;
OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorAddress;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Prepare sensors
  sensors.begin();
  sensors.setResolution(sensorAddress, 11);

  // Connect to WIFI
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Waiting for WIFI connection...");
    delay(500);
  }

  // Perform update
  performUpdate();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to sleep...");
  Serial.flush();
  esp_deep_sleep_start();
}

void performUpdate() {
  if (wifiMulti.run() == WL_CONNECTED) {
    // Start connection
    client.begin(URL);

    // Read sensor reading
    sensors.requestTemperatures();
    float reading = sensors.getTempCByIndex(0);
    Serial.printf("Read temperature: %.2f\n", reading);

    // Construct payload
    String payload = String("{\"temperature\":") + String((int) reading) + String("}");
    Serial.println("Sending temperature data...");

    // Send JSON payload
    int httpCode = client.PUT(payload);
    if (httpCode > 0) {
      Serial.printf("[HTTP] Received response code: %d\n", httpCode);
    }
    else {
      Serial.printf("[HTTP] Error: %s\n", client.errorToString(httpCode).c_str());
    }
    client.end();
  }
}

void loop() {
  // No op
}
