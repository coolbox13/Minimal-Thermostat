#include "config_manager.h"
#include <ESPAsyncWiFiManager.h>
#include <DNSServer.h>
#include <esp_log.h>

static const char* CONFIG_TAG = "ConfigManager";

bool ConfigManager::setupWiFi() {
    AsyncWebServer server(80);
    DNSServer dns;
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.setConfigPortalTimeout(180); // Auto-exit config portal after 3 minutes
    
    // Try to connect to WiFi
    if (!wifiManager.autoConnect(getDeviceName())) {
        ESP_LOGE(CONFIG_TAG, "Failed to connect to WiFi - restarting");
        delay(1000);
        ESP.restart();
        return false;
    }
    
    ESP_LOGI(CONFIG_TAG, "Connected to WiFi");
    ESP_LOGI(CONFIG_TAG, "IP Address: %s", WiFi.localIP().toString().c_str());
    
    return true;
} 