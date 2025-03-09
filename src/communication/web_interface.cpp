#include "web_interface.h"
#include <ArduinoJson.h>

WebInterface::WebInterface() : 
  server(80),
  thermostatState(nullptr),
  configManager(nullptr),
  sensorInterface(nullptr),
  knxInterface(nullptr),
  mqttInterface(nullptr),
  pidController(nullptr),
  protocolManager(nullptr) {
}

bool WebInterface::begin(ThermostatState* state, 
                        ConfigManager* config, 
                        SensorInterface* sensor,
                        KNXInterface* knx,
                        MQTTInterface* mqtt,
                        PIDController* pid,
                        ProtocolManager* protocol) {
  
  thermostatState = state;
  configManager = config;
  sensorInterface = sensor;
  knxInterface = knx;
  mqttInterface = mqtt;
  pidController = pid;
  protocolManager = protocol;
  
  // Initialize SPIFFS/LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An error occurred while mounting LittleFS");
    return false;
  }
  
  // Setup MDNS
  setupMDNS();
  
  // Setup server routes with proper lambda return types
  server.on("/", HTTP_GET, std::bind(&WebInterface::handleRoot, this));
  server.on("/save", HTTP_POST, std::bind(&WebInterface::handleSave, this));
  server.on("/saveconfig", HTTP_POST, std::bind(&WebInterface::handleSaveConfig, this));
  server.on("/setpoint", HTTP_POST, std::bind(&WebInterface::handleSetpoint, this));
  server.on("/reboot", HTTP_POST, std::bind(&WebInterface::handleReboot, this));
  server.on("/factory_reset", HTTP_POST, std::bind(&WebInterface::handleFactoryReset, this));
  server.on("/status", HTTP_GET, std::bind(&WebInterface::handleGetStatus, this));
  
  // Handle not found - needs special handling for file serving
  server.onNotFound([this]() {
    if (!handleFileRead(server.uri())) {
      handleNotFound();
    }
  });
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  
  return true;
}

void WebInterface::handle() {
  server.handleClient();
  MDNS.update();
}