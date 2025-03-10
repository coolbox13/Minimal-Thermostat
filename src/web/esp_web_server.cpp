#include "web/esp_web_server.h"
#include <ArduinoJson.h>

ESPWebServer::ESPWebServer(uint16_t serverPort)
    : server(serverPort)
    , port(serverPort)
    , lastError(ThermostatStatus::OK)
    , initialized(false)
    , thermostatState(nullptr)
    , configManager(nullptr)
    , pidController(nullptr)
    , knxInterface(nullptr)
    , mqttInterface(nullptr) {
    username[0] = '\0';
    password[0] = '\0';
    strcpy(hostname, "thermostat");
}

bool ESPWebServer::begin() {
    if (!FileFS.begin()) {
        lastError = ThermostatStatus::FILESYSTEM_ERROR;
        return false;
    }

    setupMDNS();
    setupRoutes();
    server.begin();
    initialized = true;
    return true;
}

void ESPWebServer::loop() {
    if (initialized) {
        server.handleClient();
    }
}

bool ESPWebServer::isConnected() const {
    return initialized && WiFi.status() == WL_CONNECTED;
}

void ESPWebServer::setPort(uint16_t newPort) {
    port = newPort;
}

void ESPWebServer::setCredentials(const char* user, const char* pass) {
    strncpy(username, user, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';
    strncpy(password, pass, sizeof(password) - 1);
    password[sizeof(password) - 1] = '\0';
}

void ESPWebServer::setHostname(const char* name) {
    strncpy(hostname, name, sizeof(hostname) - 1);
    hostname[sizeof(hostname) - 1] = '\0';
}

ThermostatStatus ESPWebServer::getLastError() const {
    return lastError;
}

void ESPWebServer::registerComponents(
    ThermostatState* state,
    ConfigInterface* config,
    ControlInterface* control,
    ProtocolInterface* knx,
    ProtocolInterface* mqtt) {
    thermostatState = state;
    configManager = config;
    pidController = control;
    knxInterface = knx;
    mqttInterface = mqtt;
}

bool ESPWebServer::isAuthenticated() {
    if (strlen(username) == 0) return true;  // No authentication required
    return server.authenticate(username, password);
}

void ESPWebServer::requestAuthentication() {
    server.requestAuthentication();
}

void ESPWebServer::setupMDNS() {
    if (MDNS.begin(hostname)) {
        MDNS.addService("http", "tcp", port);
    }
}

void ESPWebServer::setupRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/setpoint", HTTP_POST, [this]() { handleSetpoint(); });
    server.on("/mode", HTTP_POST, [this]() { handleMode(); });
    server.on("/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/config", HTTP_GET, [this]() { handleConfig(); });
    server.on("/reboot", HTTP_POST, [this]() { handleReboot(); });
    server.on("/reset", HTTP_POST, [this]() { handleReset(); });
    
    // Handle file reads for static content
    server.onNotFound([this]() {
        if (!handleFileRead(server.uri())) {
            handleNotFound();
        }
    });
}

String ESPWebServer::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    return "text/plain";
}

bool ESPWebServer::handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    String contentType = getContentType(path);
    
    if (FileFS.exists(path)) {
        File file = FileFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

void ESPWebServer::handleJsonResponse(String& json) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}

void ESPWebServer::handleError(const char* message, int code) {
    DynamicJsonDocument doc(128);
    doc["error"] = message;
    String response;
    serializeJson(doc, response);
    server.send(code, "application/json", response);
}

// API Endpoint Implementations
void ESPWebServer::handleRoot() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    String html = generateHtml();
    server.send(200, "text/html", html);
}

void ESPWebServer::handleStatus() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    String json = generateStatusJson();
    handleJsonResponse(json);
}

void ESPWebServer::handleConfig() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    String json = generateConfigJson();
    handleJsonResponse(json);
}

