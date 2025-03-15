#include "web_server.h"
#include "SPIFFS.h"
#include <Update.h>
#include "bme280_sensor.h"
#include "valve_control.h"
#include "adaptive_pid_controller.h"
#include "persistence_manager.h"

WebServerManager* WebServerManager::_instance = nullptr;

WebServerManager* WebServerManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new WebServerManager();
    }
    return _instance;
}

WebServerManager::WebServerManager() : _server(nullptr) {}

void WebServerManager::begin(AsyncWebServer* server) {
    _server = server;
    
    // Print persistence values during bootup
    PersistenceManager::getInstance()->printStoredValues();
    
    // Enable CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Initialize SPIFFS with format_if_failed=true
    if(!SPIFFS.begin(true)) {
        Serial.println("ERROR: Failed to mount SPIFFS");
        Serial.println("Common causes:");
        Serial.println("1. First time use - SPIFFS needs to be formatted");
        Serial.println("2. Partition scheme doesn't include SPIFFS");
        Serial.println("3. Flash memory corruption");
        return;
    }

    // Verify SPIFFS contents
    if(!SPIFFS.exists("/index.html")) {
        Serial.println("WARNING: index.html not found in SPIFFS");
        Serial.println("Please ensure all required files are uploaded to SPIFFS");
    } else {
        Serial.println("SPIFFS mounted successfully, web files found");
    }
    
    setupDefaultRoutes();
}

void WebServerManager::addEndpoint(const char* uri, WebRequestMethodComposite method,
                                  ArRequestHandlerFunction handler) {
    if (_server) {
        _server->on(uri, method, handler);
    }
}

void WebServerManager::addEndpoint(const char* uri, WebRequestMethodComposite method,
                                  ArRequestHandlerFunction onRequest,
                                  ArUploadHandlerFunction onUpload) {
    if (_server) {
        _server->on(uri, method, onRequest, onUpload);
    }
}

void WebServerManager::setupDefaultRoutes() {
    if (!_server) return;

    // Root route - Serve index.html from SPIFFS if available
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if(SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
        } else {
            handleRoot(request);
        }
    });

    // Add firmware update page
    _server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", 
            "<html><body>"
            "<h1>ESP32 KNX Thermostat Firmware Update</h1>"
            "<form method='POST' action='/doUpdate' enctype='multipart/form-data'>"
            "<input type='file' name='update'><br><br>"
            "<input type='submit' value='Update Firmware'>"
            "</form>"
            "</body></html>");
    });
    
    // Handler for the actual update
    _server->on("/doUpdate", HTTP_POST, 
        [](AsyncWebServerRequest *request) {
            bool shouldReboot = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", 
                shouldReboot ? "Update successful! Rebooting..." : "Update failed!");
            response->addHeader("Connection", "close");
            request->send(response);
            if (shouldReboot) {
                delay(1000);
                ESP.restart();
            }
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                Serial.printf("OTA: Update start: %s\n", filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Serial.println("OTA: Failed to begin update");
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA could not begin");
                }
            }
            
            if (Update.write(data, len) != len) {
                Serial.println("OTA: Failed to write update");
                Update.printError(Serial);
                return request->send(400, "text/plain", "OTA could not proceed");
            }
            
            if (final) {
                if (!Update.end(true)) {
                    Serial.println("OTA: Failed to complete update");
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA failed to complete");
                }
                Serial.println("OTA: Update complete");
            }
        }
    );

    // Test route
    _server->on("/test", HTTP_GET, handleTest);

    // Server health check
    _server->on("/ping", HTTP_GET, handlePing);

    // Server functionality verification
    _server->on("/servertest", HTTP_GET, handleServerTest);

    // Persistence values endpoint
    _server->on("/api/persistence", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<512> doc;
        PersistenceManager::getInstance()->getStoredValues(doc);
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Sensor data endpoint
    _server->on("/api/sensor-data", HTTP_GET, [](AsyncWebServerRequest *request) {
        extern BME280Sensor bme280;
        extern ValveControl valveControl;
        extern AdaptivePID_Input g_pid_input;
        
        StaticJsonDocument<200> doc;
        doc["temperature"] = bme280.readTemperature();
        doc["humidity"] = bme280.readHumidity();
        doc["pressure"] = bme280.readPressure();
        doc["valve"] = valveControl.getPosition();
        doc["setpoint"] = g_pid_input.setpoint_temp;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Serve static files from SPIFFS if available
    if(SPIFFS.exists("/")) {
        _server->serveStatic("/", SPIFFS, "/");
    }
    
    // Explicitly start the server
    _server->begin();
}

void WebServerManager::handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<html><body><h1>ESP32 KNX Thermostat</h1>"
                                   "<p>Welcome to the thermostat dashboard.</p></body></html>");
}

void WebServerManager::handleTest(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Test endpoint working");
}

void WebServerManager::handlePing(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "pong");
}

void WebServerManager::handleServerTest(AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\": \"ok\", \"message\": \"Server is functioning correctly\"}");
}