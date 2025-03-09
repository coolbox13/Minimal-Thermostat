#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include "base64.h"

#include "config_manager.h"
#include "interfaces/sensor_interface.h"
#include "pid_controller.h"
#include "thermostat_state.h"
#include "protocol_manager.h"

class WebInterface {
public:
    WebInterface(ConfigManager* configManager, SensorInterface* sensorInterface,
                PIDController* pidController, ThermostatState* thermostatState,
                ProtocolManager* protocolManager = nullptr);
    ~WebInterface();

    void begin();
    void end();
    void loop();

private:
    AsyncWebServer server;
    ConfigManager* configManager;
    SensorInterface* sensorInterface;
    PIDController* pidController;
    ThermostatState* thermostatState;
    ProtocolManager* protocolManager;

    // Request handlers
    void handleRoot(AsyncWebServerRequest* request);
    void handleSave(AsyncWebServerRequest* request);
    void handleGetStatus(AsyncWebServerRequest* request);
    void handleSetpoint(AsyncWebServerRequest* request);
    void handleSaveConfig(AsyncWebServerRequest* request);
    void handleGetConfig(AsyncWebServerRequest* request);
    void handleReboot(AsyncWebServerRequest* request);
    void handleFactoryReset(AsyncWebServerRequest* request);
    void handleNotFound(AsyncWebServerRequest* request);
    bool handleFileRead(AsyncWebServerRequest* request, String path);

    // Helper functions
    bool isAuthenticated(AsyncWebServerRequest* request);
    void requestAuthentication(AsyncWebServerRequest* request);
    String getContentType(String filename);
    void addSecurityHeaders(AsyncWebServerResponse* response);
    bool validateCSRFToken(AsyncWebServerRequest* request);
    String generateCSRFToken(AsyncWebServerRequest* request);
    String generateHtml();
    void setupMDNS();
}; 