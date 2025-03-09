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

ConfigManager::ConfigManager() {
    resetToDefaults();
}

ConfigManager::~ConfigManager() {
    end();
}

bool ConfigManager::begin() {
    if (!LittleFS.begin()) {
        log_e("Failed to mount LittleFS");
        return false;
    }
    return loadConfig();
}

void ConfigManager::end() {
    saveConfig();
}

bool ConfigManager::setupWiFi() {
    WiFi.mode(WIFI_STA);
    
    // Try to connect using saved credentials
    if (WiFi.begin() == WL_CONNECT_FAILED) {
        // If connection fails, start WiFi Manager
        AsyncWebServer server(80);
        DNSServer dns;
        AsyncWiFiManager wifiManager(&server, &dns);
        wifiManager.setConfigPortalTimeout(180); // 3 minutes timeout
        
        if (!wifiManager.autoConnect(deviceName)) {
            log_e("Failed to connect to WiFi - restarting");
            delay(1000);
            ESP.restart();
            return false;
        }
    }
    
    log_i("Connected to WiFi");
    log_i("IP address: %s", WiFi.localIP().toString().c_str());
    return true;
}

bool ConfigManager::loadConfig() {
    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        log_e("Failed to open config file");
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        log_e("Failed to parse config file");
        return false;
    }

    // Device settings
    strlcpy(deviceName, doc["deviceName"] | "ESP32-Thermostat", sizeof(deviceName));
    sendInterval = doc["sendInterval"] | 10000;
    pidInterval = doc["pidInterval"] | 10000;

    // Web interface settings
    strlcpy(webUsername, doc["webUsername"] | "admin", sizeof(webUsername));
    strlcpy(webPassword, doc["webPassword"] | "admin", sizeof(webPassword));

    // KNX settings
    knxEnabled = doc["knxEnabled"] | false;
    knxPhysicalArea = doc["knxPhysicalArea"] | 1;
    knxPhysicalLine = doc["knxPhysicalLine"] | 1;
    knxPhysicalMember = doc["knxPhysicalMember"] | 1;
    knxTempArea = doc["knxTempArea"] | 1;
    knxTempLine = doc["knxTempLine"] | 1;
    knxTempMember = doc["knxTempMember"] | 1;
    knxSetpointArea = doc["knxSetpointArea"] | 1;
    knxSetpointLine = doc["knxSetpointLine"] | 1;
    knxSetpointMember = doc["knxSetpointMember"] | 1;
    knxValveArea = doc["knxValveArea"] | 1;
    knxValveLine = doc["knxValveLine"] | 1;
    knxValveMember = doc["knxValveMember"] | 1;
    knxModeArea = doc["knxModeArea"] | 1;
    knxModeLine = doc["knxModeLine"] | 1;
    knxModeMember = doc["knxModeMember"] | 1;

    // MQTT settings
    mqttEnabled = doc["mqttEnabled"] | false;
    strlcpy(mqttServer, doc["mqttServer"] | "", sizeof(mqttServer));
    mqttPort = doc["mqttPort"] | 1883;
    strlcpy(mqttUser, doc["mqttUser"] | "", sizeof(mqttUser));
    strlcpy(mqttPassword, doc["mqttPassword"] | "", sizeof(mqttPassword));
    strlcpy(mqttClientId, doc["mqttClientId"] | "", sizeof(mqttClientId));

    // Thermostat settings
    setpoint = doc["setpoint"] | 21.0;

    return true;
}

