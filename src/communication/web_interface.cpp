#include "web_interface.h"
#include "config_manager.h"
#include "thermostat_state.h"
#include "sensor_interface.h"
#include "knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"
#include "protocol_manager.h"
#include <ArduinoJson.h>

WebInterface::WebInterface() : server(80) {
}

bool WebInterface::begin(ThermostatState* state, 
                        ConfigManager* config, 
                        SensorInterface* sensor,
                        KNXInterface* knx,
                        MQTTInterface* mqtt,
                        PIDController* pid,
                        ProtocolManager* protocol) {
    thermostatState = state;
    configManager = config;
    sensorInterface = sensor;
    knxInterface = knx;
    mqttInterface = mqtt;
    pidController = pid;
    protocolManager = protocol;
    
    // Create auth manager
    authManager = std::make_unique<WebAuthManager>(server, *configManager);
    
    // Setup routes
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/config", HTTP_POST, [this]() { handleSaveConfig(); });
    server.on("/setpoint", HTTP_POST, [this]() { handleSetpoint(); });
    server.on("/reboot", HTTP_POST, [this]() { handleReboot(); });
    server.on("/factory_reset", HTTP_POST, [this]() { handleFactoryReset(); });
    server.on("/status", HTTP_GET, [this]() { handleGetStatus(); });
    
    // Handle file not found
    server.onNotFound([this]() { handleNotFound(); });
    
    // Start server
    server.begin();
    
    // Setup MDNS
    setupMDNS();
    
    return true;
}

void WebInterface::handle() {
    server.handleClient();
}

void WebInterface::handleRoot() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    addSecurityHeaders();
    String html = generateHtml();
    server.send(200, "text/html", html);
}

void WebInterface::handleSave() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    if (!validateCSRFToken()) {
        server.send(403, "text/plain", "Invalid CSRF token");
        return;
    }
    
    addSecurityHeaders();
    
    // ... rest of handleSave implementation ...
}

void WebInterface::handleSaveConfig() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    if (!validateCSRFToken()) {
        server.send(403, "text/plain", "Invalid CSRF token");
        return;
    }
    
    addSecurityHeaders();
    
    // ... rest of handleSaveConfig implementation ...
}

void WebInterface::handleSetpoint() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    if (!validateCSRFToken()) {
        server.send(403, "text/plain", "Invalid CSRF token");
        return;
    }
    
    addSecurityHeaders();
    
    // ... rest of handleSetpoint implementation ...
}

void WebInterface::handleReboot() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    if (!validateCSRFToken()) {
        server.send(403, "text/plain", "Invalid CSRF token");
        return;
    }
    
    addSecurityHeaders();
    
    // ... rest of handleReboot implementation ...
}

void WebInterface::handleFactoryReset() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    if (!validateCSRFToken()) {
        server.send(403, "text/plain", "Invalid CSRF token");
        return;
    }
    
    addSecurityHeaders();
    
    // ... rest of handleFactoryReset implementation ...
}

void WebInterface::handleGetStatus() {
    if (!isAuthenticated()) {
        requestAuthentication();
        return;
    }
    
    addSecurityHeaders();
    
    // ... rest of handleGetStatus implementation ...
}

void WebInterface::handleNotFound() {
    if (!handleFileRead(server.uri())) {
        server.send(404, "text/plain", "File Not Found");
    }
}

String WebInterface::generateHtml() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>KNX Thermostat</title>";
    
    // Add CSRF token as meta tag
    String csrfToken = generateCSRFToken();
    html += "<meta name='csrf-token' content='" + csrfToken + "'>";
    
    // Add JavaScript to automatically add CSRF token to forms
    html += "<script>";
    html += "document.addEventListener('DOMContentLoaded', function() {";
    html += "  var token = document.querySelector('meta[name=\"csrf-token\"]').getAttribute('content');";
    html += "  var forms = document.getElementsByTagName('form');";
    html += "  for (var i = 0; i < forms.length; i++) {";
    html += "    var input = document.createElement('input');";
    html += "    input.type = 'hidden';";
    html += "    input.name = 'csrf_token';";
    html += "    input.value = token;";
    html += "    forms[i].appendChild(input);";
    html += "  }";
    html += "});";
    html += "</script>";
    
    // Add styles
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }";
    html += ".container { max-width: 800px; margin: 0 auto; }";
    html += ".card { background: #fff; border-radius: 8px; padding: 20px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
    html += "h1 { color: #333; }";
    html += "label { display: block; margin-bottom: 5px; }";
    html += "input[type=text], input[type=number], input[type=password] { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 4px; }";
    html += "button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; }";
    html += "button:hover { background: #0056b3; }";
    html += "</style>";
    html += "</head><body>";
    
    // Add content
    html += "<div class='container'>";
    
    // ... rest of generateHtml implementation ...
    
    html += "</div></body></html>";
    return html;
}

bool WebInterface::isAuthenticated() {
    return authManager->isAuthenticated();
}

void WebInterface::requestAuthentication() {
    authManager->requestAuthentication();
}

void WebInterface::addSecurityHeaders() {
    authManager->addSecurityHeaders();
}

bool WebInterface::validateCSRFToken() {
    if (!server.hasHeader("X-CSRF-Token")) {
        return false;
    }
    return authManager->validateCSRFToken(server.header("X-CSRF-Token"));
}

String WebInterface::generateCSRFToken() {
    String token = authManager->generateCSRFToken();
    server.sendHeader("X-CSRF-Token", token);
    return token;
}

// ... existing code for other methods ...