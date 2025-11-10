#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include "logger.h"
#include "esp-knx-ip/esp-knx-ip.h"
#include "bme280_sensor.h"
#include "config.h"
#include "utils.h"
#include "knx_manager.h"
#include "mqtt_manager.h"
#include "adaptive_pid_controller.h"
#include "ota_manager.h"
#include "valve_control.h"
#include "config_manager.h"
#include "web_server.h"
#include "watchdog_manager.h"
#include "wifi_connection.h"
#include "event_log.h"


static const char* TAG_MAIN = "MAIN";
static const char* TAG_SENSOR = "SENSOR";
static const char* TAG_PID = "PID";

// Global variables
BME280Sensor bme280;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESPKNXIP knxInstance;  // Using our local instance
KNXManager knxManager(knxInstance);
MQTTManager mqttManager(mqttClient);
OTAManager otaManager;
ValveControl valveControl(mqttClient, knxInstance);
ConfigManager* configManager;
float temperature = 0;
float humidity = 0;
float pressure = 0;

// Make WiFiManager persistent
WiFiManager wifiManager;

// WiFi monitoring variables
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 60000; // Check WiFi every minute
unsigned long lastConnectedTime = 0;
int reconnectAttempts = 0;
bool configPortalActive = false;

// Function declarations
void setupWiFi();
void checkWiFiConnection();
void updateSensorReadings();
void updatePIDControl();
void storeLogToFlash(LogLevel level, const char* tag, const char* message, unsigned long timestamp);

// Create a global web server
AsyncWebServer webServer(80);

// Timing for PID updates
unsigned long lastPIDUpdate = 0;

// Add global instance of WatchdogManager
WatchdogManager watchdogManager;

// Helper functions for setup
// IMPORTANT: These functions must be called in a specific order due to dependencies:
// 1. initializeLogger() - No dependencies, required by all other components
// 2. initializeConfig() - Depends on logger, required by most other components
// 3. initializeWatchdog() - Depends on logger
// 4. initializeSensor() - Depends on logger, sets up custom log handler for KNX
// 5. initializeWiFi() - Depends on logger and config (reads WiFi credentials)
// 6. initializeWebServer() - Depends on WiFi (needs network), logger
// 7. initializeKNXAndMQTT() - Depends on logger, WiFi, web server (for callbacks)
// 8. initializePID() - Depends on config (reads setpoint), logger
// 9. performInitialSetup() - Depends on all above (WiFi status, sensors, watchdog)

