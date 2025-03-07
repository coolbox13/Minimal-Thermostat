#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include "thermostat_state.h"
#include "config_manager.h"
#include "sensor_interface.h"
#include "knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"

#ifdef ESP32
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #define WebServerClass WebServer
#elif defined(ESP8266)
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #define WebServerClass ESP8266WebServer
#endif

class WebInterface {
public:
  // Constructor
  WebInterface();
  
  // Initialize web server
  bool begin(ThermostatState* state, 
             ConfigManager* config, 
             SensorInterface* sensor,
             KNXInterface* knx,
             MQTTInterface* mqtt,
             PIDController* pid);
  
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
  
  // HTTP request handlers
  void handleRoot();
  void handleSave();
  void handleReboot();
  void handleReset();
  void handleNotFound();
  void handleGetStatus();
  
  // Helper to generate HTML
  String generateHtml();
  
  // Setup MDNS
  void setupMDNS();
};

#endif // WEB_INTERFACE_H