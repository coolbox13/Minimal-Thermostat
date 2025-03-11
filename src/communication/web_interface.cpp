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
    , protocolManager(protocolManager) {
    ESP_LOGI(TAG, "Web interface initialized");
}

WebInterface::~WebInterface() {
    end();
}

void WebInterface::begin() {
    if (!LittleFS.begin()) {
        ESP_LOGE(TAG, "An error occurred while mounting LittleFS");
        return;
    }

    server.on("/", HTTP_GET, std::bind(&WebInterface::handleRoot, this, std::placeholders::_1));
    server.on("/save", HTTP_POST, std::bind(&WebInterface::handleSave, this, std::placeholders::_1));
    server.on("/status", HTTP_GET, std::bind(&WebInterface::handleGetStatus, this, std::placeholders::_1));
    server.on("/setpoint", HTTP_POST, std::bind(&WebInterface::handleSetpoint, this, std::placeholders::_1));
    server.on("/config", HTTP_POST, std::bind(&WebInterface::handleSaveConfig, this, std::placeholders::_1));
    server.on("/reboot", HTTP_POST, std::bind(&WebInterface::handleReboot, this, std::placeholders::_1));
    server.on("/reset", HTTP_POST, std::bind(&WebInterface::handleFactoryReset, this, std::placeholders::_1));
    server.onNotFound(std::bind(&WebInterface::handleNotFound, this, std::placeholders::_1));

    AsyncElegantOta.begin(&server);
    server.begin();

    if (!MDNS.begin("thermostat")) {
        ESP_LOGE(TAG, "Error setting up MDNS responder!");
        return;
    }

    ESP_LOGI(TAG, "HTTP server started");
}

void WebInterface::end() {
    server.end();
    ESP_LOGI(TAG, "Web interface stopped");
}

String WebInterface::generateCSRFToken(AsyncWebServerRequest *request) {
    uint8_t random[16];
    for(int i = 0; i < 16; i++) {
        random[i] = esp_random() & 0xFF;
    }
    return Base64::encode(random, 16);
}

String WebInterface::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}

bool WebInterface::handleFileRead(AsyncWebServerRequest* request, String path) {
    if (path.endsWith("/")) {
        path += "index.html";
    }
    
    String contentType;
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".ico")) contentType = "image/x-icon";
    else if (path.endsWith(".json")) contentType = "application/json";
    else contentType = "text/plain";
    
    if (LittleFS.exists(path)) {
        AsyncWebServerResponse* response = request->beginResponse(LittleFS, path, contentType);
        addSecurityHeaders(response);
        request->send(response);
        ESP_LOGD(TAG, "Serving file: %s", path.c_str());
        return true;
    }
    ESP_LOGW(TAG, "File not found: %s", path.c_str());
    return false;
}

void WebInterface::addSecurityHeaders(AsyncWebServerResponse* response) {
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("X-Frame-Options", "DENY");
    response->addHeader("X-XSS-Protection", "1; mode=block");
    response->addHeader("Strict-Transport-Security", "max-age=31536000; includeSubDomains");
    response->addHeader("Content-Security-Policy", "default-src 'self'");
    response->addHeader("Referrer-Policy", "same-origin");
}

void WebInterface::requestAuthentication(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(401);
    response->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
    request->send(response);
    ESP_LOGI(TAG, "Requesting authentication from IP: %s", request->client()->remoteIP().toString().c_str());
}

bool WebInterface::isAuthenticated(AsyncWebServerRequest* request) {
    if (configManager->getWebUsername()[0] == '\0') {
        return true;
    }

    if (!request->authenticate(configManager->getWebUsername(), configManager->getWebPassword())) {
        ESP_LOGW(TAG, "Authentication failed for IP: %s", request->client()->remoteIP().toString().c_str());
        return false;
    }

    return true;
}

bool WebInterface::validateCSRFToken(AsyncWebServerRequest* request) {
    if (!request->hasHeader("X-CSRF-Token")) {
        ESP_LOGW(TAG, "Missing CSRF token from IP: %s", request->client()->remoteIP().toString().c_str());
        return false;
    }
    
    String token = request->header("X-CSRF-Token");
    return token == generateCSRFToken(request);
}

