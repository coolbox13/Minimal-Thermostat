#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <ElegantOTA.h>
#include "config_manager.h"
#include "thermostat_state.h"
#include "sensor_interface.h"
#include "communication/knx/knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"
#include "protocol_manager.h"
#include "thermostat_controller.h"
#include <memory>
#include <functional>

// HTTP method constants
#ifndef HTTP_GET
#define HTTP_GET 0
#endif
#ifndef HTTP_POST
#define HTTP_POST 1
#endif
#ifndef HTTP_PUT
#define HTTP_PUT 2
#endif
#ifndef HTTP_DELETE
#define HTTP_DELETE 3
#endif

// Base64 encoding helper
class Base64 {
public:
    static String encode(const String& data) {
        static const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        size_t in_len = data.length();
        String out;
        out.reserve(4 * ((in_len + 2) / 3));
        int i = 0;
        while (i < in_len) {
            uint32_t octet_a = i < in_len ? data[i++] : 0;
            uint32_t octet_b = i < in_len ? data[i++] : 0;
            uint32_t octet_c = i < in_len ? data[i++] : 0;
            uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
            out += ALPHABET[(triple >> 18) & 0x3F];
            out += ALPHABET[(triple >> 12) & 0x3F];
            out += ALPHABET[(triple >> 6) & 0x3F];
            out += ALPHABET[triple & 0x3F];
        }
        switch (in_len % 3) {
            case 1: out[out.length() - 2] = '=';
            case 2: out[out.length() - 1] = '=';
        }
        return out;
    }
};

// JSON converter for ThermostatMode
namespace ArduinoJson {
namespace V6215PB2 {
template <>
struct Converter<ThermostatMode> {
    static void toJson(const ThermostatMode& src, JsonVariant dst) {
        switch(src) {
            case ThermostatMode::OFF: dst.set("OFF"); break;
            case ThermostatMode::HEAT: dst.set("HEAT"); break;
            case ThermostatMode::COOL: dst.set("COOL"); break;
            case ThermostatMode::AUTO: dst.set("AUTO"); break;
            default: dst.set("UNKNOWN"); break;
        }
    }
    
    static ThermostatMode fromJson(JsonVariantConst src) {
        const char* str = src.as<const char*>();
        if (!str) return ThermostatMode::OFF;
        if (strcmp(str, "HEAT") == 0) return ThermostatMode::HEAT;
        if (strcmp(str, "COOL") == 0) return ThermostatMode::COOL;
        if (strcmp(str, "AUTO") == 0) return ThermostatMode::AUTO;
        return ThermostatMode::OFF;
    }
};
}
}

class WebInterface {
public:
    WebInterface(ConfigManager* configManager, ThermostatController* thermostatController, MQTTInterface* mqttInterface);
    
    void begin();
    
    // Request handlers
    void handleRoot(AsyncWebServerRequest* request);
    void handleSave(AsyncWebServerRequest* request);
    void handleSaveConfig(AsyncWebServerRequest* request);
    void handleSetpoint(AsyncWebServerRequest* request);
    void handleReboot(AsyncWebServerRequest* request);
    void handleFactoryReset(AsyncWebServerRequest* request);
    void handleNotFound(AsyncWebServerRequest* request);
    void handleGetStatus(AsyncWebServerRequest* request);
    
    // File handling
    bool handleFileRead(AsyncWebServerRequest* request, String path);
    
    // Security helpers
    bool isAuthenticated(AsyncWebServerRequest* request);
    void requestAuthentication(AsyncWebServerRequest* request);
    void addSecurityHeaders(AsyncWebServerResponse* response);
    bool validateCSRFToken(AsyncWebServerRequest* request);
    String generateCSRFToken();
    
    // HTML generation
    String generateHtml();

private:
    AsyncWebServer server;
    ConfigManager* configManager;
    ThermostatController* thermostatController;
    MQTTInterface* mqttInterface;
    
    // HTML generation
    String generateHtml();
};

#endif // WEB_INTERFACE_H 