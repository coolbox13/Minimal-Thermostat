#pragma once

#include <Arduino.h>
#include <ESPAsyncWiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "thermostat_types.h"

class ConfigManager {
private:
    // MQTT settings
    char mqttServer[64];
    uint16_t mqttPort;
    char mqttUser[32];
    char mqttPassword[32];
    bool mqttEnabled;

    // Web interface settings
    char adminUsername[32];
    char adminPassword[32];

    // Thermostat settings
    float targetTemp;
    float hysteresis;
    ThermostatMode mode;

    // PID settings
    float pidKp;
    float pidKi;
    float pidKd;

    void loadConfig();
    void saveConfig();

public:
    ConfigManager();
    ~ConfigManager();
    
    bool begin();
    void end();
    void resetToDefaults();

    // MQTT getters and setters
    const char* getMQTTServer() const;
    uint16_t getMQTTPort() const;
    const char* getMQTTUser() const;
    const char* getMQTTPassword() const;
    bool getMQTTEnabled() const { return mqttEnabled; }

    void setMQTTServer(const char* server);
    void setMQTTPort(uint16_t port);
    void setMQTTUser(const char* user);
    void setMQTTPassword(const char* password);
    void setMQTTEnabled(bool enabled) { mqttEnabled = enabled; }

    // Web interface getters and setters
    const char* getAdminUsername() const;
    const char* getAdminPassword() const;

    // Thermostat getters and setters
    float getTargetTemp() const;
    float getHysteresis() const;
    ThermostatMode getMode() const;

    void setTargetTemp(float temp);
    void setHysteresis(float hyst);
    void setMode(ThermostatMode m);

    // PID getters and setters
    float getKp() const;
    float getKi() const;
    float getKd() const;

    void setKp(float value);
    void setKi(float value);
    void setKd(float value);

    // Configuration management
    bool loadConfig();
    void saveConfig();
};