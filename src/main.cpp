#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include "esp-knx-ip/esp-knx-ip.h"
#include "bme280_sensor.h"
#include "config.h"
#include "utils.h"
#include "knx_manager.h"
#include "mqtt_manager.h"
#include "adaptive_pid_controller.h"
#include "ota_manager.h"

// Global variables
BME280Sensor bme280;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESPKNXIP knxInstance;  // Using our local instance
KNXManager knxManager(knxInstance);
MQTTManager mqttManager(mqttClient);
OTAManager otaManager;
float temperature = 0;
float humidity = 0;
float pressure = 0;

// Make WiFiManager persistent
WiFiManager wifiManager;

// WiFi monitoring variables
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 60000; // Check WiFi every minute
unsigned long lastConnectedTime = 0;
const unsigned long WIFI_WATCHDOG_TIMEOUT = 1800000; // 30 minutes in milliseconds
int reconnectAttempts = 0;
const int MAX_RECONNECT_ATTEMPTS = 10; // Increased to 10 attempts
bool configPortalActive = false;

// Function declarations
void setupWiFi();
void checkWiFiConnection();
void updateSensorReadings();
void updatePIDControl();

// Timing for PID updates
unsigned long lastPIDUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 KNX Thermostat - With Adaptive PID Controller");
  
  // Initialize watchdog timer (45 minutes)
  esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert ms to seconds, true = reboot on timeout
  esp_task_wdt_add(NULL); // Add current thread to watchdog
  Serial.println("Watchdog timer initialized (45 minutes)");

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

  // Initialize OTA using the KNX web server
  // Get the web server instance from knxInstance
  AsyncWebServer* webServer = knxInstance.getWebServer();
  if (webServer) {
    otaManager.begin(webServer);
    Serial.println("OTA manager initialized with KNX web server");
  } else {
    Serial.println("Failed to initialize OTA - no web server available");
  }
  
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
  
  // Initialize last connected time
  if (WiFi.status() == WL_CONNECTED) {
    lastConnectedTime = millis();
  }
}

void loop() {
  // Reset watchdog timer to prevent reboot
  esp_task_wdt_reset();

  // Check WiFi connection status periodically
  if (millis() - lastWifiCheck > WIFI_CHECK_INTERVAL) {
    checkWiFiConnection();
    lastWifiCheck = millis();
  }
  
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
  
  // Process WiFiManager in non-blocking mode
  wifiManager.process();
  
  // WiFi watchdog - reboot if disconnected for too long
  if (WiFi.status() != WL_CONNECTED && !configPortalActive) {
    if (millis() - lastConnectedTime > WIFI_WATCHDOG_TIMEOUT) {
      Serial.println("WiFi disconnected for 30 minutes. Rebooting device...");
      delay(1000);
      ESP.restart();
    }
  }
}

void setupWiFi() {
  Serial.println("Setting up WiFi...");
  
  // Configure WiFiManager
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectRetries(10); // Set to 10 reconnection attempts
  
  if (!wifiManager.autoConnect("ESP32-Thermostat-AP")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  lastConnectedTime = millis(); // Update last connected time
}

void checkWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    // Update last connected time when WiFi is connected
    lastConnectedTime = millis();
    reconnectAttempts = 0;
  } else {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    
    // Increment reconnect attempts
    reconnectAttempts++;
    
    if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
      // Try to reconnect using WiFi.begin() with saved credentials
      WiFi.disconnect();
      delay(1000);
      WiFi.begin();
      
      // Wait up to 10 seconds for connection
      unsigned long startAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
      }
      Serial.println();
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Reconnected to WiFi");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        lastConnectedTime = millis(); // Update last connected time
      } else {
        Serial.println("Reconnection attempt failed");
        Serial.print("Attempt ");
        Serial.print(reconnectAttempts);
        Serial.print(" of ");
        Serial.println(MAX_RECONNECT_ATTEMPTS);
      }
    } else {
      // After several failed attempts, start config portal
      Serial.println("Multiple reconnection attempts failed. Starting config portal...");
      configPortalActive = true;
      wifiManager.startConfigPortal("ESP32-Thermostat-Recovery");
    }
    
    // Print time since last connection
    unsigned long disconnectedTime = (millis() - lastConnectedTime) / 1000;
    Serial.print("Time since last connection: ");
    Serial.print(disconnectedTime);
    Serial.println(" seconds");
    
    // Print time remaining before watchdog reboot
    if (disconnectedTime > 0) {
      unsigned long timeToReboot = (WIFI_WATCHDOG_TIMEOUT / 1000) - disconnectedTime;
      Serial.print("Device will reboot in ");
      Serial.print(timeToReboot);
      Serial.println(" seconds if WiFi remains disconnected");
    }
  }
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