#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include "thermostat_types.h"
#include "protocol_types.h"

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Initialization
    bool begin();
    void end();

    // Configuration management
    bool loadConfig();
    bool saveConfig();
    void factoryReset();

    // Device settings
    void setDeviceName(const char* name);
    const char* getDeviceName() const;
    void setSendInterval(uint32_t interval);
    uint32_t getSendInterval() const;
    void setPidInterval(uint32_t interval);
    uint32_t getPidInterval() const;

    // Web authentication
    void setWebUsername(const char* username);
    const char* getWebUsername() const;
    void setWebPassword(const char* password);
    const char* getWebPassword() const;

    // KNX settings
    void setKnxEnabled(bool enabled);
    bool getKnxEnabled() const;
    void setKnxPhysicalAddress(uint8_t area, uint8_t line, uint8_t member);
    void getKnxPhysicalAddress(uint8_t& area, uint8_t& line, uint8_t& member) const;
    uint8_t getKnxPhysicalArea() const;
    uint8_t getKnxPhysicalLine() const;
    uint8_t getKnxPhysicalMember() const;

    // KNX group addresses
    void setKnxTemperatureGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxTemperatureGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    void setKnxSetpointGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxSetpointGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    void setKnxValveGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxValveGA(uint8_t& area, uint8_t& line, uint8_t& member) const;
    void setKnxModeGA(uint8_t area, uint8_t line, uint8_t member);
    void getKnxModeGA(uint8_t& area, uint8_t& line, uint8_t& member) const;

    // MQTT settings
    void setMqttEnabled(bool enabled);
    bool getMqttEnabled() const;
    void setMqttServer(const char* server);
    const char* getMqttServer() const;
    void setMqttPort(uint16_t port);
    uint16_t getMqttPort() const;
    void setMqttUser(const char* user);
    const char* getMqttUser() const;
    void setMqttPassword(const char* password);
    const char* getMqttPassword() const;
    void setMqttClientId(const char* clientId);
    const char* getMqttClientId() const;

    // PID settings
    void setKp(float kp);
    float getKp() const;
    void setKi(float ki);
    float getKi() const;
    void setKd(float kd);
    float getKd() const;
    void setSetpoint(float setpoint);
    float getSetpoint() const;

private:
    // Private implementation
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif // CONFIG_MANAGER_H