void initializeLogger() {
    Logger::getInstance().setLogLevel(LOG_INFO);
    LOG_I(TAG_MAIN, "ESP32 KNX Thermostat - With Adaptive PID Controller");

    // Initialize EventLog for persistent logging
    EventLog::getInstance().begin();
    LOG_I(TAG_MAIN, "Event log initialized");

    // Register log callback for persistent logging
    Logger::getInstance().setLogCallback(storeLogToFlash);
}
void initializeConfig() {
    configManager = ConfigManager::getInstance();
    if (!configManager->begin()) {
        LOG_E(TAG_MAIN, "Failed to initialize configuration storage");
    }
}
void initializeWatchdog() {
    if (!watchdogManager.begin()) {
        LOG_E(TAG_MAIN, "Failed to initialize watchdog manager");
    }
    LOG_I(TAG_MAIN, "Watchdog timer initialized (45 minutes)");
}
void initializeSensor() {
    setupCustomLogHandler();
    if (!bme280.begin()) {
        LOG_E(TAG_SENSOR, "Failed to initialize BME280 sensor!");
    }
}
void initializeWiFi() {
    LOG_I(TAG_WIFI, "Initializing WiFi connection manager...");
    WiFiConnectionManager& wifiManager = WiFiConnectionManager::getInstance();
    wifiManager.registerEventCallback([&wifiManager](const WiFiConnectionEvent& event) {
        LOG_I(TAG_WIFI, "WiFi event: %s - %s",
              wifiManager.getEventTypeName(event.type),
              event.message.c_str());
        if (event.type == WiFiEventType::CONNECTED) {
            LOG_I(TAG_WIFI, "Connected to: %s", event.ssid.c_str());
            LOG_I(TAG_WIFI, "IP address: %s", event.networkInfo.ip.toString().c_str());
        }
    });
    bool wifiConnected = wifiManager.begin(configManager->getWifiConnectTimeout(), true);
    if (!wifiConnected) {
        LOG_W(TAG_WIFI, "WiFi connection failed or timed out during setup");
    }
}
void initializeWebServer() {
    WebServerManager* webServerManager = WebServerManager::getInstance();
    webServerManager->begin(&webServer);
    LOG_I(TAG_MAIN, "Web server started on port 80");
    otaManager.begin(webServerManager);
    LOG_I(TAG_MAIN, "OTA manager initialized with web server");
}
void initializeKNXAndMQTT() {
    knxManager.begin();
    mqttManager.begin();

    // Connect the managers for cross-communication
    knxManager.setMQTTManager(&mqttManager);
    mqttManager.setKNXManager(&knxManager);

    // Setup MQTT logging callback for EventLog
    EventLog::getInstance().setMQTTLoggingEnabled(true);
    EventLog::getInstance().setMQTTCallback([](LogLevel level, const char* tag, const char* message) {
        if (mqttClient.connected()) {
            // Create JSON payload for the log
            StaticJsonDocument<256> doc;
            doc["timestamp"] = millis();
            doc["level"] = EventLog::logLevelToString(level);
            doc["tag"] = tag;
            doc["message"] = message;

            String payload;
            serializeJson(doc, payload);

            mqttClient.publish("esp32_thermostat/logs", payload.c_str());
        }
    });

    // Register callback for KNX address configuration changes
    WebServerManager* webServerManager = WebServerManager::getInstance();
    webServerManager->setKnxAddressChangedCallback([&]() {
        LOG_I(TAG_MAIN, "KNX address configuration changed, reloading addresses");
        knxManager.reloadAddresses();
    });
}
void initializePID() {
    initializePIDController();
    float setpoint = configManager->getSetpoint();
    setTemperatureSetpoint(setpoint);
    LOG_I(TAG_PID, "PID controller initialized with setpoint: %.2f°C", setpoint);
}
void performInitialSetup() {
    updateSensorReadings();
    if (WiFi.status() == WL_CONNECTED) {
        lastConnectedTime = millis();
    }
    WiFiConnectionManager::getInstance().setWatchdogManager(&watchdogManager);
    WiFiConnectionManager::getInstance().begin();
}

// In setup function
void setup() {
    Serial.begin(115200);
    initializeLogger();
    initializeConfig();
    initializeWatchdog();
    initializeSensor();
    initializeWiFi();
    initializeWebServer();
    initializeKNXAndMQTT();
    initializePID();
    performInitialSetup();
}

// In loop function
void loop() {
    // Reset watchdog timer to prevent reboot
    // Update the watchdog manager at the beginning of each loop
    watchdogManager.update();
  
    // Replace old WiFi check with WiFiConnectionManager loop
    WiFiConnectionManager::getInstance().loop();
    
    // Handle KNX communications
    knxManager.loop();
    
    // Monitor and decode KNX debug messages if enabled
    monitorKnxDebugMessages();
    
    // Handle MQTT communications
    mqttManager.loop();
    
    // Update sensor readings and publish status
    static unsigned long lastSensorUpdate = 0;
    if (millis() - lastSensorUpdate > configManager->getSensorUpdateInterval()) {
        updateSensorReadings();
        lastSensorUpdate = millis();
    }

    // Update PID controller at specified interval
    unsigned long currentTime = millis();
    if (currentTime - lastPIDUpdate > configManager->getPidUpdateInterval()) {
        updatePIDControl();
        lastPIDUpdate = currentTime;
    }

    // Add periodic internet connectivity test
    static unsigned long lastConnectivityCheck = 0;
    if (millis() - lastConnectivityCheck > configManager->getConnectivityCheckInterval()) {
        lastConnectivityCheck = millis();
        
        WiFiConnectionManager& wifiManager = WiFiConnectionManager::getInstance();
        if (wifiManager.getState() == WiFiConnectionState::CONNECTED) {
            if (wifiManager.testInternetConnectivity()) {
                LOG_D(TAG_WIFI, "Internet connectivity test passed");
            } else {
                LOG_W(TAG_WIFI, "Internet connectivity test failed despite WiFi connection");
            }
        }
    }
}

