#include "web/web_interface.h"
#include <LittleFS.h>
#include <ElegantOTA.h>

WebInterface::WebInterface(ConfigManager* configManager, ThermostatController* thermostatController, MQTTInterface* mqttInterface)
    : server(80)
    , configManager(configManager)
    , thermostatController(thermostatController)
    , mqttInterface(mqttInterface) {
}

void WebInterface::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("An error occurred while mounting LittleFS");
        return;
    }

    // Setup server routes
    server.on("/", HTTP_GET, std::bind(&WebInterface::handleRoot, this, std::placeholders::_1));
    server.on("/save", HTTP_POST, std::bind(&WebInterface::handleSave, this, std::placeholders::_1));
    server.on("/save_config", HTTP_POST, std::bind(&WebInterface::handleSaveConfig, this, std::placeholders::_1));
    server.on("/setpoint", HTTP_POST, std::bind(&WebInterface::handleSetpoint, this, std::placeholders::_1));
    server.on("/status", HTTP_GET, std::bind(&WebInterface::handleGetStatus, this, std::placeholders::_1));
    server.on("/reboot", HTTP_POST, std::bind(&WebInterface::handleReboot, this, std::placeholders::_1));
    server.on("/factory_reset", HTTP_POST, std::bind(&WebInterface::handleFactoryReset, this, std::placeholders::_1));
    
    // Setup OTA updates
    ElegantOTA.begin(&server);
    
    // Setup static file handler
    server.onNotFound(std::bind(&WebInterface::handleNotFound, this, std::placeholders::_1));
    
    server.begin();
    Serial.println("HTTP server started");
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
        return true;
    }
    return false;
}

void WebInterface::addSecurityHeaders(AsyncWebServerResponse* response) {
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("X-Frame-Options", "DENY");
    response->addHeader("X-XSS-Protection", "1; mode=block");
    response->addHeader("Content-Security-Policy", "default-src 'self'");
}

void WebInterface::requestAuthentication(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(401);
    response->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
    request->send(response);
}

bool WebInterface::isAuthenticated(AsyncWebServerRequest* request) {
    if (!request->authenticate(configManager->getAdminUsername(), configManager->getAdminPassword())) {
        return false;
    }
    return true;
}

bool WebInterface::validateCSRFToken(AsyncWebServerRequest* request) {
    if (!request->hasHeader("X-CSRF-Token")) {
        return false;
    }
    
    String token = request->header("X-CSRF-Token");
    return token == generateCSRFToken();
}

String WebInterface::generateCSRFToken() {
    // Generate a random token using ESP32's hardware RNG
    uint8_t random[16];
    for (int i = 0; i < 16; i++) {
        random[i] = esp_random() & 0xFF;
    }
    
    // Convert to base64
    return base64::encode(random, 16);
}

String WebInterface::generateHtml() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>ESP32 Thermostat</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<link rel='stylesheet' type='text/css' href='/style.css'>";
    html += "</head><body>";
    
    // Add CSRF token to meta tag
    html += "<meta name='csrf-token' content='" + generateCSRFToken() + "'>";
    
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