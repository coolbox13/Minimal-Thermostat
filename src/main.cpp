#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
// Update the include path to use your local version
#include "esp-knx-ip/esp-knx-ip.h"
#include "bme280_sensor.h"
#include "config.h"
#include "utils.h"
#include "knx_manager.h"
#include "mqtt_manager.h"
#include "adaptive_pid_controller.h"

// Global variables
BME280Sensor bme280;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESPKNXIP knxInstance;  // Using our local instance
KNXManager knxManager(knxInstance);
MQTTManager mqttManager(mqttClient);
float temperature = 0;
float humidity = 0;
float pressure = 0;

// Function declarations
void setupWiFi();
void updateSensorReadings();
void updatePIDControl();

// Timing for PID updates
unsigned long lastPIDUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 KNX Thermostat - With Adaptive PID Controller");
  
  // Setup custom log handler before initializing KNX
  setupCustomLogHandler();
  
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
  
  // Initialize adaptive PID controller
  initializePIDController();
  
  // Set initial temperature setpoint from config
  setTemperatureSetpoint(PID_SETPOINT);
  
  Serial.print("PID controller initialized with setpoint: ");
  Serial.print(PID_SETPOINT);
  Serial.println("°C");
  
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
  static unsigned long lastSensorUpdate = 0;
  if (millis() - lastSensorUpdate > 30000) {
    updateSensorReadings();
    lastSensorUpdate = millis();
  }
  
  // Update PID controller at specified interval
  unsigned long currentTime = millis();
  if (currentTime - lastPIDUpdate > PID_UPDATE_INTERVAL) {
    updatePIDControl();
    lastPIDUpdate = currentTime;
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

void updatePIDControl() {
  // Get current temperature from BME280
  float currentTemp = bme280.readTemperature();
  
  // Get current valve position from KNX (feedback)
  float valvePosition = knxManager.getValvePosition();
  
  // Update PID controller
  updatePIDController(currentTemp, valvePosition);
  
  // Get new valve position from PID
  float pidOutput = getPIDOutput();
  
  // Apply PID output to valve control via KNX
  // We're using the test valve address for now
  knxManager.setValvePosition(pidOutput);
  
  // Log PID information
  Serial.println("PID controller updated:");
  Serial.print("Temperature: "); Serial.print(currentTemp); Serial.print("°C, Setpoint: ");
  Serial.print(g_pid_input.setpoint_temp); Serial.println("°C");
  Serial.print("Valve position: "); Serial.print(pidOutput); Serial.println("%");
  Serial.print("PID params - Kp: "); Serial.print(g_pid_input.Kp);
  Serial.print(", Ki: "); Serial.print(g_pid_input.Ki);
  Serial.print(", Kd: "); Serial.print(g_pid_input.Kd);
  Serial.println();
}