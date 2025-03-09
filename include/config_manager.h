#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "thermostat_types.h"

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    bool begin();
    void end();
    bool setupWiFi();
    bool loadConfig();
    void saveConfig();
    void resetToDefaults();

    // Device settings
    void setDeviceName(const char* name);
    const char* getDeviceName() const;
    void setSendInterval(uint32_t interval);
    uint32_t getSendInterval() const;
    void setPidInterval(uint32_t interval);
    uint32_t getPidInterval() const;

    // Web interface settings
    void setWebUsername(const char* username);
    void setWebPassword(const char* password);
    const char* getWebUsername() const;
    const char* getWebPassword() const;

    // KNX settings
    void setKnxEnabled(bool enabled);
    bool getKnxEnabled() const;
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
    void setMQTTEnabled(bool enabled);
    bool getMQTTEnabled() const;
    void setMQTTServer(const char* server);
    const char* getMQTTServer() const;
    void setMQTTPort(uint16_t port);
    uint16_t getMQTTPort() const;
    void setMQTTUser(const char* user);
    const char* getMQTTUser() const;
    void setMQTTPassword(const char* password);
    const char* getMQTTPassword() const;
    void setMQTTClientId(const char* clientId);
    const char* getMQTTClientId() const;

    // Thermostat settings
    void setSetpoint(float setpoint);
    float getSetpoint() const;

private:
    char deviceName[32];
    uint32_t sendInterval;
    uint32_t pidInterval;
    
    // Web interface credentials
    char webUsername[32];
    char webPassword[32];
    
    // KNX settings
    bool knxEnabled;
    uint8_t knxPhysicalArea;
    uint8_t knxPhysicalLine;
    uint8_t knxPhysicalMember;
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
    char mqttServer[64];
    uint16_t mqttPort;
    char mqttUser[32];
    char mqttPassword[32];
    char mqttClientId[32];
    
    // Thermostat settings
    float setpoint;
};