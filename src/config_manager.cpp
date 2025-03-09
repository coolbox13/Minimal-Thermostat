#include "config_manager.h"
#include <ArduinoJson.h>

ConfigManager::ConfigManager() {
    setDefaults();
}

bool ConfigManager::begin() {
    // Initialize file system
    if (!FileFS.begin()) {
        lastError = ThermostatStatus::FILESYSTEM_ERROR;
        return false;
    }
    
    // Try to load configuration
    if (!load()) {
        Serial.println("Using default configuration");
        save(); // Save default configuration
    }
    
    lastError = ThermostatStatus::OK;
    return true;
}

bool ConfigManager::load() {
    Serial.println("Loading configuration...");
    if (!FileFS.exists(CONFIG_FILE)) {
        lastError = ThermostatStatus::FILE_NOT_FOUND;
        return false;
    }
    
    File configFile = FileFS.open(CONFIG_FILE, "r");
    if (!configFile) {
        lastError = ThermostatStatus::FILE_READ_ERROR;
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        lastError = ThermostatStatus::JSON_PARSE_ERROR;
        return false;
    }
    
    // Device settings
    strlcpy(deviceName, doc["deviceName"] | DEFAULT_DEVICE_NAME, sizeof(deviceName));
    sendInterval = doc["sendInterval"] | 10000;
    strlcpy(webUsername, doc["webUsername"] | "admin", sizeof(webUsername));
    strlcpy(webPassword, doc["webPassword"] | "admin", sizeof(webPassword));
    
    // KNX settings
    knxEnabled = doc["knxEnabled"] | false;
    knxPhysicalArea = doc["knxPhysicalArea"] | 1;
    knxPhysicalLine = doc["knxPhysicalLine"] | 1;
    knxPhysicalMember = doc["knxPhysicalMember"] | 201;
    
    // MQTT settings
    mqttEnabled = doc["mqttEnabled"] | false;
    strlcpy(mqttServer, doc["mqttServer"] | "192.168.1.100", sizeof(mqttServer));
    mqttPort = doc["mqttPort"] | 1883;
    strlcpy(mqttUser, doc["mqttUser"] | "", sizeof(mqttUser));
    strlcpy(mqttPassword, doc["mqttPassword"] | "", sizeof(mqttPassword));
    strlcpy(mqttClientId, doc["mqttClientId"] | "ESP32Thermostat", sizeof(mqttClientId));
    
    // PID settings
    kp = doc["kp"] | 1.0;
    ki = doc["ki"] | 0.1;
    kd = doc["kd"] | 0.01;
    setpoint = doc["setpoint"] | 21.0;
    
    lastError = ThermostatStatus::OK;
    return true;
}

bool ConfigManager::save() {
    DynamicJsonDocument doc(1024);
    
    // Device settings
    doc["deviceName"] = deviceName;
    doc["sendInterval"] = sendInterval;
    doc["webUsername"] = webUsername;
    doc["webPassword"] = webPassword;
    
    // KNX settings
    doc["knxEnabled"] = knxEnabled;
    doc["knxPhysicalArea"] = knxPhysicalArea;
    doc["knxPhysicalLine"] = knxPhysicalLine;
    doc["knxPhysicalMember"] = knxPhysicalMember;
    
    // MQTT settings
    doc["mqttEnabled"] = mqttEnabled;
    doc["mqttServer"] = mqttServer;
    doc["mqttPort"] = mqttPort;
    doc["mqttUser"] = mqttUser;
    doc["mqttPassword"] = mqttPassword;
    doc["mqttClientId"] = mqttClientId;
    
    // PID settings
    doc["kp"] = kp;
    doc["ki"] = ki;
    doc["kd"] = kd;
    doc["setpoint"] = setpoint;
    
    File configFile = FileFS.open(CONFIG_FILE, "w");
    if (!configFile) {
        lastError = ThermostatStatus::FILE_WRITE_ERROR;
        return false;
    }
    
    if (serializeJson(doc, configFile) == 0) {
        lastError = ThermostatStatus::JSON_WRITE_ERROR;
        return false;
    }
    
    configFile.close();
    lastError = ThermostatStatus::OK;
    return true;
}

void ConfigManager::reset() {
    setDefaults();
    save();
}

void ConfigManager::setDefaults() {
    strlcpy(deviceName, DEFAULT_DEVICE_NAME, sizeof(deviceName));
    sendInterval = 10000;
    strlcpy(webUsername, "admin", sizeof(webUsername));
    strlcpy(webPassword, "admin", sizeof(webPassword));
    
    knxEnabled = false;
    knxPhysicalArea = 1;
    knxPhysicalLine = 1;
    knxPhysicalMember = 201;
    
    mqttEnabled = false;
    strlcpy(mqttServer, "192.168.1.100", sizeof(mqttServer));
    mqttPort = 1883;
    mqttUser[0] = '\0';
    mqttPassword[0] = '\0';
    strlcpy(mqttClientId, "ESP32Thermostat", sizeof(mqttClientId));
    
    kp = 1.0;
    ki = 0.1;
    kd = 0.01;
    setpoint = 21.0;
    
    lastError = ThermostatStatus::OK;
}

// Getters
const char* ConfigManager::getDeviceName() const { return deviceName; }
unsigned long ConfigManager::getSendInterval() const { return sendInterval; }
const char* ConfigManager::getWebUsername() const { return webUsername; }
const char* ConfigManager::getWebPassword() const { return webPassword; }
float ConfigManager::getKp() const { return kp; }
float ConfigManager::getKi() const { return ki; }
float ConfigManager::getKd() const { return kd; }
float ConfigManager::getSetpoint() const { return setpoint; }
bool ConfigManager::getKnxEnabled() const { return knxEnabled; }
bool ConfigManager::getMqttEnabled() const { return mqttEnabled; }
ThermostatStatus ConfigManager::getLastError() const { return lastError; }

// Setters
void ConfigManager::setDeviceName(const char* name) {
    strlcpy(deviceName, name, sizeof(deviceName));
}

void ConfigManager::setSendInterval(unsigned long interval) {
    sendInterval = interval;
}

void ConfigManager::setWebUsername(const char* username) {
    strlcpy(webUsername, username, sizeof(webUsername));
}

void ConfigManager::setWebPassword(const char* password) {
    strlcpy(webPassword, password, sizeof(webPassword));
}

void ConfigManager::setKp(float value) {
    kp = value;
}

void ConfigManager::setKi(float value) {
    ki = value;
}

void ConfigManager::setKd(float value) {
    kd = value;
}

void ConfigManager::setSetpoint(float value) {
    setpoint = value;
}

void ConfigManager::setKnxEnabled(bool enabled) {
    knxEnabled = enabled;
}

void ConfigManager::setMqttEnabled(bool enabled) {
    mqttEnabled = enabled;
} 