void updateSensorReadings() {
    temperature = bme280.readTemperature();
    humidity = bme280.readHumidity();
    pressure = bme280.readPressure();
    
    LOG_D(TAG_SENSOR, "Sensor readings updated:");
    LOG_D(TAG_SENSOR, "Temperature: %.2f °C", temperature);
    LOG_D(TAG_SENSOR, "Humidity: %.2f %%", humidity);
    LOG_D(TAG_SENSOR, "Pressure: %.2f hPa", pressure);
    LOG_D(TAG_SENSOR, "Valve position: %d %%", knxManager.getValvePosition());
    
    // Send sensor data to KNX
    knxManager.sendSensorData(temperature, humidity, pressure);
    
    // Publish sensor data to MQTT
    mqttManager.publishSensorData(temperature, humidity, pressure);
}

// Modified updatePIDControl function for main.cpp
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
    knxManager.setValvePosition(pidOutput);
    
    // Log PID information
    LOG_D(TAG_PID, "PID controller updated:");
    LOG_D(TAG_PID, "Temperature: %.2f°C, Setpoint: %.2f°C", 
          currentTemp, g_pid_input.setpoint_temp);
    LOG_D(TAG_PID, "Valve position: %.1f%%", pidOutput);
    LOG_D(TAG_PID, "PID params - Kp: %.3f, Ki: %.3f, Kd: %.3f", 
          g_pid_input.Kp, g_pid_input.Ki, g_pid_input.Kd);
    
    // Save PID parameters to config with write coalescing to reduce flash wear
    // Write to flash max once every 5 minutes if parameters have changed
    static float last_saved_kp = g_pid_input.Kp;
    static float last_saved_ki = g_pid_input.Ki;
    static float last_saved_kd = g_pid_input.Kd;
    static float last_saved_setpoint = g_pid_input.setpoint_temp;
    static unsigned long lastConfigWrite = 0;
    static bool pendingConfigWrite = false;
    if (fabs(last_saved_kp - g_pid_input.Kp) > 0.001f ||
        fabs(last_saved_ki - g_pid_input.Ki) > 0.001f ||
        fabs(last_saved_kd - g_pid_input.Kd) > 0.001f ||
        fabs(last_saved_setpoint - g_pid_input.setpoint_temp) > 0.01f) {
        pendingConfigWrite = true;
    }
    if (pendingConfigWrite && (millis() - lastConfigWrite > configManager->getPidConfigWriteInterval())) {
        configManager->setPidKp(g_pid_input.Kp);
        configManager->setPidKi(g_pid_input.Ki);
        configManager->setPidKd(g_pid_input.Kd);
        configManager->setSetpoint(g_pid_input.setpoint_temp);
        last_saved_kp = g_pid_input.Kp;
        last_saved_ki = g_pid_input.Ki;
        last_saved_kd = g_pid_input.Kd;
        last_saved_setpoint = g_pid_input.setpoint_temp;
        lastConfigWrite = millis();
        pendingConfigWrite = false;
        LOG_I(TAG_PID, "PID parameters written to flash storage");
    }
}

// Callback for storing logs to persistent storage
void storeLogToFlash(LogLevel level, const char* tag, const char* message, unsigned long timestamp) {
    // Only store errors and warnings to save flash wear
    if (level <= LOG_WARNING) {
        // Store to EventLog (using the same LogLevel enum)
        EventLog::getInstance().addEntry(level, tag, message);
    }
}