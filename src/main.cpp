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
const unsigned long WIFI_WATCHDOG_TIMEOUT = 1800000; // 30 minutes in milliseconds
int reconnectAttempts = 0;
const int MAX_RECONNECT_ATTEMPTS = 10; // Increased to 10 attempts
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
    esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true);
    esp_task_wdt_add(NULL);
    LOG_I(TAG_MAIN, "Watchdog timer initialized (45 minutes)");

    // Setup custom log handler before initializing KNX
    setupCustomLogHandler();
    
    // Initialize BME280 sensor
    if (!bme280.begin()) {
        LOG_E(TAG_SENSOR, "Failed to initialize BME280 sensor!");
    }
    
    // Setup WiFi
    setupWiFi();
    
    // Initialize WebServerManager singleton
    WebServerManager* webServerManager = WebServerManager::getInstance();
    webServerManager->begin(&webServer);
    LOG_I(TAG_MAIN, "Web server started on port 80");
    
    // Setup KNX and MQTT managers
    knxManager.begin();
    mqttManager.begin();

    // Initialize OTA using WebServerManager
    otaManager.begin(webServerManager);
    LOG_I(TAG_MAIN, "OTA manager initialized with web server");
    
    // Connect the managers for cross-communication
    knxManager.setMQTTManager(&mqttManager);
    mqttManager.setKNXManager(&knxManager);
    
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
            LOG_E(TAG_WIFI, "WiFi disconnected for 30 minutes. Rebooting device...");
            delay(1000);
            ESP.restart();
        }
    }
}

void setupWiFi() {
    LOG_I(TAG_WIFI, "Setting up WiFi...");
    
    // Get credentials from config if available
    String storedSSID = configManager->getWifiSSID();
    String storedPass = configManager->getWifiPassword();
    
    // Only use stored credentials if both exist
    if (storedSSID.length() > 0 && storedPass.length() > 0) {
        LOG_I(TAG_WIFI, "Using stored WiFi credentials for SSID: %s", storedSSID.c_str());
        WiFi.begin(storedSSID.c_str(), storedPass.c_str());
        
        // Wait up to 10 seconds for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            LOG_I(TAG_WIFI, "Connected to WiFi using stored credentials");
            LOG_I(TAG_WIFI, "IP address: %s", WiFi.localIP().toString().c_str());
            return;
        }
        
        LOG_W(TAG_WIFI, "Failed to connect with stored credentials, using WiFiManager");
    }
    
    // Configure WiFiManager
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setConnectRetries(10); // Set to 10 reconnection attempts
    
    // Set callback to store credentials
    wifiManager.setSaveConfigCallback([]() {
        // Store WiFi credentials
        configManager->setWifiSSID(WiFi.SSID());
        configManager->setWifiPassword(WiFi.psk());
        LOG_I(TAG_WIFI, "WiFi credentials saved");
    });
    
    if (!wifiManager.autoConnect("ESP32-Thermostat-AP")) {
        LOG_E(TAG_WIFI, "Failed to connect and hit timeout");
        ESP.restart();
        delay(1000);
    }
    
    LOG_I(TAG_WIFI, "WiFi connected");
    LOG_I(TAG_WIFI, "IP address: %s", WiFi.localIP().toString().c_str());
    lastConnectedTime = millis(); // Update last connected time
}

void checkWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        // Update last connected time when WiFi is connected
        lastConnectedTime = millis();
        reconnectAttempts = 0;
    } else {
        LOG_W(TAG_WIFI, "WiFi connection lost. Attempting to reconnect...");
        
        // Increment reconnect attempts
        reconnectAttempts++;
        
        if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            // Try to reconnect using WiFi.begin() with saved credentials
            WiFi.disconnect();
            delay(1000);
            
            // Get credentials from config
            String storedSSID = configManager->getWifiSSID();
            String storedPass = configManager->getWifiPassword();
            
            if (storedSSID.length() > 0 && storedPass.length() > 0) {
                WiFi.begin(storedSSID.c_str(), storedPass.c_str());
            } else {
                WiFi.begin();
            }
            
            // Wait up to 10 seconds for connection
            unsigned long startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
                delay(500);
                Serial.print(".");
            }
            Serial.println();
            
            if (WiFi.status() == WL_CONNECTED) {
                LOG_I(TAG_WIFI, "Reconnected to WiFi");
                LOG_I(TAG_WIFI, "IP address: %s", WiFi.localIP().toString().c_str());
                lastConnectedTime = millis(); // Update last connected time
            } else {
                LOG_W(TAG_WIFI, "Reconnection attempt %d of %d failed", 
                      reconnectAttempts, MAX_RECONNECT_ATTEMPTS);
            }
        } else {
            // After several failed attempts, start config portal
            LOG_W(TAG_WIFI, "Multiple reconnection attempts failed. Starting config portal...");
            configPortalActive = true;
            wifiManager.startConfigPortal("ESP32-Thermostat-Recovery");
        }
        
        // Print time since last connection
        unsigned long disconnectedTime = (millis() - lastConnectedTime) / 1000;
        LOG_D(TAG_WIFI, "Time since last connection: %lu seconds", disconnectedTime);
        
        // Print time remaining before watchdog reboot
        if (disconnectedTime > 0) {
            unsigned long timeToReboot = (WIFI_WATCHDOG_TIMEOUT / 1000) - disconnectedTime;
            LOG_D(TAG_WIFI, "Device will reboot in %lu seconds if WiFi remains disconnected", 
                  timeToReboot);
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
    
    // Save current setpoint to config
    configManager->setSetpoint(g_pid_input.setpoint_temp);
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