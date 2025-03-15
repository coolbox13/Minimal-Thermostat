#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

class OTAManager {
public:
    OTAManager();
    
    // Initialize OTA with existing web server
    void begin(AsyncWebServer* server);

private:
    AsyncWebServer* _server;
};

#endif // OTA_MANAGER_H