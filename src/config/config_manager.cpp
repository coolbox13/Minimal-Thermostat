// Basic includes
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>

// Include the ThermostatState first since other components depend on it
#include "thermostat_state.h"

// Then include your component headers
#include "communication/knx/knx_interface.h"
#include "config_manager.h"
#include <ESPAsyncWiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <esp_log.h>
#include "pid_controller.h"
#include "protocol_types.h"
#include "communication/mqtt/mqtt_interface.h"

static const char* TAG = "ConfigManager";

ConfigManager::ConfigManager() {
    // Initialize default values
    strlcpy(deviceName, "ESP32 Thermostat", sizeof(deviceName));
    
    // Web interface defaults
    strlcpy(webUsername, "admin", sizeof(webUsername));
    strlcpy(webPassword, "admin", sizeof(webPassword));
    
    // KNX defaults
    knxEnabled = false;
    knxPhysicalAddress = {1, 1, 1};
    
    // MQTT defaults
    mqttEnabled = false;
    strlcpy(mqttServer, "localhost", sizeof(mqttServer));
    mqttPort = 1883;
    strlcpy(mqttUser, "", sizeof(mqttUser));
    strlcpy(mqttPassword, "", sizeof(mqttPassword));
    strlcpy(mqttClientId, "esp32_thermostat", sizeof(mqttClientId));
    strlcpy(mqttTopicPrefix, "esp32/thermostat/", sizeof(mqttTopicPrefix));
    
    // PID defaults
    pidConfig = {
        .kp = 1.0f,
        .ki = 0.1f,
        .kd = 0.05f,
        .outputMin = 0.0f,
        .outputMax = 100.0f,
        .sampleTime = 1000
    };
}

bool ConfigManager::begin() {
    if (!LittleFS.begin(true)) {
        ESP_LOGE(TAG, "Failed to mount file system");
        return false;
    }
    return loadConfig();
}

bool ConfigManager::loadConfig() {
    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        ESP_LOGW(TAG, "Failed to open config file, using defaults");
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        ESP_LOGE(TAG, "Failed to parse config file: %s", error.c_str());
        return false;
    }

    // Load device settings
    if (doc.containsKey("deviceName")) {
        strlcpy(deviceName, doc["deviceName"] | "ESP32 Thermostat", sizeof(deviceName));
    }

    // Load web interface settings
    JsonObject web = doc["web"];
    if (web) {
        strlcpy(webUsername, web["username"] | "admin", sizeof(webUsername));
        strlcpy(webPassword, web["password"] | "admin", sizeof(webPassword));
    }

    // Load KNX settings
    JsonObject knx = doc["knx"];
    if (knx) {
        knxEnabled = knx["enabled"] | false;
        JsonObject physical = knx["physical"];
        if (physical) {
            knxPhysicalAddress.area = physical["area"] | 1;
            knxPhysicalAddress.line = physical["line"] | 1;
            knxPhysicalAddress.member = physical["member"] | 1;
        }
    }

    // Load MQTT settings
    JsonObject mqtt = doc["mqtt"];
    if (mqtt) {
        mqttEnabled = mqtt["enabled"] | false;
        strlcpy(mqttServer, mqtt["server"] | "localhost", sizeof(mqttServer));
        mqttPort = mqtt["port"] | 1883;
        strlcpy(mqttUser, mqtt["username"] | "", sizeof(mqttUser));
        strlcpy(mqttPassword, mqtt["password"] | "", sizeof(mqttPassword));
        strlcpy(mqttClientId, mqtt["clientId"] | "esp32_thermostat", sizeof(mqttClientId));
        strlcpy(mqttTopicPrefix, mqtt["topicPrefix"] | "esp32/thermostat/", sizeof(mqttTopicPrefix));
    }

    // Load PID settings
    JsonObject pid = doc["pid"];
    if (pid) {
        pidConfig.kp = pid["kp"] | 1.0f;
        pidConfig.ki = pid["ki"] | 0.1f;
        pidConfig.kd = pid["kd"] | 0.05f;
        pidConfig.outputMin = pid["outputMin"] | 0.0f;
        pidConfig.outputMax = pid["outputMax"] | 100.0f;
        pidConfig.sampleTime = pid["sampleTime"] | 1000;
    }

    return true;
}

void ConfigManager::saveConfig() {
    StaticJsonDocument<1024> doc;

    // Save device settings
    doc["deviceName"] = deviceName;

    // Save web interface settings
    JsonObject web = doc.createNestedObject("web");
    web["username"] = webUsername;
    web["password"] = webPassword;

    // Save KNX settings
    JsonObject knx = doc.createNestedObject("knx");
    knx["enabled"] = knxEnabled;
    JsonObject physical = knx.createNestedObject("physical");
    physical["area"] = knxPhysicalAddress.area;
    physical["line"] = knxPhysicalAddress.line;
    physical["member"] = knxPhysicalAddress.member;

    // Save MQTT settings
    JsonObject mqtt = doc.createNestedObject("mqtt");
    mqtt["enabled"] = mqttEnabled;
    mqtt["server"] = mqttServer;
    mqtt["port"] = mqttPort;
    mqtt["username"] = mqttUser;
    mqtt["password"] = mqttPassword;
    mqtt["clientId"] = mqttClientId;
    mqtt["topicPrefix"] = mqttTopicPrefix;

    // Save PID settings
    JsonObject pid = doc.createNestedObject("pid");
    pid["kp"] = pidConfig.kp;
    pid["ki"] = pidConfig.ki;
    pid["kd"] = pidConfig.kd;
    pid["outputMin"] = pidConfig.outputMin;
    pid["outputMax"] = pidConfig.outputMax;
    pid["sampleTime"] = pidConfig.sampleTime;

    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open config file for writing");
        return;
    }

    if (serializeJson(doc, file) == 0) {
        ESP_LOGE(TAG, "Failed to write config file");
    }
    file.close();
}

