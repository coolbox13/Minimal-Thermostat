// src/web_server.cpp - Fixed version
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

// Add the callback type definition and variable here
typedef std::function<void()> KnxAddressChangedCallback;
KnxAddressChangedCallback _knxAddressChangedCallback = nullptr;

// Add the method to register the callback
void WebServerManager::setKnxAddressChangedCallback(KnxAddressChangedCallback callback) {
    _knxAddressChangedCallback = callback;
    Serial.println("KNX address changed callback registered");
}

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
        
        // Create a dummy index.html in memory since SPIFFS failed
        setupDefaultRoutes();
        return;
    }

    // Verify SPIFFS contents and list files for debugging
    Serial.println("SPIFFS mounted successfully. Files in SPIFFS:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    bool foundIndex = false;
    
    // In the WebServerManager::begin method, update the file listing code:
    while(file) {
        String fileName = file.name();
        size_t fileSize = file.size();
        Serial.print("File: ");
        Serial.print(fileName);
        Serial.print(" - Size: ");
        Serial.println(fileSize);
        
        // Check for index.html (both with and without leading slash)
        if(fileName == "/index.html" || fileName == "index.html") {
            foundIndex = true;
        }
        file = root.openNextFile();
    }

    if(!foundIndex) {
        Serial.println("WARNING: index.html not found in SPIFFS");
        Serial.println("Creating a default index.html in memory");
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

// Fixed version of web server routes to handle static files properly
void WebServerManager::setupDefaultRoutes() {
    if (!_server) return;

    // Setup root route to serve index.html
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Check if index.html exists in SPIFFS
        if (SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
        } else {
            // Serve a basic HTML with error message if file doesn't exist
            request->send(200, "text/html", 
                "<html><head><title>ESP32 KNX Thermostat</title></head>"
                "<body style='font-family:Arial;text-align:center;'>"
                "<h1>ESP32 KNX Thermostat</h1>"
                "<div style='color:red;margin:20px;'>"
                "Web interface files not found. Please upload the data files using 'platformio run --target uploadfs'."
                "</div>"
                "<p><a href='/api/config'>View current configuration (JSON)</a></p>"
                "</body></html>");
        }
    });

    // Explicitly route to config.html
    _server->on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (SPIFFS.exists("/config.html")) {
            request->send(SPIFFS, "/config.html", "text/html");
        } else {
            request->send(404, "text/plain", "Configuration page not found. Please upload the data files.");
        }
    });

    // Serve static files from SPIFFS with proper MIME types
    _server->serveStatic("/", SPIFFS, "/")
        .setCacheControl("max-age=600");

    // API endpoints
    _server->on("/api/sensor-data", HTTP_GET, [](AsyncWebServerRequest *request) {
        extern BME280Sensor bme280;
        extern float temperature;
        extern float humidity;
        extern float pressure;
        extern AdaptivePID_Input g_pid_input;
        
        StaticJsonDocument<200> doc;
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["pressure"] = pressure;
        doc["valve"] = g_pid_input.valve_feedback;
        doc["setpoint"] = g_pid_input.setpoint_temp;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Set temperature setpoint
    _server->on("/api/setpoint", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value", true)) {
            float setpoint = request->getParam("value", true)->value().toFloat();
            setTemperatureSetpoint(setpoint);
            request->send(200, "application/json", "{\"success\":true,\"setpoint\":" + String(setpoint) + "}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing value parameter\"}");
        }
    });

    // Get current configuration
    _server->on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();
        StaticJsonDocument<1024> doc;
        
        configManager->getJson(doc);
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Update configuration
    _server->on("/api/config", HTTP_POST, 
        [](AsyncWebServerRequest *request) {
            // Response will be sent after processing is complete via the onBody handler
        },
        NULL, // Upload handler is NULL
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            static DynamicJsonDocument jsonDoc(1024);
            static String jsonBuffer;
            
            if (index == 0) {
                jsonBuffer = "";
            }
            
            // Add data to buffer with size check
            if (jsonBuffer.length() + len < 1024) {
                for (size_t i = 0; i < len; i++) {
                    jsonBuffer += (char)data[i];
                }
            } else {
                // Buffer would overflow
                request->send(400, "application/json", 
                    "{\"success\":false,\"message\":\"Configuration data too large\"}");
                return;
            }
            
            if (index + len == total) {
                // All data received, process it
                
                // Store the old KNX test address setting
                bool oldUseTestSetting = false;
                ConfigManager* configManager = ConfigManager::getInstance();
                if (configManager) {
                    oldUseTestSetting = configManager->getUseTestAddresses();
                }
                
                DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
                if (error) {
                    request->send(400, "application/json", 
                        "{\"success\":false,\"message\":\"Invalid JSON: " + String(error.c_str()) + "\"}");
                    return;
                }
                
                // Update configuration
                String errorMessage;
                bool success = configManager->setFromJson(jsonDoc, errorMessage);
                
                // Inside the config update handler's onBody function, where JSON is processed
                // After deserializing the JSON and before sending the response
                
                if (success) {
                    // Check if KNX test address setting changed
                    if (jsonDoc.containsKey("knx") && jsonDoc["knx"].containsKey("use_test")) {
                        bool newUseTestSetting = jsonDoc["knx"]["use_test"].as<bool>();
                        
                        // Now using 'this' correctly since it's captured
                        if (oldUseTestSetting != newUseTestSetting && this->_knxAddressChangedCallback) {
                            Serial.println("KNX address setting changed, triggering callback");
                            this->_knxAddressChangedCallback();
                        }
                    }
                    
                    // Add code to handle PID parameter changes with improved rounding 
                    if (jsonDoc.containsKey("pid")) {
                        // Update PID controller with new values if they exist in the JSON
                        if (jsonDoc["pid"].containsKey("kp")) {
                            // Get the value and apply explicit rounding to ensure proper precision
                            float kp = jsonDoc["pid"]["kp"].as<float>();
                            kp = roundf(kp * 100) / 100.0f; // Round to 2 decimal places
                            setPidKp(kp);
                            Serial.print("Rounded Kp value: ");
                            Serial.println(kp, 2); // Print with 2 decimal places for debugging
                        }
                        
                        if (jsonDoc["pid"].containsKey("ki")) {
                            float ki = jsonDoc["pid"]["ki"].as<float>();
                            ki = roundf(ki * 1000) / 1000.0f; // Round to 3 decimal places
                            setPidKi(ki);
                            Serial.print("Rounded Ki value: ");
                            Serial.println(ki, 3); // Print with 3 decimal places for debugging
                        }
                        
                        if (jsonDoc["pid"].containsKey("kd")) {
                            float kd = jsonDoc["pid"]["kd"].as<float>();
                            kd = roundf(kd * 1000) / 1000.0f; // Round to 3 decimal places
                            setPidKd(kd);
                            Serial.print("Rounded Kd value: ");
                            Serial.println(kd, 3); // Print with 3 decimal places for debugging
                        }
                        
                        if (jsonDoc["pid"].containsKey("setpoint")) {
                            float setpoint = jsonDoc["pid"]["setpoint"].as<float>();
                            setpoint = roundf(setpoint * 10) / 10.0f; // Round to 1 decimal place
                            setTemperatureSetpoint(setpoint);
                            Serial.print("Rounded setpoint value: ");
                            Serial.println(setpoint, 1); // Print with 1 decimal place for debugging
                        }
                        
                        Serial.println("PID parameters updated from web interface with precision-controlled values");
                    }
                    
                    request->send(200, "application/json", "{\"success\":true}");
                } else {
                    request->send(500, "application/json", 
                        "{\"success\":false,\"message\":\"" + errorMessage + "\"}");
                }
            }
        }
    );

    // Reboot device
    _server->on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Rebooting...\"}");
        // Schedule reboot after response is sent
        delay(500);
        ESP.restart();
    });

    // Set up 404 handler last to ensure it catches unmatched routes
    _server->onNotFound([](AsyncWebServerRequest *request) {
        String html = R"(<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>404 - Page Not Found</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 0;
                background-color: #f5f5f5;
                color: #333;
            }
            header {
                background-color: #1e88e5;
                color: white;
                padding: 15px 20px;
                text-align: center;
            }
            h1 {
                margin: 0;
                font-size: 24px;
            }
            .container {
                max-width: 800px;
                margin: 0 auto;
                padding: 20px;
            }
            .card {
                background-color: white;
                border-radius: 8px;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                margin-bottom: 20px;
                padding: 20px;
            }
            .error-container {
                text-align: center;
                padding: 40px 20px;
            }
            .error-code {
                font-size: 80px;
                font-weight: bold;
                color: #e74c3c;
                margin: 0;
            }
            .error-message {
                font-size: 24px;
                margin: 20px 0;
            }
            .error-details {
                margin-bottom: 30px;
                color: #7f8c8d;
            }
            .home-button {
                display: inline-block;
                padding: 10px 20px;
                background-color: #1e88e5;
                color: white;
                text-decoration: none;
                border-radius: 4px;
                font-weight: bold;
            }
        </style>
    </head>
    <body>
        <header>
            <h1>ESP32 KNX Thermostat</h1>
        </header>
        
        <div class="container">
            <div class="card error-container">
                <h2 class="error-code">404</h2>
                <h3 class="error-message">Page Not Found</h3>
                <p class="error-details">
                    The page you requested could not be found on this server.<br>
                    You might have followed a broken link or typed the address incorrectly.
                </p>
                <a href="/" class="home-button">Return to Dashboard</a>
            </div>
        </div>
    </body>
</html>)";
        
        request->send(404, "text/html", html);
    });

    // Explicitly start the server
    _server->begin();
    Serial.println("Web server started");
}