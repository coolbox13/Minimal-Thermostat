#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "thermostat_state.h"
#include "config_manager.h"
#include "sensor_interface.h"
#include "knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"
#include <LittleFS.h>

class WebInterface {
public:
    WebInterface();
    bool begin(ThermostatState* state, 
               ConfigManager* config, 
               SensorInterface* sensor,
               KNXInterface* knx,
               MQTTInterface* mqtt,
               PIDController* pid);
    void handle();

private:
    // References to components
    ThermostatState* thermostatState;
    ConfigManager* configManager;
    SensorInterface* sensorInterface;
    KNXInterface* knxInterface;
    MQTTInterface* mqttInterface;
    PIDController* pidController;
    
    // WebServer instance
    WebServer server;

    // HTTP request handlers
    void handleRoot();
    void handleGetStatus();
    void handleSetpoint();
    void handleSaveConfig();
    void handleReboot();
    void handleFactoryReset();
    void handleNotFound();
  
    // Setup MDNS
    void setupMDNS();
    
    // Serve static files from LittleFS
    bool handleFileRead(String path);
    
    // Authentication helpers
    bool isAuthenticated();
    void requestAuthentication();
    
    // Rate limiting
    unsigned long lastRequestTime;
    int requestCount;
    static const int MAX_REQUESTS_PER_MINUTE = 60;
};

#endif // WEB_INTERFACE_H