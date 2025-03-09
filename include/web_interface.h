#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include "thermostat_state.h"
#include "config_manager.h"
#include "sensor_interface.h"
#include "knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"
#include "protocol_manager.h"

#ifdef ESP32
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <LITTLEFS.h>
  #define WebServerClass WebServer
  #define LittleFS LITTLEFS
#elif defined(ESP8266)
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <LittleFS.h>
  #define WebServerClass ESP8266WebServer
#endif

// Command and source definitions for protocol manager
#define CMD_SET_TEMPERATURE 1
#define SOURCE_WEB_API 1

class WebInterface {
public:
  // Constructor
  WebInterface();
  
  // Initialize web server
  bool begin(ThermostatState* state, 
             ConfigManager* config, 
             SensorInterface* sensor,
             KNXInterface* knx,
             MQTTInterface* mqtt = nullptr,
             PIDController* pid = nullptr, 
             ProtocolManager* protocol = nullptr);
  
  // Process HTTP requests (call in loop)
  void handle();

private:
  // Web server
  WebServerClass server;
  
  // References to components
  ThermostatState* thermostatState;
  ConfigManager* configManager;
  SensorInterface* sensorInterface;
  KNXInterface* knxInterface;
  MQTTInterface* mqttInterface;
  PIDController* pidController;
  ProtocolManager* protocolManager;
  
  // HTTP request handlers
  void handleRoot();
  void handleSave();
  void handleSaveConfig();
  void handleSetpoint();
  void handleReboot();
  void handleFactoryReset();
  void handleNotFound();
  void handleGetStatus();
  
  // Helper to generate HTML
  String generateHtml();
  
  // Setup MDNS
  void setupMDNS();
  
  // File handling
  bool handleFileRead(String path);
  
  // Authentication
  bool isAuthenticated();
  void requestAuthentication();
};

#endif // WEB_INTERFACE_H