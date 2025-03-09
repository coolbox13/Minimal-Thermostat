#include "web/web_interface.h"
#include "config_manager.h"
#include "thermostat_state.h"
#include "interfaces/sensor_interface.h"
#include "communication/knx/knx_interface.h"
#include "mqtt_interface.h"
#include "pid_controller.h"
#include "protocol_manager.h"
#include <ArduinoJson.h>
#include <memory>
#include <AsyncElegantOTA.h>

// Define make_unique for C++11 compatibility
#if __cplusplus < 201402L
namespace std {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

WebInterface::WebInterface() 
    : server(80)
    , wifiManager(&server, &dns) {
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

    // Setup handlers
    server.on("/", HTTP_GET, std::bind(&WebInterface::handleRoot, this, std::placeholders::_1));
    server.on("/save", HTTP_POST, std::bind(&WebInterface::handleSave, this, std::placeholders::_1));
    server.on("/config", HTTP_POST, std::bind(&WebInterface::handleSaveConfig, this, std::placeholders::_1));
    server.on("/setpoint", HTTP_POST, std::bind(&WebInterface::handleSetpoint, this, std::placeholders::_1));
    server.on("/reboot", HTTP_POST, std::bind(&WebInterface::handleReboot, this, std::placeholders::_1));
    server.on("/factory_reset", HTTP_POST, std::bind(&WebInterface::handleFactoryReset, this, std::placeholders::_1));
    server.on("/status", HTTP_GET, std::bind(&WebInterface::handleGetStatus, this, std::placeholders::_1));
    
    server.onNotFound(std::bind(&WebInterface::handleNotFound, this, std::placeholders::_1));

    // Initialize OTA updates
    AsyncElegantOTA.begin(&server);

    // Start server
    server.begin();
    
    return true;
}

void WebInterface::handle() {
    // AsyncWebServer doesn't need explicit handling
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

    // Process form data
    if (request->hasParam("deviceName", true)) {
        configManager->setDeviceName(request->getParam("deviceName", true)->value().c_str());
    }

    // Save configuration
    configManager->saveConfig();

    // Redirect back to root
    AsyncWebServerResponse *response = request->beginResponse(302);
    response->addHeader("Location", "/");
    addSecurityHeaders(response);
    request->send(response);
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

    // Process JSON data
    if (request->hasParam("plain", true)) {
        String jsonStr = request->getParam("plain", true)->value();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, jsonStr);

        if (error) {
            request->send(400, "text/plain", "Invalid JSON");
            return;
        }

        // Update configuration
        if (doc.containsKey("webUsername")) {
            configManager->setWebUsername(doc["webUsername"].as<const char*>());
        }

        if (doc.containsKey("webPassword")) {
            configManager->setWebPassword(doc["webPassword"].as<const char*>());
        }

        // Save configuration
        configManager->saveConfig();
        request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        request->send(400, "text/plain", "Missing configuration data");
    }
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

    if (request->hasParam("plain", true)) {
        String jsonStr = request->getParam("plain", true)->value();
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, jsonStr);

        if (error) {
            request->send(400, "text/plain", "Invalid JSON");
            return;
        }

        if (!doc.containsKey("setpoint")) {
            request->send(400, "text/plain", "Missing setpoint value");
            return;
        }

        float setpoint = doc["setpoint"];
        if (protocolManager) {
            protocolManager->handleIncomingCommand(
                CommandSource::SOURCE_WEB_API,
                CommandType::CMD_SETPOINT,
                setpoint
            );
            request->send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            request->send(500, "text/plain", "Protocol manager not initialized");
        }
    } else {
        request->send(400, "text/plain", "Missing request data");
    }
}

