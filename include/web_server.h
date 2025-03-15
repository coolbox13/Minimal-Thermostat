#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"

class WebServerManager {
public:
    static WebServerManager* getInstance();
    void begin(AsyncWebServer* server);
    void addEndpoint(const char* uri, WebRequestMethodComposite method,
                     ArRequestHandlerFunction handler);
    void addEndpoint(const char* uri, WebRequestMethodComposite method,
                     ArRequestHandlerFunction onRequest,
                     ArUploadHandlerFunction onUpload);
    void setupDefaultRoutes();
    AsyncWebServer* getServer() { return _server; }

private:
    WebServerManager();
    static WebServerManager* _instance;
    AsyncWebServer* _server;
    
    // Default route handlers
    static void handleRoot(AsyncWebServerRequest *request);
    static void handleTest(AsyncWebServerRequest *request);
    static void handlePing(AsyncWebServerRequest *request);
    static void handleServerTest(AsyncWebServerRequest *request);
};

#endif // WEB_SERVER_H