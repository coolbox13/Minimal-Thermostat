#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "thermostat_types.h"
#include <Arduino.h>

class WebInterface {
public:
    virtual ~WebInterface() = default;

    // Core functionality
    virtual bool begin() = 0;
    virtual void loop() = 0;
    virtual bool isConnected() const = 0;

    // Configuration
    virtual void setPort(uint16_t port) = 0;
    virtual void setCredentials(const char* username, const char* password) = 0;
    virtual void setHostname(const char* hostname) = 0;

    // Status
    virtual ThermostatStatus getLastError() const = 0;

    // API endpoints that must be implemented
    virtual void handleRoot() = 0;
    virtual void handleSave() = 0;
    virtual void handleSetpoint() = 0;
    virtual void handleMode() = 0;
    virtual void handleStatus() = 0;
    virtual void handleConfig() = 0;
    virtual void handleReboot() = 0;
    virtual void handleReset() = 0;
    virtual void handleNotFound() = 0;

protected:
    // Helper methods that implementations should provide
    virtual bool isAuthenticated() = 0;
    virtual void requestAuthentication() = 0;
    virtual String generateHtml() = 0;
    virtual bool handleFileRead(String path) = 0;
};

#endif // WEB_INTERFACE_H 