void ConfigManager::resetToDefaults() {
    // Reset device settings
    strlcpy(deviceName, "ESP32 Thermostat", sizeof(deviceName));
    
    // Reset web interface settings
    strlcpy(webUsername, "admin", sizeof(webUsername));
    strlcpy(webPassword, "admin", sizeof(webPassword));
    
    // Reset KNX settings
    knxEnabled = false;
    knxPhysicalAddress = {1, 1, 1};
    
    // Reset MQTT settings
    mqttEnabled = false;
    strlcpy(mqttServer, "localhost", sizeof(mqttServer));
    mqttPort = 1883;
    strlcpy(mqttUser, "", sizeof(mqttUser));
    strlcpy(mqttPassword, "", sizeof(mqttPassword));
    strlcpy(mqttClientId, "esp32_thermostat", sizeof(mqttClientId));
    strlcpy(mqttTopicPrefix, "esp32/thermostat/", sizeof(mqttTopicPrefix));
    
    // Reset PID settings
    pidConfig = {
        .kp = 1.0f,
        .ki = 0.1f,
        .kd = 0.05f,
        .outputMin = 0.0f,
        .outputMax = 100.0f,
        .sampleTime = 1000
    };
    
    saveConfig();
}

void ConfigManager::setDeviceName(const char* name) {
    strlcpy(deviceName, name, sizeof(deviceName));
}

void ConfigManager::setWebUsername(const char* username) {
    strlcpy(webUsername, username, sizeof(webUsername));
}

void ConfigManager::setWebPassword(const char* password) {
    strlcpy(webPassword, password, sizeof(webPassword));
}

void ConfigManager::setMQTTServer(const char* server) {
    strlcpy(mqttServer, server, sizeof(mqttServer));
}

void ConfigManager::setMQTTUser(const char* user) {
    strlcpy(mqttUser, user, sizeof(mqttUser));
}

void ConfigManager::setMQTTPassword(const char* password) {
    strlcpy(mqttPassword, password, sizeof(mqttPassword));
}

void ConfigManager::setMQTTClientId(const char* clientId) {
    strlcpy(mqttClientId, clientId, sizeof(mqttClientId));
}

void ConfigManager::setMQTTTopicPrefix(const char* prefix) {
    strlcpy(mqttTopicPrefix, prefix, sizeof(mqttTopicPrefix));
}

void ConfigManager::setKnxPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
    knxPhysicalAddress.area = area;
    knxPhysicalAddress.line = line;
    knxPhysicalAddress.member = member;
}

void ConfigManager::getKnxPhysicalAddress(uint8_t& area, uint8_t& line, uint8_t& member) const {
    area = knxPhysicalAddress.area;
    line = knxPhysicalAddress.line;
    member = knxPhysicalAddress.member;
}

uint8_t ConfigManager::getKnxPhysicalArea() const {
    return knxPhysicalAddress.area;
}

uint8_t ConfigManager::getKnxPhysicalLine() const {
    return knxPhysicalAddress.line;
}

uint8_t ConfigManager::getKnxPhysicalMember() const {
    return knxPhysicalAddress.member;
}

const char* ConfigManager::getDeviceName() const {
    return deviceName;
}

const char* ConfigManager::getWebUsername() const {
    return webUsername;
}

const char* ConfigManager::getWebPassword() const {
    return webPassword;
}

bool ConfigManager::getKnxEnabled() const {
    return knxEnabled;
}

void ConfigManager::setKnxEnabled(bool enabled) {
    knxEnabled = enabled;
}

bool ConfigManager::getMQTTEnabled() const {
    return mqttEnabled;
}

void ConfigManager::setMQTTEnabled(bool enabled) {
    mqttEnabled = enabled;
}

const char* ConfigManager::getMQTTServer() const {
    return mqttServer;
}

uint16_t ConfigManager::getMQTTPort() const {
    return mqttPort;
}

void ConfigManager::setMQTTPort(uint16_t port) {
    mqttPort = port;
}

const char* ConfigManager::getMQTTUser() const {
    return mqttUser;
}

const char* ConfigManager::getMQTTPassword() const {
    return mqttPassword;
}

const char* ConfigManager::getMQTTClientId() const {
    return mqttClientId;
}

const char* ConfigManager::getMQTTTopicPrefix() const {
    return mqttTopicPrefix;
}

const PIDConfig& ConfigManager::getPidConfig() const {
    return pidConfig;
}

void ConfigManager::setPidConfig(const PIDConfig& config) {
    pidConfig = config;
}