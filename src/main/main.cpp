#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <Adafruit_BME280.h>
#include "config_manager.h"
#include "web_interface.h"
#include "thermostat_state.h"
#include "sensor_interface.h"
#include "pid_controller.h"

// Global objects
Adafruit_BME280 bme;
bool bmeAvailable = false;
ConfigManager configManager;
ThermostatState thermostatState;
SensorInterface sensorInterface;
PIDController pidController;
WebInterface webInterface;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nStarting ESP32-KNX-Thermostat...");

  // Initialize ConfigManager and load configuration
  if (!configManager.begin()) {
    Serial.println("ConfigManager initialization failed!");
  }

  // Initialize WiFi using ConfigManager
  if (!configManager.setupWiFi()) {
    Serial.println("WiFi setup failed!");
    ESP.restart();
  }

  // Initialize sensors
  bmeAvailable = sensorInterface.begin(&thermostatState);

  // Initialize PID Controller
  pidController.begin(&thermostatState, 1.0, 0.1, 0.01);

  // Initialize WebInterface
  if (!webInterface.begin(&thermostatState, &sensorInterface, &pidController)) {
    Serial.println("WebInterface initialization failed!");
  }

  Serial.println("Setup completed");
}

void loop() {
  // Handle web server requests
  webInterface.handle();

  // Read BME280 every 10 seconds if available
  static unsigned long lastReadTime = 0;
  if (bmeAvailable && millis() - lastReadTime > 10000) {
    sensorInterface.update();
    lastReadTime = millis();
  }

  // Update PID controller
  pidController.update();

  delay(10);
}