void ConfigManager::saveConfig() {
    StaticJsonDocument<1024> doc;

    // Device settings
    doc["deviceName"] = deviceName;
    doc["sendInterval"] = sendInterval;
    doc["pidInterval"] = pidInterval;

    // Web interface settings
    doc["webUsername"] = webUsername;
    doc["webPassword"] = webPassword;

    // KNX settings
    doc["knxEnabled"] = knxEnabled;
    doc["knxPhysicalArea"] = knxPhysicalArea;
    doc["knxPhysicalLine"] = knxPhysicalLine;
    doc["knxPhysicalMember"] = knxPhysicalMember;
    doc["knxTempArea"] = knxTempArea;
    doc["knxTempLine"] = knxTempLine;
    doc["knxTempMember"] = knxTempMember;
    doc["knxSetpointArea"] = knxSetpointArea;
    doc["knxSetpointLine"] = knxSetpointLine;
    doc["knxSetpointMember"] = knxSetpointMember;
    doc["knxValveArea"] = knxValveArea;
    doc["knxValveLine"] = knxValveLine;
    doc["knxValveMember"] = knxValveMember;
    doc["knxModeArea"] = knxModeArea;
    doc["knxModeLine"] = knxModeLine;
    doc["knxModeMember"] = knxModeMember;

    // MQTT settings
    doc["mqttEnabled"] = mqttEnabled;
    doc["mqttServer"] = mqttServer;
    doc["mqttPort"] = mqttPort;
    doc["mqttUser"] = mqttUser;
    doc["mqttPassword"] = mqttPassword;
    doc["mqttClientId"] = mqttClientId;

    // Thermostat settings
    doc["setpoint"] = setpoint;

    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        log_e("Failed to open config file for writing");
        return;
    }

    if (serializeJson(doc, file) == 0) {
        log_e("Failed to write config file");
    }

    file.close();
}

void ConfigManager::resetToDefaults() {
    // Device settings
    strlcpy(deviceName, "ESP32-Thermostat", sizeof(deviceName));
    sendInterval = 10000;
    pidInterval = 10000;

    // Web interface settings
    strlcpy(webUsername, "admin", sizeof(webUsername));
    strlcpy(webPassword, "admin", sizeof(webPassword));

    // KNX settings
    knxEnabled = false;
    knxPhysicalArea = 1;
    knxPhysicalLine = 1;
    knxPhysicalMember = 1;
    knxTempArea = 1;
    knxTempLine = 1;
    knxTempMember = 1;
    knxSetpointArea = 1;
    knxSetpointLine = 1;
    knxSetpointMember = 1;
    knxValveArea = 1;
    knxValveLine = 1;
    knxValveMember = 1;
    knxModeArea = 1;
    knxModeLine = 1;
    knxModeMember = 1;

    // MQTT settings
    mqttEnabled = false;
    strlcpy(mqttServer, "", sizeof(mqttServer));
    mqttPort = 1883;
    strlcpy(mqttUser, "", sizeof(mqttUser));
    strlcpy(mqttPassword, "", sizeof(mqttPassword));
    strlcpy(mqttClientId, "", sizeof(mqttClientId));

    // Thermostat settings
    setpoint = 21.0;
}

// Device settings
void ConfigManager::setDeviceName(const char* name) {
    strlcpy(deviceName, name, sizeof(deviceName));
}

const char* ConfigManager::getDeviceName() const {
    return deviceName;
}

void ConfigManager::setSendInterval(uint32_t interval) {
    sendInterval = interval;
}

uint32_t ConfigManager::getSendInterval() const {
    return sendInterval;
}

void ConfigManager::setPidInterval(uint32_t interval) {
    pidInterval = interval;
}

uint32_t ConfigManager::getPidInterval() const {
    return pidInterval;
}

// Web interface settings
void ConfigManager::setWebUsername(const char* username) {
    strlcpy(webUsername, username, sizeof(webUsername));
}

void ConfigManager::setWebPassword(const char* password) {
    strlcpy(webPassword, password, sizeof(webPassword));
}

const char* ConfigManager::getWebUsername() const {
    return webUsername;
}

const char* ConfigManager::getWebPassword() const {
    return webPassword;
}

// KNX settings
void ConfigManager::setKnxEnabled(bool enabled) {
    knxEnabled = enabled;
}

bool ConfigManager::getKnxEnabled() const {
    return knxEnabled;
}

void ConfigManager::setKnxPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
    knxPhysicalArea = area;
    knxPhysicalLine = line;
    knxPhysicalMember = member;
}

void ConfigManager::getKnxPhysicalAddress(uint8_t& area, uint8_t& line, uint8_t& member) const {
    area = knxPhysicalArea;
    line = knxPhysicalLine;
    member = knxPhysicalMember;
}

uint8_t ConfigManager::getKnxPhysicalArea() const {
    return knxPhysicalArea;
}

