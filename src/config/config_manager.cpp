// Basic includes
#include <Arduino.h>

// Include the ThermostatState first since other components depend on it
#include "thermostat_state.h"

// Then include your component headers
#include "communication/knx/knx_interface.h"
#include "config_manager.h"
#include <ESPAsyncWiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

ConfigManager::ConfigManager() {
    loadConfig();
}

ConfigManager::~ConfigManager() {
    // Nothing to clean up
}

bool ConfigManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    return true;
}

void ConfigManager::end() {
    // Nothing to clean up
}

bool ConfigManager::setupWiFi() {
    AsyncWebServer server(80);
    DNSServer dns;
    AsyncWiFiManager wifiManager(&server, &dns);
    
    wifiManager.setConfigPortalTimeout(180);
    
    if (!wifiManager.autoConnect("ESP32-Thermostat")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
        return false;
    }
    
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void ConfigManager::loadConfig() {
    if (!LittleFS.exists("/config.json")) {
        resetToDefaults();
        return;
    }

    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to parse config file");
        return;
    }

    // Load MQTT settings
    strlcpy(mqttServer, doc["mqtt_server"] | "mqtt.local", sizeof(mqttServer));
    mqttPort = doc["mqtt_port"] | 1883;
    strlcpy(mqttUser, doc["mqtt_user"] | "", sizeof(mqttUser));
    strlcpy(mqttPassword, doc["mqtt_pass"] | "", sizeof(mqttPassword));

    // Load thermostat settings
    targetTemp = doc["target_temp"] | 21.0;
    hysteresis = doc["hysteresis"] | 0.5;
    mode = static_cast<ThermostatMode>(doc["mode"].as<int>());

    // Load PID settings
    pidKp = doc["pid_kp"] | 2.0;
    pidKi = doc["pid_ki"] | 5.0;
    pidKd = doc["pid_kd"] | 1.0;
}

void ConfigManager::saveConfig() {
    StaticJsonDocument<1024> doc;

    // Save MQTT settings
    doc["mqtt_server"] = mqttServer;
    doc["mqtt_port"] = mqttPort;
    doc["mqtt_user"] = mqttUser;
    doc["mqtt_pass"] = mqttPassword;

    // Save thermostat settings
    doc["target_temp"] = targetTemp;
    doc["hysteresis"] = hysteresis;
    doc["mode"] = static_cast<int>(mode);

    // Save PID settings
    doc["pid_kp"] = pidKp;
    doc["pid_ki"] = pidKi;
    doc["pid_kd"] = pidKd;

    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write config file");
    }
    file.close();
}

void ConfigManager::resetToDefaults() {
    // Reset MQTT settings
    strlcpy(mqttServer, "mqtt.local", sizeof(mqttServer));
    mqttPort = 1883;
    mqttUser[0] = '\0';
    mqttPassword[0] = '\0';

    // Reset thermostat settings
    targetTemp = 21.0;
    hysteresis = 0.5;
    mode = ThermostatMode::OFF;

    // Reset PID settings
    pidKp = 2.0;
    pidKi = 5.0;
    pidKd = 1.0;

    saveConfig();
}

// MQTT getters and setters
const char* ConfigManager::getMQTTServer() const { return mqttServer; }
uint16_t ConfigManager::getMQTTPort() const { return mqttPort; }
const char* ConfigManager::getMQTTUser() const { return mqttUser; }
const char* ConfigManager::getMQTTPassword() const { return mqttPassword; }

void ConfigManager::setMQTTServer(const char* server) { strlcpy(mqttServer, server, sizeof(mqttServer)); }
void ConfigManager::setMQTTPort(uint16_t port) { mqttPort = port; }
void ConfigManager::setMQTTUser(const char* user) { strlcpy(mqttUser, user, sizeof(mqttUser)); }
void ConfigManager::setMQTTPassword(const char* password) { strlcpy(mqttPassword, password, sizeof(mqttPassword)); }

