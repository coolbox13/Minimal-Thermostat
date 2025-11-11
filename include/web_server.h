#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <functional>
#include "config.h"
#include "config_manager.h"

// Forward declaration of embedded HTML
extern const char* THERMOSTAT_HTML;

class WebServerManager {
public:
    // Define callback type for KNX address changes
    typedef std::function<void()> KnxAddressChangedCallback;

    static WebServerManager* getInstance();
    void begin(AsyncWebServer* server);
    void addEndpoint(const char* uri, WebRequestMethodComposite method,
                     ArRequestHandlerFunction handler);
    void addEndpoint(const char* uri, WebRequestMethodComposite method,
                     ArRequestHandlerFunction onRequest,
                     ArUploadHandlerFunction onUpload);
    void setupDefaultRoutes();
    AsyncWebServer* getServer() { return _server; }
    
    // Method to register callback for KNX address changes
    void setKnxAddressChangedCallback(KnxAddressChangedCallback callback);

private:
    WebServerManager();
    static WebServerManager* _instance;
    AsyncWebServer* _server;
    
    // Callback for KNX address changes
    KnxAddressChangedCallback _knxAddressChangedCallback = nullptr;

    // Default route handlers
    static void handleRoot(AsyncWebServerRequest *request);
    static void handleTest(AsyncWebServerRequest *request);
    static void handlePing(AsyncWebServerRequest *request);

    // Config update helper methods
    void handleKNXAddressChange(const JsonDocument& jsonDoc, bool oldUseTestSetting);
    void handlePIDParameterUpdates(const JsonDocument& jsonDoc);
    void handleNTPUpdate(const JsonDocument& jsonDoc);
};

#endif // WEB_SERVER_H