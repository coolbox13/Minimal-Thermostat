#include "web/web_interface.h"
#include <LittleFS.h>
#include "esp_log.h"
#include <ESPmDNS.h>
#include "web/base64.h"
#include "interfaces/sensor_interface.h"
#include "web/elegant_ota_async.h"

static const char* TAG = "WebInterface";

WebInterface::WebInterface(ConfigManager* configManager, SensorInterface* sensorInterface, 
                          PIDController* pidController, ThermostatState* thermostatState,
                          ProtocolManager* protocolManager)
    : server(80)
    , configManager(configManager)
    , sensorInterface(sensorInterface)
    , pidController(pidController)
    , thermostatState(thermostatState)
    , protocolManager(protocolManager)
    , otaInitialized(false) {
    ESP_LOGI(TAG, "Web interface initialized");
}

WebInterface::~WebInterface() {
    end();
}

bool WebInterface::begin() {
    ESP_LOGI(TAG, "Starting web interface...");
    
    // Note: LittleFS is now initialized in main.cpp
    
    try {
        // Initialize web server
        server.begin();
        ESP_LOGI(TAG, "Web interface started successfully");
        return true;
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Failed to start web interface: %s", e.what());
        return false;
    } catch (...) {
        ESP_LOGE(TAG, "Failed to start web interface: Unknown error");
        return false;
    }
}

void WebInterface::end() {
    server.end();
    ESP_LOGI(TAG, "Web interface stopped");
}

void WebInterface::loop() {
    // AsyncWebServer doesn't need explicit loop handling
}