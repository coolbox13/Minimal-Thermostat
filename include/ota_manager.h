#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Update.h>
#include "web_server.h"

class OTAManager {
public:
    OTAManager();
    
    // Initialize OTA with WebServerManager
    void begin(WebServerManager* webServerManager);

private:
    // No need to store server pointer as we use WebServerManager
};

#endif // OTA_MANAGER_H