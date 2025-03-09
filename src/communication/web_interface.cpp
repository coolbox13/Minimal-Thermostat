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
  
  // Set up web server routes
  server.on("/", HTTP_GET, [this](){ this->handleRoot(); });
  server.on("/save", HTTP_POST, [this](){ this->handleSave(); });
  server.on("/api/save-config", HTTP_POST, [this](){ this->handleSaveConfig(); });
  server.on("/api/setpoint", HTTP_GET, [this](){ this->handleSetpoint(); });
  server.on("/reboot", HTTP_GET, [this](){ this->handleReboot(); });
  server.on("/factory-reset", HTTP_GET, [this](){ this->handleFactoryReset(); });
  server.on("/api/status", HTTP_GET, [this](){ this->handleGetStatus(); });
  
  // For static files from LittleFS
  server.onNotFound([this](){
    if(!handleFileRead(server.uri())) {
      this->handleNotFound();
    }
  });
  
  // Start the web server
  server.begin();
  
  // Set up mDNS
  setupMDNS();
  
  Serial.println("Web server started");
  return true;
}

void WebInterface::handle() {
  server.handleClient();
}