uint8_t ConfigManager::getKnxPhysicalLine() const {
    return knxPhysicalLine;
}

uint8_t ConfigManager::getKnxPhysicalMember() const {
    return knxPhysicalMember;
}

void ConfigManager::setKnxTemperatureGA(uint8_t area, uint8_t line, uint8_t member) {
    knxTempArea = area;
    knxTempLine = line;
    knxTempMember = member;
}

void ConfigManager::getKnxTemperatureGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
    area = knxTempArea;
    line = knxTempLine;
    member = knxTempMember;
}

uint8_t ConfigManager::getKnxTempArea() const {
    return knxTempArea;
}

uint8_t ConfigManager::getKnxTempLine() const {
    return knxTempLine;
}

uint8_t ConfigManager::getKnxTempMember() const {
    return knxTempMember;
}

void ConfigManager::setKnxSetpointGA(uint8_t area, uint8_t line, uint8_t member) {
    knxSetpointArea = area;
    knxSetpointLine = line;
    knxSetpointMember = member;
}

void ConfigManager::getKnxSetpointGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
    area = knxSetpointArea;
    line = knxSetpointLine;
    member = knxSetpointMember;
}

uint8_t ConfigManager::getKnxSetpointArea() const {
    return knxSetpointArea;
}

uint8_t ConfigManager::getKnxSetpointLine() const {
    return knxSetpointLine;
}

uint8_t ConfigManager::getKnxSetpointMember() const {
    return knxSetpointMember;
}

void ConfigManager::setKnxValveGA(uint8_t area, uint8_t line, uint8_t member) {
    knxValveArea = area;
    knxValveLine = line;
    knxValveMember = member;
}

void ConfigManager::getKnxValveGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
    area = knxValveArea;
    line = knxValveLine;
    member = knxValveMember;
}

uint8_t ConfigManager::getKnxValveArea() const {
    return knxValveArea;
}

uint8_t ConfigManager::getKnxValveLine() const {
    return knxValveLine;
}

uint8_t ConfigManager::getKnxValveMember() const {
    return knxValveMember;
}

void ConfigManager::setKnxModeGA(uint8_t area, uint8_t line, uint8_t member) {
    knxModeArea = area;
    knxModeLine = line;
    knxModeMember = member;
}

void ConfigManager::getKnxModeGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
    area = knxModeArea;
    line = knxModeLine;
    member = knxModeMember;
}

uint8_t ConfigManager::getKnxModeArea() const {
    return knxModeArea;
}

uint8_t ConfigManager::getKnxModeLine() const {
    return knxModeLine;
}

uint8_t ConfigManager::getKnxModeMember() const {
    return knxModeMember;
}

// MQTT settings
void ConfigManager::setMQTTEnabled(bool enabled) {
    mqttEnabled = enabled;
}

bool ConfigManager::getMQTTEnabled() const {
    return mqttEnabled;
}

void ConfigManager::setMQTTServer(const char* server) {
    strlcpy(mqttServer, server, sizeof(mqttServer));
}

const char* ConfigManager::getMQTTServer() const {
    return mqttServer;
}

void ConfigManager::setMQTTPort(uint16_t port) {
    mqttPort = port;
}

uint16_t ConfigManager::getMQTTPort() const {
    return mqttPort;
}

void ConfigManager::setMQTTUser(const char* user) {
    strlcpy(mqttUser, user, sizeof(mqttUser));
}

const char* ConfigManager::getMQTTUser() const {
    return mqttUser;
}

void ConfigManager::setMQTTPassword(const char* password) {
    strlcpy(mqttPassword, password, sizeof(mqttPassword));
}

const char* ConfigManager::getMQTTPassword() const {
    return mqttPassword;
}

void ConfigManager::setMQTTClientId(const char* clientId) {
    strlcpy(mqttClientId, clientId, sizeof(mqttClientId));
}

const char* ConfigManager::getMQTTClientId() const {
    return mqttClientId;
}

// Thermostat settings
void ConfigManager::setSetpoint(float value) {
    setpoint = value;
}

float ConfigManager::getSetpoint() const {
    return setpoint;
}