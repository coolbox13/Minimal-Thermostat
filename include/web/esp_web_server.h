#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncJson.h>

// Forward declarations
class ThermostatState;
class ProtocolManager;

#include "web/web_interface.h"
#include "config_manager.h"
#include "thermostat_state.h"

class ESPWebServer {
public:
    ESPWebServer(ConfigManager* configManager, ThermostatState* state);
    virtual ~ESPWebServer() = default;

    bool begin();
    void stop();

    // WebInterface implementation
    bool isConnected() const;
    void setPort(uint16_t port);
    void setCredentials(const char* username, const char* password);
    void setHostname(const char* hostname);
    ThermostatStatus getLastError() const;

    // API endpoints
    void handleRoot();
    void handleSave();
    void handleSetpoint();
    void handleMode();
    void handleStatus();
    void handleConfig();
    void handleReboot();
    void handleReset();
    void handleNotFound();

    // Component registration
    void registerComponents(
        ThermostatState* state,
        ConfigInterface* config,
        ControlInterface* control = nullptr,
        ProtocolInterface* knx = nullptr,
        ProtocolInterface* mqtt = nullptr
    );

protected:
    // Helper methods
    bool isAuthenticated();
    void requestAuthentication();
    String generateHtml();
    bool handleFileRead(String path);

private:
    // Web server
    AsyncWebServer server;
    uint16_t port;
    char username[32];
    char password[32];
    char hostname[32];
    ThermostatStatus lastError;
    bool initialized;

    // Component references
    ThermostatState* thermostatState;
    ConfigInterface* configManager;
    ControlInterface* pidController;
    ProtocolInterface* knxInterface;
    ProtocolInterface* mqttInterface;

    // Internal helpers
    void setupRoutes();
    void setupMDNS();
    String getContentType(String filename);
    String generateStatusJson();
    String generateConfigJson();
    void handleJsonResponse(String& json);
    void handleError(const char* message, int code = 500);

    void handleGetStatus(AsyncWebServerRequest* request);
    void handleSaveConfig(AsyncWebServerRequest* request);
    void handleNotFound(AsyncWebServerRequest* request);
};

#endif // ESP_WEB_SERVER_H 