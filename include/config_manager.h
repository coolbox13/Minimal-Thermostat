#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "thermostat_types.h"
#include "pid_controller.h"

// Forward declarations
class ThermostatState;
class ProtocolManager;

struct KNXPhysicalAddress {
    uint8_t area;
    uint8_t line;
    uint8_t member;
};

class ConfigManager {
public:
    ConfigManager();
    virtual ~ConfigManager() = default;
    
    bool begin();
    void end();
    bool setupWiFi();
    bool loadConfig();
    void saveConfig();
    void resetToDefaults();

    // WiFi settings
    const char* getWiFiSSID() const { return wifiSSID; }
    const char* getWiFiPassword() const { return wifiPassword; }
    void setWiFiSSID(const char* ssid);
    void setWiFiPassword(const char* password);

    // Device settings
    const char* getDeviceName() const;
    void setDeviceName(const char* name);
    void setSendInterval(uint32_t interval);
    uint32_t getSendInterval() const;
    void setPidInterval(uint32_t interval);
    uint32_t getPidInterval() const;

    // Web interface settings
    const char* getWebUsername() const;
    const char* getWebPassword() const;
    void setWebUsername(const char* username);
    void setWebPassword(const char* password);

    // KNX settings
    bool getKnxEnabled() const;
    void setKnxEnabled(bool enabled);
    void setKnxPhysicalAddress(uint8_t area, uint8_t line, uint8_t member);
    void getKnxPhysicalAddress(uint8_t& area, uint8_t& line, uint8_t& member) const;
    uint8_t getKnxPhysicalArea() const;
    uint8_t getKnxPhysicalLine() const;
    uint8_t getKnxPhysicalMember() const;

    void setKnxTemperatureGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxTemperatureGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    uint8_t getKnxTempArea() const;
    uint8_t getKnxTempLine() const;
    uint8_t getKnxTempMember() const;

    void setKnxSetpointGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxSetpointGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    uint8_t getKnxSetpointArea() const;
    uint8_t getKnxSetpointLine() const;
    uint8_t getKnxSetpointMember() const;

    void setKnxValveGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxValveGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    uint8_t getKnxValveArea() const;
    uint8_t getKnxValveLine() const;
    uint8_t getKnxValveMember() const;

    void setKnxModeGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxModeGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    uint8_t getKnxModeArea() const;
    uint8_t getKnxModeLine() const;
    uint8_t getKnxModeMember() const;

    // MQTT settings
    bool getMQTTEnabled() const;
    void setMQTTEnabled(bool enabled);
    const char* getMQTTServer() const;
    uint16_t getMQTTPort() const;
    const char* getMQTTUser() const;
    const char* getMQTTPassword() const;
    const char* getMQTTClientId() const;
    void setMQTTServer(const char* server);
    void setMQTTPort(uint16_t port);
    void setMQTTUser(const char* user);
    void setMQTTPassword(const char* password);
    void setMQTTClientId(const char* clientId);
    const char* getMQTTTopicPrefix() const;
    void setMQTTTopicPrefix(const char* prefix);

    // Thermostat settings
    void setSetpoint(float setpoint);
    float getSetpoint() const;

    // PID settings
    const PIDConfig& getPidConfig() const;
    void setPidConfig(const PIDConfig& config);

private:
    Preferences prefs;
    bool initialized;

    // WiFi settings
    char wifiSSID[33];
    char wifiPassword[65];

    // Device settings
    char deviceName[33];
    uint32_t sendInterval;
    uint32_t pidInterval;
    
    // Web interface credentials
    char webUsername[33];
    char webPassword[65];
    
    // KNX settings
    bool knxEnabled;
    KNXPhysicalAddress knxPhysicalAddress;
    uint8_t knxTempArea;
    uint8_t knxTempLine;
    uint8_t knxTempMember;
    uint8_t knxSetpointArea;
    uint8_t knxSetpointLine;
    uint8_t knxSetpointMember;
    uint8_t knxValveArea;
    uint8_t knxValveLine;
    uint8_t knxValveMember;
    uint8_t knxModeArea;
    uint8_t knxModeLine;
    uint8_t knxModeMember;
    
    // MQTT settings
    bool mqttEnabled;
    char mqttServer[65];
    uint16_t mqttPort;
    char mqttUser[33];
    char mqttPassword[65];
    char mqttClientId[33];
    char mqttTopicPrefix[65];
    
    // Thermostat settings
    float setpoint;

    // PID settings
    PIDConfig pidConfig;

    void loadDefaults();
};