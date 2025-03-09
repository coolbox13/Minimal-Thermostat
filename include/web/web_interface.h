#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

#ifdef ESP32
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <FS.h>
  #include <LittleFS.h>
  #define WebServerClass WebServer
  #define FileFS LittleFS
#elif defined(ESP8266)
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <LittleFS.h>
  #define WebServerClass ESP8266WebServer
  #define FileFS LittleFS
#endif

#include <WiFi.h>
#include "web_auth_manager.h"

// Forward declarations
class ThermostatState;
class ConfigManager;
class SensorInterface;
class KNXInterface;
class MQTTInterface;
class PIDController;
class ProtocolManager;

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
  std::unique_ptr<WebAuthManager> authManager;
  
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
  
  // Security helpers
  bool isAuthenticated();
  void requestAuthentication();
  void addSecurityHeaders();
  bool validateCSRFToken();
  String generateCSRFToken();
};

#endif // WEB_INTERFACE_H 