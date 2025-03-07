#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef ESP32
  #include <WiFi.h>
  #include <SPIFFS.h>
  #define FileFS SPIFFS
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <LittleFS.h>
  #define FileFS LittleFS
#endif

#include <WiFiManager.h>

class ConfigManager {
public:
  // Constructor
  ConfigManager();
  
  // Initialize configuration manager
  bool begin();
  
  // Set up WiFi using WiFiManager
  bool setupWiFi();
  
  // Load/save configuration
  bool saveConfig();
  bool loadConfig();
  
  // Reset settings
  void setDefaults();
  void factoryReset();
  
  // Setters
  void setDeviceName(const char* name);
  void setSendInterval(int interval);
  void setPidInterval(int interval);
  
  void setKnxPhysicalAddress(int area, int line, int member);
  void setKnxTemperatureGA(int area, int line, int member);
  void setKnxSetpointGA(int area, int line, int member);
  void setKnxValveGA(int area, int line, int member);
  
  void setMqttServer(const char* server);
  void setMqttPort(int port);
  void setMqttUser(const char* user);
  void setMqttPassword(const char* password);
  void setMqttClientId(const char* clientId);
  
  void setKp(float value);
  void setKi(float value);
  void setKd(float value);
  void setSetpoint(float value);
  
  // Getters
  const char* getDeviceName() const;
  int getSendInterval() const;
  int getPidInterval() const;
  
  int getKnxPhysicalArea() const;
  int getKnxPhysicalLine() const;
  int getKnxPhysicalMember() const;
  
  int getKnxTempArea() const;
  int getKnxTempLine() const;
  int getKnxTempMember() const;
  
  int getKnxSetpointArea() const;
  int getKnxSetpointLine() const;
  int getKnxSetpointMember() const;
  
  int getKnxValveArea() const;
  int getKnxValveLine() const;
  int getKnxValveMember() const;
  
  const char* getMqttServer() const;
  int getMqttPort() const;
  const char* getMqttUser() const;
  const char* getMqttPassword() const;
  const char* getMqttClientId() const;
  
  float getKp() const;
  float getKi() const;
  float getKd() const;
  float getSetpoint() const;

private:
  // Configuration file path
  static constexpr const char* CONFIG_FILE = "/config.json";
  static constexpr const char* DEFAULT_DEVICE_NAME = "KNX-Thermostat";
  
  // Settings
  char deviceName[32];
  int sendInterval;
  int pidInterval;
  
  // KNX settings
  int knxPhysicalArea;
  int knxPhysicalLine;
  int knxPhysicalMember;
  
  int knxTempArea;
  int knxTempLine;
  int knxTempMember;
  
  int knxSetpointArea;
  int knxSetpointLine;
  int knxSetpointMember;
  
  int knxValveArea;
  int knxValveLine;
  int knxValveMember;
  
  // MQTT settings
  char mqttServer[40];
  int mqttPort;
  char mqttUser[24];
  char mqttPassword[24];
  char mqttClientId[24];
  
  // PID settings
  float kp;
  float ki;
  float kd;
  float setpoint;
};

#endif // CONFIG_MANAGER_H