void ESPWebServer::handleSave() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }

    if (!configManager) {
        handleError("Configuration manager not available", 500);
        return;
    }

    // Parse configuration from POST data
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        handleError("Invalid JSON", 400);
        return;
    }

    // Update configuration
    if (doc.containsKey("deviceName")) {
        configManager->setDeviceName(doc["deviceName"]);
    }
    // Add more configuration updates as needed

    // Save configuration
    if (!configManager->save()) {
        handleError("Failed to save configuration", 500);
        return;
    }

    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void ESPWebServer::handleSetpoint() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }

    if (!thermostatState) {
        handleError("Thermostat state not available", 500);
        return;
    }

    if (!server.hasArg("value")) {
        handleError("Missing setpoint value", 400);
        return;
    }

    float setpoint = server.arg("value").toFloat();
    thermostatState->setTargetTemperature(setpoint);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void ESPWebServer::handleMode() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }

    if (!thermostatState) {
        handleError("Thermostat state not available", 500);
        return;
    }

    if (!server.hasArg("mode")) {
        handleError("Missing mode value", 400);
        return;
    }

    String modeStr = server.arg("mode");
    ThermostatMode mode;
    
    if (modeStr == "off") mode = ThermostatMode::OFF;
    else if (modeStr == "comfort") mode = ThermostatMode::COMFORT;
    else if (modeStr == "eco") mode = ThermostatMode::ECO;
    else if (modeStr == "away") mode = ThermostatMode::AWAY;
    else if (modeStr == "boost") mode = ThermostatMode::BOOST;
    else if (modeStr == "antifreeze") mode = ThermostatMode::ANTIFREEZE;
    else {
        handleError("Invalid mode value", 400);
        return;
    }

    thermostatState->setMode(mode);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void ESPWebServer::handleReboot() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    server.send(200, "application/json", "{\"status\":\"ok\"}");
    delay(500);
    ESP.restart();
}

void ESPWebServer::handleReset() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }

    if (!configManager) {
        handleError("Configuration manager not available", 500);
        return;
    }

    configManager->reset();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
    delay(500);
    ESP.restart();
}

void ESPWebServer::handleNotFound() {
    server.send(404, "text/plain", "File Not Found");
}

String ESPWebServer::generateHtml() {
    return HtmlGenerator::generatePage(thermostatState, configManager, pidController);
}

String ESPWebServer::generateStatusJson() {
    DynamicJsonDocument doc(512);
    
    if (thermostatState) {
        doc["temperature"] = thermostatState->getCurrentTemperature();
        doc["humidity"] = thermostatState->getCurrentHumidity();
        doc["pressure"] = thermostatState->getCurrentPressure();
        doc["setpoint"] = thermostatState->getTargetTemperature();
        doc["valve"] = thermostatState->getValvePosition();
        doc["mode"] = thermostatState->getMode();
        doc["heating"] = thermostatState->isHeating();
    }

    if (pidController) {
        doc["pid"]["active"] = pidController->isActive();
        doc["pid"]["output"] = pidController->getOutput();
    }

    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["ip"] = WiFi.localIP().toString();

    String response;
    serializeJson(doc, response);
    return response;
}

String ESPWebServer::generateConfigJson() {
    DynamicJsonDocument doc(1024);
    
    if (configManager) {
        doc["device"]["name"] = configManager->getDeviceName();
        doc["device"]["interval"] = configManager->getSendInterval();
        
        doc["web"]["username"] = configManager->getWebUsername();
        // Don't send password
        
        doc["pid"]["kp"] = configManager->getKp();
        doc["pid"]["ki"] = configManager->getKi();
        doc["pid"]["kd"] = configManager->getKd();
        doc["pid"]["setpoint"] = configManager->getSetpoint();
        
        doc["knx"]["enabled"] = configManager->getKnxEnabled();
        doc["mqtt"]["enabled"] = configManager->getMqttEnabled();
    }

    String response;
    serializeJson(doc, response);
    return response;
} 