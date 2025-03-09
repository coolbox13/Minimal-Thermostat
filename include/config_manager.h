#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include "thermostat_types.h"
#include "protocol_types.h"

// Define CONFIG_FILE path
#define CONFIG_FILE "/config.json"
#define DEFAULT_DEVICE_NAME "ESP32-Thermostat"

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Initialization
    bool begin();
    void end();
    bool setupWiFi();

    // Configuration management
    bool loadConfig();
    bool saveConfig();
    void factoryReset();
    void setDefaults();

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
    // Member variables instead of PIMPL
    char deviceName[32];
    uint32_t sendInterval;
    uint32_t pidInterval;
    
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
    
    // Web authentication
    char webUsername[32];
    char webPassword[32];
    
    // PID settings
    float kp;
    float ki;
    float kd;
    float setpoint;
};

#endif // CONFIG_MANAGER_H