String WebInterface::generateHtml() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>ESP32 Thermostat</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<link rel='stylesheet' type='text/css' href='/style.css'>";
    html += "</head><body>";
    
    // Add CSRF token to meta tag
    html += "<meta name='csrf-token' content='" + generateCSRFToken(nullptr) + "'>";
    
    // Add your HTML content here
    html += "<h1>ESP32 Thermostat</h1>";
    
    // Current temperature and setpoint
    html += "<div class='card'>";
    html += "<h2>Current Temperature: <span id='currentTemp'>--</span>°C</h2>";
    html += "<h2>Setpoint: <span id='setpoint'>--</span>°C</h2>";
    html += "</div>";
    
    // Mode selection
    html += "<div class='card'>";
    html += "<h2>Mode</h2>";
    html += "<select id='mode' onchange='updateMode()'>";
    html += "<option value='OFF'>Off</option>";
    html += "<option value='HEAT'>Heat</option>";
    html += "<option value='COOL'>Cool</option>";
    html += "<option value='AUTO'>Auto</option>";
    html += "</select>";
    html += "</div>";
    
    // Temperature control
    html += "<div class='card'>";
    html += "<h2>Temperature Control</h2>";
    html += "<button onclick='adjustTemp(-0.5)'>-</button>";
    html += "<span id='targetTemp'>--</span>°C";
    html += "<button onclick='adjustTemp(0.5)'>+</button>";
    html += "</div>";
    
    // Add JavaScript
    html += "<script src='/script.js'></script>";
    html += "</body></html>";
    
    return html;
}

void WebInterface::handleRoot(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    String html = generateHtml();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
    addSecurityHeaders(response);
    request->send(response);
}

void WebInterface::handleSave(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!validateCSRFToken(request)) {
        request->send(403, "text/plain", "Invalid CSRF token");
        return;
    }

    if (!configManager) {
        request->send(500, "text/plain", "Configuration manager not available");
        return;
    }

    // Handle device name
    if (request->hasParam("deviceName", true)) {
        configManager->setDeviceName(request->getParam("deviceName", true)->value().c_str());
    }

    // Handle send interval
    if (request->hasParam("sendInterval", true)) {
        uint32_t interval = request->getParam("sendInterval", true)->value().toInt();
        if (interval > 0) {
            sensorInterface->setUpdateInterval(interval);
        }
    }

    // Handle PID interval
    if (request->hasParam("pidInterval", true)) {
        uint32_t interval = request->getParam("pidInterval", true)->value().toInt();
        if (interval > 0) {
            pidController->setUpdateInterval(interval);
        }
    }

    // Handle KNX settings
    if (request->hasParam("knxArea", true) && request->hasParam("knxLine", true) && request->hasParam("knxMember", true)) {
        uint8_t area = request->getParam("knxArea", true)->value().toInt();
        uint8_t line = request->getParam("knxLine", true)->value().toInt();
        uint8_t member = request->getParam("knxMember", true)->value().toInt();
        configManager->setKnxPhysicalAddress(area, line, member);
    }

    configManager->setKnxEnabled(request->hasParam("knxEnabled", true));

    // Handle KNX temperature GA
    if (request->hasParam("knxTempArea", true) && request->hasParam("knxTempLine", true) && request->hasParam("knxTempMember", true)) {
        uint8_t area = request->getParam("knxTempArea", true)->value().toInt();
        uint8_t line = request->getParam("knxTempLine", true)->value().toInt();
        uint8_t member = request->getParam("knxTempMember", true)->value().toInt();
        configManager->setKnxTemperatureGA(area, line, member);
    }

    // Handle KNX setpoint GA
    if (request->hasParam("knxSetpointArea", true) && request->hasParam("knxSetpointLine", true) && request->hasParam("knxSetpointMember", true)) {
        uint8_t area = request->getParam("knxSetpointArea", true)->value().toInt();
        uint8_t line = request->getParam("knxSetpointLine", true)->value().toInt();
        uint8_t member = request->getParam("knxSetpointMember", true)->value().toInt();
        configManager->setKnxSetpointGA(area, line, member);
    }

    // Handle KNX valve GA
    if (request->hasParam("knxValveArea", true) && request->hasParam("knxValveLine", true) && request->hasParam("knxValveMember", true)) {
        uint8_t area = request->getParam("knxValveArea", true)->value().toInt();
        uint8_t line = request->getParam("knxValveLine", true)->value().toInt();
        uint8_t member = request->getParam("knxValveMember", true)->value().toInt();
        configManager->setKnxValveGA(area, line, member);
    }

    // Handle KNX mode GA
    if (request->hasParam("knxModeArea", true) && request->hasParam("knxModeLine", true) && request->hasParam("knxModeMember", true)) {
        uint8_t area = request->getParam("knxModeArea", true)->value().toInt();
        uint8_t line = request->getParam("knxModeLine", true)->value().toInt();
        uint8_t member = request->getParam("knxModeMember", true)->value().toInt();
        configManager->setKnxModeGA(area, line, member);
    }

    // Handle MQTT settings
    configManager->setMQTTEnabled(request->hasParam("mqttEnabled", true));

    if (request->hasParam("mqttServer", true)) {
        configManager->setMQTTServer(request->getParam("mqttServer", true)->value().c_str());
    }

    if (request->hasParam("mqttPort", true)) {
        uint16_t port = request->getParam("mqttPort", true)->value().toInt();
        configManager->setMQTTPort(port);
    }

    if (request->hasParam("mqttUser", true)) {
        configManager->setMQTTUser(request->getParam("mqttUser", true)->value().c_str());
    }

    if (request->hasParam("mqttPassword", true)) {
        configManager->setMQTTPassword(request->getParam("mqttPassword", true)->value().c_str());
    }

    if (request->hasParam("mqttClientId", true)) {
        configManager->setMQTTClientId(request->getParam("mqttClientId", true)->value().c_str());
    }

    // Save configuration
    configManager->saveConfig();

    request->send(200, "text/plain", "Settings saved");
}

