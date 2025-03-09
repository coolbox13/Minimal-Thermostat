#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

// Include our fixed WebServer definitions
#include "../config/wifi_manager_fix.h"

#include <ESPmDNS.h>
#include <FS.h>
#include <LITTLEFS.h>
#define FileFS LITTLEFS

#include <WiFi.h>
#include "config_manager.h"
#include "thermostat_state.h"
#include "interfaces/sensor_interface.h"
#include "communication/knx/knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"
#include "protocol_manager.h"
#include "web/web_auth_manager.h"

// HTTP method constants
#ifndef HTTP_GET
#define HTTP_GET 0
#endif
#ifndef HTTP_POST
#define HTTP_POST 1
#endif
#ifndef HTTP_PUT
#define HTTP_PUT 2
#endif
#ifndef HTTP_DELETE
#define HTTP_DELETE 3
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
             MQTTInterface* mqtt = nullptr,
             PIDController* pid = nullptr, 
             ProtocolManager* protocol = nullptr);
  
  // Process HTTP requests (call in loop)
  void handle();

private:
  // Web server
  WebServer server;
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