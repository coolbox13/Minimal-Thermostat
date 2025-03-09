#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include "base64.h"
#include "../config_manager.h"
#include "../sensor_interface.h"
#include "../pid_controller.h"
#include "../thermostat_state.h"
#include "../communication/protocol_manager.h"

class WebInterface {
public:
    WebInterface(ConfigManager* configManager, SensorInterface* sensorInterface, 
                PIDController* pidController, ThermostatState* thermostatState,
                ProtocolManager* protocolManager);
    ~WebInterface();

    void begin();
    void end();
    void handleClient();

private:
    WebServer server;
    ConfigManager* configManager;
    SensorInterface* sensorInterface;
    PIDController* pidController;
    ThermostatState* thermostatState;
    ProtocolManager* protocolManager;

    // Request handlers
    void handleRoot();
    void handleSave();
    void handleGetStatus();
    void handleSetpoint();
    void handleSaveConfig();
    void handleReboot();
    void handleFactoryReset();
    void handleNotFound();

    // File handling
    bool handleFileRead(String path);
    String getContentType(String filename);

    // Security
    bool isAuthenticated();
    void requestAuthentication();
    void addSecurityHeaders();
    bool validateCSRFToken();
    String generateCSRFToken();

    // HTML generation
    String generateHtml();
}; 