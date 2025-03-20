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

// Define tags for logging
static const char* TAG_MAIN = "MAIN";
static const char* TAG_WIFI = "WIFI";
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

//const unsigned long WIFI_WATCHDOG_TIMEOUT = 1800000; // 30 minutes in milliseconds
//const int MAX_RECONNECT_ATTEMPTS = 10; // Increased to 10 attempts

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

// In setup function
void setup() {
    Serial.begin(115200);
    
    // Initialize logger with default log level
    Logger::getInstance().setLogLevel(LOG_INFO);
    LOG_I(TAG_MAIN, "ESP32 KNX Thermostat - With Adaptive PID Controller");
    
    // Register log callback for persistent logging
    Logger::getInstance().setLogCallback(storeLogToFlash);
    
    // Initialize config manager
    configManager = ConfigManager::getInstance();
    if (!configManager->begin()) {
        LOG_E(TAG_MAIN, "Failed to initialize configuration storage");
    }
    
    // Initialize watchdog timer (45 minutes)
    // Initialize the watchdog manager
    if (!watchdogManager.begin()) {
      LOG_E(TAG_MAIN, "Failed to initialize watchdog manager");
    }
    
    // Remove the old watchdog initialization code:
    // esp_task_wdt_init(SYSTEM_WATCHDOG_TIMEOUT / 1000, true);
    // esp_task_wdt_add(NULL);
    LOG_I(TAG_MAIN, "Watchdog timer initialized (45 minutes)");

    // Setup custom log handler before initializing KNX
    setupCustomLogHandler();
    
    // Initialize BME280 sensor
    if (!bme280.begin()) {
        LOG_E(TAG_SENSOR, "Failed to initialize BME280 sensor!");
    }
    
    // Replace old WiFi setup with WiFiConnectionManager
    LOG_I(TAG_WIFI, "Initializing WiFi connection manager...");
    WiFiConnectionManager& wifiManager = WiFiConnectionManager::getInstance();
    
    // Register for WiFi events - Fix the capture issue by adding wifiManager to the capture list
    wifiManager.registerEventCallback([&wifiManager](const WiFiConnectionEvent& event) {
        LOG_I(TAG_WIFI, "WiFi event: %s - %s", 
              wifiManager.getEventTypeName(event.type), 
              event.message.c_str());
              
        // If we're connected, log the network details
        if (event.type == WiFiEventType::CONNECTED) {
            LOG_I(TAG_WIFI, "Connected to: %s", event.ssid.c_str());
            LOG_I(TAG_WIFI, "IP address: %s", event.networkInfo.ip.toString().c_str());
        }
    });
    
    // Initialize WiFi with appropriate settings
    bool wifiConnected = wifiManager.begin(180, true); // 3 minute timeout, start portal on fail
    
    if (!wifiConnected) {
        LOG_W(TAG_WIFI, "WiFi connection failed or timed out during setup");
    }
    
    // Initialize WebServerManager singleton
    WebServerManager* webServerManager = WebServerManager::getInstance();
    webServerManager->begin(&webServer);
    LOG_I(TAG_MAIN, "Web server started on port 80");

    // Initialize OTA using WebServerManager
    otaManager.begin(webServerManager);
    LOG_I(TAG_MAIN, "OTA manager initialized with web server");
    
    // Setup KNX and MQTT managers
    knxManager.begin();
    mqttManager.begin();
    
    // Connect the managers for cross-communication
    knxManager.setMQTTManager(&mqttManager);
    mqttManager.setKNXManager(&knxManager);
    
    // Register callback for KNX address configuration changes
    webServerManager->setKnxAddressChangedCallback([&]() {
        LOG_I(TAG_MAIN, "KNX address configuration changed, reloading addresses");
        knxManager.reloadAddresses();
    });

    // Initialize adaptive PID controller
    initializePIDController();
    
    // Use stored setpoint temperature from config
    float setpoint = configManager->getSetpoint();
    setTemperatureSetpoint(setpoint);
    
    LOG_I(TAG_PID, "PID controller initialized with setpoint: %.2f°C", setpoint);
    
    // Initial sensor readings
    updateSensorReadings();
    
    // Initialize last connected time
    if (WiFi.status() == WL_CONNECTED) {
        lastConnectedTime = millis();
    }
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
    
    // Remove old WiFiManager process call as it's now handled by WiFiConnectionManager
    // wifiManager.process();
    
    // Remove old WiFi watchdog code as it's now handled by WiFiConnectionManager
    // if (WiFi.status() != WL_CONNECTED && !configPortalActive) {
    //     if (millis() - lastConnectedTime > WIFI_WATCHDOG_TIMEOUT) {
    //         LOG_E(TAG_WIFI, "WiFi disconnected for 30 minutes. Rebooting device...");
    //         delay(1000);
    //         ESP.restart();
    //     }
    // }
    
    // Add periodic internet connectivity test
    static unsigned long lastConnectivityCheck = 0;
    if (millis() - lastConnectivityCheck > 300000) { // Every 5 minutes
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
    
    // Save all PID parameters to config
    // Only do this if they've changed significantly (to reduce flash wear)
    static float last_saved_kp = g_pid_input.Kp;
    static float last_saved_ki = g_pid_input.Ki;
    static float last_saved_kd = g_pid_input.Kd;
    static float last_saved_setpoint = g_pid_input.setpoint_temp;
    
    bool params_changed = false;
    
    if (fabs(last_saved_kp - g_pid_input.Kp) > 0.001f) {
        configManager->setPidKp(g_pid_input.Kp);
        last_saved_kp = g_pid_input.Kp;
        params_changed = true;
    }
    
    if (fabs(last_saved_ki - g_pid_input.Ki) > 0.001f) {
        configManager->setPidKi(g_pid_input.Ki);
        last_saved_ki = g_pid_input.Ki;
        params_changed = true;
    }
    
    if (fabs(last_saved_kd - g_pid_input.Kd) > 0.001f) {
        configManager->setPidKd(g_pid_input.Kd);
        last_saved_kd = g_pid_input.Kd;
        params_changed = true;
    }
    
    if (fabs(last_saved_setpoint - g_pid_input.setpoint_temp) > 0.01f) {
        configManager->setSetpoint(g_pid_input.setpoint_temp);
        last_saved_setpoint = g_pid_input.setpoint_temp;
        params_changed = true;
    }
    
    if (params_changed) {
        LOG_I(TAG_PID, "PID parameters updated in storage");
    }
}

// Callback for storing logs to persistent storage
void storeLogToFlash(LogLevel level, const char* tag, const char* message, unsigned long timestamp) {
    // Only store errors and warnings to save flash wear
    if (level <= LOG_WARNING) {
        // Get current date/time if RTC is available
        char timeStr[20];
        sprintf(timeStr, "%lu", timestamp);
        
        // Format: timestamp|level|tag|message
        String logEntry = String(timeStr) + "|" + String(level) + "|" + tag + "|" + message;
        
        // In a real implementation, append this to a log file in SPIFFS
        // This is just a placeholder since file operations can be slow
        // and would affect main loop timing
        if (level == LOG_ERROR) {
            // For errors, we might want to ensure they're saved immediately
            // File logFile = SPIFFS.open("/error.log", "a");
            // logFile.println(logEntry);
            // logFile.close();
        }
    }
}