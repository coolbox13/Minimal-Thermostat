#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

// Determine which file system to use based on platform
#include <LittleFS.h>
#define FileFS LittleFS

#include <ArduinoJson.h>

// Default device name
#define DEFAULT_DEVICE_NAME "KNX-Thermostat"
#define CONFIG_FILE "/config.json"

class ConfigManager {
public:
  // Constructor
  ConfigManager();
  
  // Initialize configuration
  bool begin();
  
  // WiFi setup with WiFiManager
  bool setupWiFi();
  
  // Save and load configuration
  bool saveConfig();
  bool loadConfig();
  
  // Set defaults
  void setDefaults();
  
  // Factory reset (clear all settings)
  void factoryReset();
  
  // Setters
  void setDeviceName(const char* name);
  void setSendInterval(int interval);
  void setPidInterval(int interval);
  
  // KNX physical address
  void setKnxPhysicalAddress(int area, int line, int member);
  
  // KNX group addresses
  void setKnxTemperatureGA(int area, int line, int member);
  void setKnxSetpointGA(int area, int line, int member);
  void setKnxValveGA(int area, int line, int member);
  void setKnxModeGA(int area, int line, int member);
  
  // KNX general settings
  void setKnxEnabled(bool enabled);
  
  // MQTT settings
  void setMqttServer(const char* server);
  void setMqttPort(int port);
  void setMqttUser(const char* user);
  void setMqttPassword(const char* password);
  void setMqttClientId(const char* clientId);
  void setMqttEnabled(bool enabled);
  
  // PID settings
  void setKp(float value);
  void setKi(float value);
  void setKd(float value);
  void setSetpoint(float value);
  
  // Getters
  const char* getDeviceName() const;
  int getSendInterval() const;
  int getPidInterval() const;
  
  // KNX physical address
  int getKnxPhysicalArea() const;
  int getKnxPhysicalLine() const;
  int getKnxPhysicalMember() const;
  
  // KNX group addresses
  int getKnxTempArea() const;
  int getKnxTempLine() const;
  int getKnxTempMember() const;
  
  int getKnxSetpointArea() const;
  int getKnxSetpointLine() const;
  int getKnxSetpointMember() const;
  
  int getKnxValveArea() const;
  int getKnxValveLine() const;
  int getKnxValveMember() const;
  
  int getKnxModeArea() const;
  int getKnxModeLine() const;
  int getKnxModeMember() const;
  
  // KNX general settings
  bool getKnxEnabled() const;
  
  // MQTT settings
  const char* getMqttServer() const;
  int getMqttPort() const;
  const char* getMqttUser() const;
  const char* getMqttPassword() const;
  const char* getMqttClientId() const;
  bool getMqttEnabled() const;
  
  // PID settings
  float getKp() const;
  float getKi() const;
  float getKd() const;
  float getSetpoint() const;

private:
  // Device settings
  char deviceName[32];
  int sendInterval;
  int pidInterval;
  
  // KNX physical address
  int knxPhysicalArea;
  int knxPhysicalLine;
  int knxPhysicalMember;
  
  // KNX enabled flag
  bool knxEnabled;
  
  // KNX temperature group address
  int knxTempArea;
  int knxTempLine;
  int knxTempMember;
  
  // KNX setpoint group address
  int knxSetpointArea;
  int knxSetpointLine;
  int knxSetpointMember;
  
  // KNX valve position group address
  int knxValveArea;
  int knxValveLine;
  int knxValveMember;
  
  // KNX operating mode group address
  int knxModeArea;
  int knxModeLine;
  int knxModeMember;
  
  // MQTT settings
  char mqttServer[40];
  int mqttPort;
  char mqttUser[24];
  char mqttPassword[24];
  char mqttClientId[24];
  bool mqttEnabled;
  
  // PID settings
  float kp;
  float ki;
  float kd;
  float setpoint;
};

#endif // CONFIG_MANAGER_H