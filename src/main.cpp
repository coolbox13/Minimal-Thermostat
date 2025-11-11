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
#include "history_manager.h"
#include "ntp_manager.h"

/**
 * LOGGING TAG NAMING STANDARD:
 *
 * Single-purpose files: Use `static const char* TAG = "MODULE"`
 * Examples:
 *   - config_manager.cpp: TAG = "CONFIG"
 *   - knx_manager.cpp: TAG = "KNX"
 *   - adaptive_pid_controller.cpp: TAG = "PID"
 *
 * Multi-purpose files: Use `static const char* TAG_PURPOSE = "PURPOSE"`
 * Example (main.cpp handles multiple concerns):
 */
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
    
    // Initialize NTP manager with config values
    NTPManager& ntpManager = NTPManager::getInstance();
    String ntpServer = configManager->getNtpServer();
    int timezoneOffset = configManager->getNtpTimezoneOffset();
    int daylightOffset = configManager->getNtpDaylightOffset();
    ntpManager.begin(ntpServer.c_str(), timezoneOffset, daylightOffset);
    
    WiFiConnectionManager& wifiManager = WiFiConnectionManager::getInstance();
    wifiManager.registerEventCallback([&wifiManager, &ntpManager](const WiFiConnectionEvent& event) {
        LOG_I(TAG_WIFI, "WiFi event: %s - %s",
              wifiManager.getEventTypeName(event.type),
              event.message.c_str());
        if (event.type == WiFiEventType::CONNECTED) {
            LOG_I(TAG_WIFI, "Connected to: %s", event.ssid.c_str());
            LOG_I(TAG_WIFI, "IP address: %s", event.networkInfo.ip.toString().c_str());
            
            // Sync time with NTP server after WiFi connection
            LOG_I(TAG_WIFI, "Synchronizing time with NTP server...");
            if (ntpManager.syncTime(NTP_SYNC_TIMEOUT_MS)) {
                String timeStr = ntpManager.getFormattedTime();
                LOG_I(TAG_WIFI, "Time synchronized: %s", timeStr.c_str());
            } else {
                LOG_W(TAG_WIFI, "NTP time synchronization failed");
            }
        }
    });
    bool wifiConnected = wifiManager.begin(configManager->getWifiConnectTimeout(), true);
    if (!wifiConnected) {
        LOG_W(TAG_WIFI, "WiFi connection failed or timed out during setup");
    } else {
        // If WiFi is already connected, sync time immediately
        if (WiFi.status() == WL_CONNECTED) {
            LOG_I(TAG_WIFI, "WiFi already connected, syncing time...");
            if (ntpManager.syncTime(NTP_SYNC_TIMEOUT_MS)) {
                String timeStr = ntpManager.getFormattedTime();
                LOG_I(TAG_WIFI, "Time synchronized: %s", timeStr.c_str());
            } else {
                LOG_W(TAG_WIFI, "NTP time synchronization failed");
            }
        }
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

    // HA MQTT FIX: Publish initial PID parameters after MQTT connection
    // This ensures PID sensors don't show "Unknown" in Home Assistant
    float kp = configManager->getPidKp();
    float ki = configManager->getPidKi();
    float kd = configManager->getPidKd();
    mqttManager.updatePIDParameters(kp, ki, kd);
    LOG_D(TAG_MQTT, "Published initial PID parameters: Kp=%.3f, Ki=%.3f, Kd=%.3f", kp, ki, kd);
}
void initializePID() {
    initializePIDController();
    float setpoint = configManager->getSetpoint();
    setTemperatureSetpoint(setpoint);
    LOG_I(TAG_PID, "PID controller initialized with setpoint: %.2f°C", setpoint);
}
void performInitialSetup() {
    // Log comprehensive memory and flash information
    LOG_I(TAG_MAIN, "========== MEMORY & FLASH INFORMATION ==========");

    // RAM Information
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    float heapPercent = (float)usedHeap / totalHeap * 100.0f;
    LOG_I(TAG_MAIN, "RAM: %u KB free / %u KB total (%.1f%% used)",
          freeHeap / 1024, totalHeap / 1024, heapPercent);

    // Flash Information
    uint32_t flashSize = ESP.getFlashChipSize();
    uint32_t freeFlash = ESP.getFreeSketchSpace();
    uint32_t usedFlash = ESP.getSketchSize();
    uint32_t otaPartition = freeFlash + usedFlash;
    float flashPercent = (float)usedFlash / otaPartition * 100.0f;

    LOG_I(TAG_MAIN, "Flash Total: %u KB (%u MB)", flashSize / 1024, flashSize / 1024 / 1024);
    LOG_I(TAG_MAIN, "OTA Partition: %u KB total", otaPartition / 1024);
    LOG_I(TAG_MAIN, "OTA Usage: %u KB used / %u KB free (%.1f%% used)",
          usedFlash / 1024, freeFlash / 1024, flashPercent);

    if (flashPercent > 95.0f) {
        LOG_W(TAG_MAIN, "WARNING: Flash usage critically high (>95%%)!");
    } else if (flashPercent > 90.0f) {
        LOG_W(TAG_MAIN, "CAUTION: Flash usage high (>90%%)");
    }

    // SPIFFS Information
    uint32_t spiffsTotal = SPIFFS.totalBytes();
    uint32_t spiffsUsed = SPIFFS.usedBytes();
    uint32_t spiffsFree = spiffsTotal - spiffsUsed;
    float spiffsPercent = (float)spiffsUsed / spiffsTotal * 100.0f;
    LOG_I(TAG_MAIN, "SPIFFS: %u KB used / %u KB total (%.1f%% used)",
          spiffsUsed / 1024, spiffsTotal / 1024, spiffsPercent);
    LOG_I(TAG_MAIN, "SPIFFS Free: %u KB (%.1f%% available)",
          spiffsFree / 1024, (float)spiffsFree / spiffsTotal * 100.0f);

    LOG_I(TAG_MAIN, "Chip: %s Rev %d @ %d MHz",
          ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz());
    LOG_I(TAG_MAIN, "==============================================");

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

    // HA MQTT FIX: Update diagnostics every 60 seconds for Home Assistant
    static unsigned long lastDiagnosticsUpdate = 0;
    if (millis() - lastDiagnosticsUpdate > 60000) {
        int rssi = WiFi.RSSI();
        unsigned long uptime = millis() / 1000;
        mqttManager.updateDiagnostics(rssi, uptime);
        lastDiagnosticsUpdate = millis();
        LOG_D(TAG_MQTT, "Published diagnostics: RSSI=%d dBm, Uptime=%lu s", rssi, uptime);
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

    // Add data point to history for graphing
    HistoryManager* historyManager = HistoryManager::getInstance();
    historyManager->addDataPoint(temperature, humidity, pressure, knxManager.getValvePosition());

    // Send sensor data to KNX
    knxManager.sendSensorData(temperature, humidity, pressure);

    // Publish sensor data to MQTT
    mqttManager.publishSensorData(temperature, humidity, pressure);
}

// Modified updatePIDControl function for main.cpp
void updatePIDControl() {
    ConfigManager* configManager = ConfigManager::getInstance();

    // Get current temperature from BME280
    float currentTemp = bme280.readTemperature();

    // CRITICAL FIX: Validate sensor reading before processing (Audit Fix #1)
    // Reject NaN, infinity, and values outside physically possible range
    if (isnan(currentTemp) || isinf(currentTemp) || currentTemp < -40.0f || currentTemp > 85.0f) {
        LOG_E(TAG_PID, "Invalid sensor reading: %.2f°C - skipping PID update", currentTemp);
        // Skip this control cycle to prevent feeding bad data to PID
        // Valve position remains unchanged from last valid cycle
        return;
    }

    // Get current valve position from KNX (feedback)
    float valvePosition = knxManager.getValvePosition();

    // Check manual override timeout
    if (configManager->getManualOverrideEnabled()) {
        uint32_t timeout = configManager->getManualOverrideTimeout();
        unsigned long activationTime = configManager->getManualOverrideActivationTime();

        // HIGH PRIORITY FIX: Use overflow-safe elapsed time calculation (Audit Fix #4)
        // millis() wraps around after ~49 days, but subtraction handles it correctly
        unsigned long elapsed = millis() - activationTime; // Handles overflow correctly

        // If timeout is set (> 0) and has expired, disable manual override
        if (timeout > 0 && (elapsed / 1000) > timeout) {
            LOG_I(TAG_PID, "Manual override timeout expired after %lu seconds, disabling", elapsed / 1000);
            configManager->setManualOverrideEnabled(false);
        }
    }

    // Determine valve position based on manual override or PID
    float finalValvePosition;
    if (configManager->getManualOverrideEnabled()) {
        // Use manual override position
        finalValvePosition = configManager->getManualOverridePosition();
        LOG_D(TAG_PID, "Manual override active: %.1f%%", finalValvePosition);
    } else {
        // Update PID controller
        updatePIDController(currentTemp, valvePosition);

        // Get new valve position from PID
        finalValvePosition = getPIDOutput();

        // Log PID information
        LOG_D(TAG_PID, "PID controller updated:");
        LOG_D(TAG_PID, "Temperature: %.2f°C, Setpoint: %.2f°C",
              currentTemp, g_pid_input.setpoint_temp);
        LOG_D(TAG_PID, "Valve position: %.1f%%", finalValvePosition);
        LOG_D(TAG_PID, "PID params - Kp: %.3f, Ki: %.3f, Kd: %.3f",
              g_pid_input.Kp, g_pid_input.Ki, g_pid_input.Kd);
    }

    // Apply final valve position to KNX
    knxManager.setValvePosition(finalValvePosition);
    
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
    // Audit Fix #4: Overflow-safe interval check
    unsigned long configWriteElapsed = millis() - lastConfigWrite;
    if (pendingConfigWrite && (configWriteElapsed > configManager->getPidConfigWriteInterval())) {
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

        // HA MQTT FIX: Publish PID parameters to Home Assistant after saving
        mqttManager.updatePIDParameters(g_pid_input.Kp, g_pid_input.Ki, g_pid_input.Kd);
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