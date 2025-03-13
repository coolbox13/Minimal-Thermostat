#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <esp-knx-ip.h>
#include "bme280_sensor.h"
#include "config.h"
#include "utils.h"
#include "knx_manager.h"
#include "mqtt_manager.h"

// commit
// Global variables
BME280Sensor bme280;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESPKNXIP knxInstance;  // Renamed from 'knx' to 'knxInstance' to avoid conflict
KNXManager knxManager(knxInstance);
MQTTManager mqttManager(mqttClient);
float temperature = 0;
float humidity = 0;
float pressure = 0;

// Function declarations
void setupWiFi();
void updateSensorReadings();

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 KNX Thermostat - Modular Version");
  
  // Initialize BME280 sensor
  if (!bme280.begin()) {
    Serial.println("Failed to initialize BME280 sensor!");
  }
  
  // Setup WiFi
  setupWiFi();
  
  // Setup KNX and MQTT managers
  knxManager.begin();
  mqttManager.begin();
  
  // Connect the managers for cross-communication
  knxManager.setMQTTManager(&mqttManager);
  mqttManager.setKNXManager(&knxManager);
  
  // Initial sensor readings
  updateSensorReadings();
}

void loop() {
  // Handle KNX communications
  knxManager.loop();
  
  // Monitor and decode KNX debug messages if enabled
  monitorKnxDebugMessages();
  
  // Handle MQTT communications
  mqttManager.loop();
  
  // Update sensor readings and publish status every 30 seconds
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 30000) {
    updateSensorReadings();
    lastUpdate = millis();
  }
}

void setupWiFi() {
  Serial.println("Setting up WiFi...");
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  
  if (!wifiManager.autoConnect("ESP32-Thermostat-AP")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void updateSensorReadings() {
  temperature = bme280.readTemperature();
  humidity = bme280.readHumidity();
  pressure = bme280.readPressure();
  
  Serial.println("Sensor readings updated:");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Pressure: "); Serial.print(pressure); Serial.println(" hPa");
  Serial.print("Valve position: "); Serial.print(knxManager.getValvePosition()); Serial.println(" %");
  
  // Send sensor data to KNX
  knxManager.sendSensorData(temperature, humidity, pressure);
  
  // Publish sensor data to MQTT
  mqttManager.publishSensorData(temperature, humidity, pressure);
}