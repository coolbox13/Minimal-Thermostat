#ifndef ESP_WEB_SERVER_H
#define ESP_WEB_SERVER_H

#include "interfaces/web_interface.h"
#include "interfaces/config_interface.h"
#include "interfaces/control_interface.h"
#include "interfaces/protocol_interface.h"
#include "thermostat_state.h"

#ifdef ESP32
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <LITTLEFS.h>
  #define WebServerClass WebServer
  #define FileFS LITTLEFS
#elif defined(ESP8266)
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <LittleFS.h>
  #define WebServerClass ESP8266WebServer
  #define FileFS LittleFS
#endif

class ESPWebServer : public WebInterface {
public:
    ESPWebServer(uint16_t port = 80);

    // WebInterface implementation
    bool begin() override;
    void loop() override;
    bool isConnected() const override;
    void setPort(uint16_t port) override;
    void setCredentials(const char* username, const char* password) override;
    void setHostname(const char* hostname) override;
    ThermostatStatus getLastError() const override;

    // API endpoints
    void handleRoot() override;
    void handleSave() override;
    void handleSetpoint() override;
    void handleMode() override;
    void handleStatus() override;
    void handleConfig() override;
    void handleReboot() override;
    void handleReset() override;
    void handleNotFound() override;

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
    bool isAuthenticated() override;
    void requestAuthentication() override;
    String generateHtml() override;
    bool handleFileRead(String path) override;

private:
    // Web server
    WebServerClass server;
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
};

#endif // ESP_WEB_SERVER_H 