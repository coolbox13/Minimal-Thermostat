#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "thermostat_state.h"
#include "config_manager.h"
#include "control/pid_controller.h"

class WebInterface {
public:
    WebInterface(ConfigManager* configManager, ThermostatState* state);
    virtual ~WebInterface() = default;

    bool begin();
    void stop();
    bool isConnected() const;
    void setPort(uint16_t port);
    void setCredentials(const char* username, const char* password);
    void setHostname(const char* hostname);
    ThermostatStatus getLastError() const;

protected:
    void setupRoutes();
    void handleGetStatus(AsyncWebServerRequest* request);
    void handleSaveConfig(AsyncWebServerRequest* request);
    void handleNotFound(AsyncWebServerRequest* request);
    bool handleFileRead(String path);
    void handleJsonResponse(String& json);
    void handleError(const char* message, int code = 500);
    String generateHtml();
    String generateStatusJson();
    String generateConfigJson();

    AsyncWebServer server;
    ConfigManager* configManager;
    ThermostatState* state;
    uint16_t port;
    char username[32];
    char password[32];
    char hostname[32];
    ThermostatStatus lastError;
}; 