#include "web/esp_web_server.h"
#include "html_generator.h"
#include <LittleFS.h>
#include <esp_log.h>

static const char* TAG = "ESPWebServer";

ESPWebServer::ESPWebServer(ConfigManager* configManager, ThermostatState* state)
    : server(80)
    , configManager(configManager)
    , thermostatState(state)
    , pidController(nullptr)
    , port(80)
    , lastError(ThermostatStatus::OK) {
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(hostname, 0, sizeof(hostname));
}

bool ESPWebServer::begin() {
    ESP_LOGI(TAG, "Starting web server...");
    
    // Initialize SPIFFS
    // Note: LittleFS is now initialized in main.cpp
    
    // Setup routes
    setupRoutes();
    
    // Start server
    server.begin();
    ESP_LOGI(TAG, "Web server started");
    return true;
}

void ESPWebServer::setupRoutes() {
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        String html = generateHtml();
        AsyncWebServerResponse* response = request->beginResponse(200, "text/html", html);
        request->send(response);
    });

    server.on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        handleSave();
    });

    server.on("/setpoint", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        if (!request->hasParam("value", true)) {
            request->send(400, "application/json", "{\"error\":\"Missing value parameter\"}");
            return;
        }
        float setpoint = request->getParam("value", true)->value().toFloat();
        thermostatState->setTargetTemperature(setpoint);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/mode", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        if (!request->hasParam("mode", true)) {
            request->send(400, "application/json", "{\"error\":\"Missing mode parameter\"}");
            return;
        }
        String modeStr = request->getParam("mode", true)->value();
        ThermostatMode mode;
        if (modeStr == "off") mode = ThermostatMode::OFF;
        else if (modeStr == "comfort") mode = ThermostatMode::COMFORT;
        else if (modeStr == "eco") mode = ThermostatMode::ECO;
        else if (modeStr == "away") mode = ThermostatMode::AWAY;
        else if (modeStr == "boost") mode = ThermostatMode::BOOST;
        else if (modeStr == "antifreeze") mode = ThermostatMode::ANTIFREEZE;
        else {
            request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
            return;
        }
        thermostatState->setMode(mode);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        String json = generateStatusJson();
        request->send(200, "application/json", json);
    });

    server.on("/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        String json = generateConfigJson();
        request->send(200, "application/json", json);
    });

    server.on("/reboot", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        ESP.restart();
    });

    server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated()) {
            requestAuthentication();
            return;
        }
        configManager->resetToDefaults();
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        ESP.restart();
    });

    server.onNotFound([this](AsyncWebServerRequest* request) {
        if (!handleFileRead(request->url())) {
            request->send(404, "text/plain", "File Not Found");
        }
    });
}

bool ESPWebServer::handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    String contentType = getContentType(path);
    
    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        if (file) {
            AsyncWebServerResponse* response = new AsyncFileResponse(file, contentType);
            response->addHeader("Cache-Control", "no-cache");
            return true;
        }
        file.close();
    }
    return false;
}

String ESPWebServer::getContentType(const String &filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}

void ESPWebServer::setPort(uint16_t port) {
    this->port = port;
}

void ESPWebServer::setCredentials(const char* user, const char* pass) {
    strncpy(username, user, sizeof(username) - 1);
    strncpy(password, pass, sizeof(password) - 1);
}

void ESPWebServer::setHostname(const char* name) {
    strncpy(hostname, name, sizeof(hostname) - 1);
}

void ESPWebServer::registerComponents(ThermostatState* state, PIDController* pid) {
    thermostatState = state;
    pidController = pid;
}

bool ESPWebServer::isAuthenticated() {
    // TODO: Implement proper authentication
    return true;
}

void ESPWebServer::requestAuthentication() {
    // TODO: Implement proper authentication
}

String ESPWebServer::generateHtml() {
    return HtmlGenerator::generatePage(thermostatState, static_cast<ConfigInterface*>(configManager), pidController);
}

String ESPWebServer::generateStatusJson() {
    StaticJsonDocument<512> doc;
    
    doc["temperature"] = thermostatState->getCurrentTemperature();
    doc["humidity"] = thermostatState->getCurrentHumidity();
    doc["pressure"] = thermostatState->getCurrentPressure();
    doc["setpoint"] = thermostatState->getTargetTemperature();
    doc["valve"] = thermostatState->getValvePosition();
    doc["heating"] = thermostatState->isHeating();
    doc["mode"] = static_cast<int>(thermostatState->getMode());
    
    String json;
    serializeJson(doc, json);
    return json;
}

String ESPWebServer::generateConfigJson() {
    StaticJsonDocument<1024> doc;
    
    // Device configuration
    doc["device"]["name"] = configManager->getDeviceName();
    doc["device"]["sendInterval"] = configManager->getSendInterval();
    
    // Web interface configuration
    doc["web"]["username"] = configManager->getWebUsername();
    doc["web"]["password"] = configManager->getWebPassword();
    
    // PID configuration
    if (pidController) {
        doc["pid"]["kp"] = pidController->getKp();
        doc["pid"]["ki"] = pidController->getKi();
        doc["pid"]["kd"] = pidController->getKd();
        doc["pid"]["active"] = pidController->isActive();
    }
    
    // Protocol configuration
    doc["knx"]["enabled"] = configManager->getKnxEnabled();
    doc["mqtt"]["enabled"] = configManager->getMqttEnabled();
    
    String json;
    serializeJson(doc, json);
    return json;
}

void ESPWebServer::handleSave() {
    // This method is called from a lambda in setupRoutes
    // In the ESPAsyncWebServer architecture, the actual implementation is different
    // This stub is here to satisfy the linker
} 