void WebInterface::handleGetStatus(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!thermostatState) {
        request->send(500, "text/plain", "Internal server error");
        return;
    }

    StaticJsonDocument<512> doc;
    doc["temperature"] = thermostatState->getCurrentTemperature();
    doc["humidity"] = thermostatState->getCurrentHumidity();
    doc["pressure"] = thermostatState->getCurrentPressure();
    doc["setpoint"] = thermostatState->getTargetTemperature();
    doc["mode"] = thermostatState->getMode();
    doc["error"] = thermostatState->getStatus();

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse *jsonResponse = request->beginResponse(200, "application/json", response);
    addSecurityHeaders(jsonResponse);
    request->send(jsonResponse);
}

void WebInterface::handleSetpoint(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!validateCSRFToken(request)) {
        request->send(403, "text/plain", "Invalid CSRF token");
        return;
    }

    if (!request->hasParam("setpoint", true)) {
        request->send(400, "text/plain", "Missing setpoint parameter");
        return;
    }

    float setpoint = request->getParam("setpoint", true)->value().toFloat();
    thermostatState->setTargetTemperature(setpoint);
    configManager->setSetpoint(setpoint);
    configManager->saveConfig();

    request->send(200, "text/plain", "Setpoint updated");
}

void WebInterface::handleSaveConfig(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!validateCSRFToken(request)) {
        request->send(403, "text/plain", "Invalid CSRF token");
        return;
    }

    if (!request->hasParam("plain", true)) {
        request->send(400, "text/plain", "Missing configuration data");
        return;
    }

    String json = request->getParam("plain", true)->value();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
    }

    // Apply configuration
    if (doc.containsKey("deviceName")) {
        configManager->setDeviceName(doc["deviceName"]);
    }

    // Save configuration
    configManager->saveConfig();

    request->send(200, "text/plain", "Configuration saved");
}

void WebInterface::handleReboot(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Device will reboot in 5 seconds...");
    addSecurityHeaders(response);
    request->send(response);

    delay(5000);
    ESP.restart();
}

void WebInterface::handleFactoryReset(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!validateCSRFToken(request)) {
        request->send(403, "text/plain", "Invalid CSRF token");
        return;
    }

    // Perform factory reset
    configManager->resetToDefaults();
    
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Factory reset complete. Device will reboot in 5 seconds...");
    addSecurityHeaders(response);
    request->send(response);

    delay(5000);
    ESP.restart();
}

void WebInterface::handleNotFound(AsyncWebServerRequest *request) {
    if (!handleFileRead(request, request->url())) {
        request->send(404, "text/plain", "File Not Found");
    }
}

// Add a basic WebInterface::loop implementation
void WebInterface::loop() {
    // Handle OTA updates if initialized
    if (otaInitialized) {
        elegantOTA.loop();
    }
    
    // Perform any other periodic tasks here
    
    // Process incoming requests (this is actually handled by ESPAsyncWebServer in the background)
}