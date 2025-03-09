#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <FS.h>

// Define the appropriate file system
#if defined(ESP32)
  #include <LittleFS.h>
  #define FileFS LittleFS
#elif defined(ESP8266)
  #include <LittleFS.h>
  #define FileFS LittleFS
#endif

#define CONFIG_FILE "/config.json"
#define DEFAULT_DEVICE_NAME "KNX-Thermostat"

class ConfigManager {
public:
  // Constructor
  ConfigManager();
  
  // Initialize configuration and file system
  bool begin();
  
  // Setup WiFi using WiFiManager
  bool setupWiFi();
  
  // Save and load configuration
  bool saveConfig();
  bool loadConfig();
  
  // Reset to factory defaults
  void factoryReset();
  
  // Setters
  void setDeviceName(const char* name);
  void setSendInterval(int interval);
  void setPidInterval(int interval);
  void setKnxEnabled(bool enabled);
  void setKnxPhysicalAddress(int area, int line, int member);
  void setKnxTemperatureGA(int area, int line, int member);
  void setKnxSetpointGA(int area, int line, int member);
  void setKnxValveGA(int area, int line, int member);
  void setKnxModeGA(int area, int line, int member);
  void setMqttEnabled(bool enabled);
  void setMqttServer(const char* server);
  void setMqttPort(int port);
  void setMqttUser(const char* user);
  void setMqttPassword(const char* password);
  void setMqttClientId(const char* clientId);
  void setKp(float value);
  void setKi(float value);
  void setKd(float value);
  void setSetpoint(float value);
  void setWebUsername(const char* username);
  void setWebPassword(const char* password);
  
  // Getters
  const char* getDeviceName() const;
  int getSendInterval() const;
  int getPidInterval() const;
  bool getKnxEnabled() const;
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
  int getKnxModeArea() const;
  int getKnxModeLine() const;
  int getKnxModeMember() const;
  bool getMqttEnabled() const;
  const char* getMqttServer() const;
  int getMqttPort() const;
  const char* getMqttUser() const;
  const char* getMqttPassword() const;
  const char* getMqttClientId() const;
  float getKp() const;
  float getKi() const;
  float getKd() const;
  float getSetpoint() const;
  const char* getWebUsername() const;
  const char* getWebPassword() const;
  
private:
  // Configuration values
  char deviceName[32];
  int sendInterval;
  int pidInterval;
  
  // KNX settings
  bool knxEnabled;
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
  
  int knxModeArea;
  int knxModeLine;
  int knxModeMember;
  
  // MQTT settings
  bool mqttEnabled;
  char mqttServer[40];
  int mqttPort;
  char mqttUser[24];
  char mqttPassword[24];
  char mqttClientId[24];
  
  // Web authentication settings
  char webUsername[24];
  char webPassword[24];
  
  // PID settings
  float kp;
  float ki;
  float kd;
  float setpoint;
  
  // Set default values
  void setDefaults();
};

#endif // CONFIG_MANAGER_H