void WebInterface::handleReboot(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!validateCSRFToken(request)) {
        request->send(403, "text/plain", "Invalid CSRF token");
        return;
    }

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Device is rebooting...");
    addSecurityHeaders(response);
    request->send(response);

    // Schedule reboot
    delay(500);
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

    if (configManager) {
        configManager->factoryReset();
        configManager->saveConfig();
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", 
            "{\"status\":\"ok\",\"message\":\"Factory reset complete\"}");
        addSecurityHeaders(response);
        request->send(response);

        // Schedule reboot
        delay(500);
        ESP.restart();
    } else {
        request->send(500, "text/plain", "Configuration manager not available");
    }
}

void WebInterface::handleGetStatus(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
        requestAuthentication(request);
        return;
    }

    if (!thermostatState || !sensorInterface) {
        request->send(500, "text/plain", "Thermostat state or sensor interface not available");
        return;
    }

    StaticJsonDocument<512> doc;
    
    doc["currentTemp"] = sensorInterface->getTemperature();
    doc["targetTemp"] = thermostatState->getTargetTemperature();
    doc["humidity"] = sensorInterface->getHumidity();
    doc["heating"] = thermostatState->isHeating();
    doc["mode"] = thermostatState->getMode();
    
    if (pidController) {
        doc["pidOutput"] = pidController->getOutput();
    }

    String response;
    serializeJson(doc, response);
    
    AsyncWebServerResponse *jsonResponse = request->beginResponse(200, "application/json", response);
    addSecurityHeaders(jsonResponse);
    request->send(jsonResponse);
}

void WebInterface::handleNotFound(AsyncWebServerRequest *request) {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";
    
    for (uint8_t i = 0; i < request->args(); i++) {
        message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }
    
    AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", message);
    addSecurityHeaders(response);
    request->send(response);
}

bool WebInterface::handleFileRead(AsyncWebServerRequest *request, String path) {
    if (path.endsWith("/")) {
        path += "index.html";
    }
    
    String contentType = getContentType(path);
    
    if (LittleFS.exists(path)) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, contentType);
        addSecurityHeaders(response);
        request->send(response);
        return true;
    }
    return false;
}

String WebInterface::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}

bool WebInterface::isAuthenticated(AsyncWebServerRequest *request) {
    if (!configManager) return true;
    
    if (request->hasHeader("Authorization")) {
        String authHeader = request->header("Authorization");
        if (authHeader.startsWith("Basic ")) {
            String expectedAuth = String(configManager->getWebUsername()) + ":" + 
                                String(configManager->getWebPassword());
            String expectedBase64 = base64::encode(expectedAuth);
            String receivedBase64 = authHeader.substring(6);
            return expectedBase64 == receivedBase64;
        }
    }
    return false;
}

void WebInterface::requestAuthentication(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(401);
    response->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
    addSecurityHeaders(response);
    request->send(response);
}

void WebInterface::addSecurityHeaders(AsyncWebServerResponse *response) {
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("X-XSS-Protection", "1; mode=block");
    response->addHeader("X-Frame-Options", "DENY");
    response->addHeader("Content-Security-Policy", "default-src 'self'");
    response->addHeader("Strict-Transport-Security", "max-age=31536000; includeSubDomains");
    response->addHeader("X-CSRF-Token", csrfToken);
}

bool WebInterface::validateCSRFToken(AsyncWebServerRequest *request) {
    if (request->hasHeader("X-CSRF-Token")) {
        return request->header("X-CSRF-Token") == csrfToken;
    }
    return false;
}

String WebInterface::generateCSRFToken() {
    const char charset[] = "0123456789"
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "abcdefghijklmnopqrstuvwxyz";
    const int len = 32;
    String token;
    
    for (int i = 0; i < len; i++) {
        int index = random(0, sizeof(charset) - 1);
        token += charset[index];
    }
    
    csrfToken = token;
    return token;
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

void WebInterface::setupMDNS() {
    // Set up mDNS responder
    String hostname = configManager ? configManager->getDeviceName() : "esp32-thermostat";
    
    if (MDNS.begin(hostname.c_str())) {
        Serial.println("mDNS responder started");
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("Error setting up mDNS responder");
    }
}

// ... existing code for other methods ...