// Thermostat getters and setters
float ConfigManager::getTargetTemp() const { return targetTemp; }
float ConfigManager::getHysteresis() const { return hysteresis; }
ThermostatMode ConfigManager::getMode() const { return mode; }

void ConfigManager::setTargetTemp(float temp) { targetTemp = temp; }
void ConfigManager::setHysteresis(float hyst) { hysteresis = hyst; }
void ConfigManager::setMode(ThermostatMode m) { mode = m; }

// Setters
void ConfigManager::setDeviceName(const char* name) {
  strlcpy(deviceName, name, sizeof(deviceName));
}

void ConfigManager::setSendInterval(uint32_t interval) {
  sendInterval = interval;
}

void ConfigManager::setPidInterval(uint32_t interval) {
  pidInterval = interval;
}

void ConfigManager::setKnxPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
  knxPhysicalArea = area;
  knxPhysicalLine = line;
  knxPhysicalMember = member;
}

void ConfigManager::setKnxTemperatureGA(uint8_t area, uint8_t line, uint8_t member) {
  knxTempArea = area;
  knxTempLine = line;
  knxTempMember = member;
}

void ConfigManager::setKnxSetpointGA(uint8_t area, uint8_t line, uint8_t member) {
  knxSetpointArea = area;
  knxSetpointLine = line;
  knxSetpointMember = member;
}

void ConfigManager::setKnxValveGA(uint8_t area, uint8_t line, uint8_t member) {
  knxValveArea = area;
  knxValveLine = line;
  knxValveMember = member;
}

void ConfigManager::setKnxModeGA(uint8_t area, uint8_t line, uint8_t member) {
  knxModeArea = area;
  knxModeLine = line;
  knxModeMember = member;
}

void ConfigManager::setKnxEnabled(bool enabled) {
  knxEnabled = enabled;
}

void ConfigManager::setMqttEnabled(bool enabled) {
  mqttEnabled = enabled;
}

void ConfigManager::setKp(float value) {
  pidKp = value;
}

void ConfigManager::setKi(float value) {
  pidKi = value;
}

void ConfigManager::setKd(float value) {
  pidKd = value;
}

// Getters
const char* ConfigManager::getDeviceName() const {
  return deviceName;
}

uint32_t ConfigManager::getSendInterval() const {
  return sendInterval;
}

uint32_t ConfigManager::getPidInterval() const {
  return pidInterval;
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

uint8_t ConfigManager::getKnxTempArea() const {
  return knxTempArea;
}

uint8_t ConfigManager::getKnxTempLine() const {
  return knxTempLine;
}

uint8_t ConfigManager::getKnxTempMember() const {
  return knxTempMember;
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

uint8_t ConfigManager::getKnxValveArea() const {
  return knxValveArea;
}

uint8_t ConfigManager::getKnxValveLine() const {
  return knxValveLine;
}

uint8_t ConfigManager::getKnxValveMember() const {
  return knxValveMember;
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

bool ConfigManager::getKnxEnabled() const {
  return knxEnabled;
}

bool ConfigManager::getMqttEnabled() const {
  return mqttEnabled;
}

float ConfigManager::getKp() const {
  return pidKp;
}

float ConfigManager::getKi() const {
  return pidKi;
}

float ConfigManager::getKd() const {
  return pidKd;
}

void ConfigManager::getKnxPhysicalAddress(uint8_t& area, uint8_t& line, uint8_t& member) const {
  area = knxPhysicalArea;
  line = knxPhysicalLine;
  member = knxPhysicalMember;
}

void ConfigManager::getKnxTemperatureGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
  area = knxTempArea;
  line = knxTempLine;
  member = knxTempMember;
}

void ConfigManager::getKnxSetpointGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
  area = knxSetpointArea;
  line = knxSetpointLine;
  member = knxSetpointMember;
}

void ConfigManager::getKnxValveGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
  area = knxValveArea;
  line = knxValveLine;
  member = knxValveMember;
}

void ConfigManager::getKnxModeGA(uint8_t& area, uint8_t& line, uint8_t& member) const {
  area = knxModeArea;
  line = knxModeLine;
  member = knxModeMember;
}