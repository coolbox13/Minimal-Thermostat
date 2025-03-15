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

void WebServerManager::setupDefaultRoutes() {
    if (!_server) return;

    // Setup root route to serve index.html
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Check if index.html exists in SPIFFS
        if (SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
        } else {
            // Serve inline HTML if file doesn't exist
            request->send(200, "text/html", THERMOSTAT_HTML);
        }
    });

    // Explicit route for index.html
    _server->on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
        } else {
            request->send(200, "text/html", THERMOSTAT_HTML);
        }
    });

    // Serve static files from SPIFFS with proper MIME types
    _server->serveStatic("/", SPIFFS, "/")
        .setCacheControl("max-age=600");

    // REMOVE THE DUPLICATE OTA ROUTES - they're handled in ota_manager.cpp
    // DO NOT include "/update" and "/doUpdate" routes here

    // Test route
    _server->on("/test", HTTP_GET, handleTest);

    // Server health check
    _server->on("/ping", HTTP_GET, handlePing);

    // API endpoints
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
            request->send(200, "application/json", "{\"success\":true}");
        },
        NULL, // Upload handler is NULL
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            static DynamicJsonDocument jsonDoc(1024);
            static String jsonBuffer;
            
            if (index == 0) {
                jsonBuffer = "";
            }
            
            for (size_t i = 0; i < len; i++) {
                jsonBuffer += (char)data[i];
            }
            
            if (index + len == total) {
                // All data received, process it
                DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
                if (error) {
                    request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
                    return;
                }
                
                // Update configuration
                ConfigManager* configManager = ConfigManager::getInstance();
                bool success = configManager->setFromJson(jsonDoc);
                
                if (!success) {
                    request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to update configuration\"}");
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
        <link rel="stylesheet" href="style.css">
        <style>
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

    // Redirect all KNX library URLs to 404
    _server->on(ROOT_PREFIX, HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/not-found");
    });

    // Explicitly start the server
    _server->begin();
    Serial.println("Web server started");
}

void WebServerManager::handleTest(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Test endpoint working");
}

void WebServerManager::handlePing(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "pong");
}

// Add the embedded HTML with CSS and JS as a constant
const char* THERMOSTAT_HTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 KNX Thermostat</title>
    <style>
        /* Embedded CSS */
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f5f5f5;
            color: #333;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        header {
            background-color: #1e88e5;
            color: white;
            padding: 15px 20px;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            margin: 0;
            font-size: 24px;
        }
        .card {
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            margin-bottom: 20px;
            padding: 20px;
        }
        .card-title {
            margin-top: 0;
            color: #1e88e5;
            font-size: 18px;
            border-bottom: 1px solid #eee;
            padding-bottom: 10px;
        }
        .reading {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 10px;
            padding: 10px;
            background-color: #f9f9f9;
            border-radius: 4px;
        }
        .reading-label {
            font-weight: bold;
        }
        .reading-value {
            font-size: 18px;
        }
        .control-row {
            display: flex;
            align-items: center;
            margin: 15px 0;
        }
        .control-label {
            width: 120px;
            font-weight: bold;
        }
        .slider-container {
            flex-grow: 1;
            margin: 0 15px;
        }
        input[type="range"] {
            width: 100%;
        }
        button {
            background-color: #1e88e5;
            color: white;
            border: none;
            padding: 10px 15px;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
        }
        button:hover {
            background-color: #1976d2;
        }
        .value-display {
            min-width: 60px;
            text-align: center;
        }
        .status {
            font-size: 14px;
            height: 20px;
            margin-top: 10px;
            color: #777;
        }
        .flex-container {
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
        }
        .flex-column {
            flex: 1;
            min-width: 300px;
        }
        @media (max-width: 650px) {
            .flex-container {
                flex-direction: column;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>ESP32 KNX Thermostat</h1>
    </header>
    
    <div class="container">
        <div class="flex-container">
            <div class="flex-column">
                <div class="card">
                    <h2 class="card-title">Current Readings</h2>
                    <div class="reading">
                        <span class="reading-label">Temperature</span>
                        <span class="reading-value" id="temperature">--</span>
                    </div>
                    <div class="reading">
                        <span class="reading-label">Humidity</span>
                        <span class="reading-value" id="humidity">--</span>
                    </div>
                    <div class="reading">
                        <span class="reading-label">Pressure</span>
                        <span class="reading-value" id="pressure">--</span>
                    </div>
                    <div class="reading">
                        <span class="reading-label">Valve Position</span>
                        <span class="reading-value" id="valve">--</span>
                    </div>
                </div>
            </div>
            
            <div class="flex-column">
                <div class="card">
                    <h2 class="card-title">Controls</h2>
                    <div class="control-row">
                        <span class="control-label">Temperature Setpoint</span>
                        <div class="slider-container">
                            <input type="range" id="setpoint-slider" min="15" max="30" step="0.5" value="22">
                        </div>
                        <span id="setpoint-value" class="value-display">22.0°C</span>
                    </div>
                    <div class="control-row">
                        <button id="set-temperature">Set Temperature</button>
                        <div id="status" class="status"></div>
                    </div>
                </div>
                
                <div class="card">
                    <h2 class="card-title">System</h2>
                    <div class="control-row">
                        <button id="refresh-data">Refresh Data</button>
                    </div>
                    <div class="control-row">
                        <a href="/update"><button>Firmware Update</button></a>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        // Embedded JavaScript
        document.addEventListener('DOMContentLoaded', function() {
        // Get DOM elements
        const temperatureElement = document.getElementById('temperature');
        const humidityElement = document.getElementById('humidity');
        const pressureElement = document.getElementById('pressure');
        const valveElement = document.getElementById('valve');
        const setpointSlider = document.getElementById('setpoint-slider');
        const setpointValue = document.getElementById('setpoint-value');
        const setTemperatureButton = document.getElementById('set-temperature');
        const refreshButton = document.getElementById('refresh-data');
        const statusElement = document.getElementById('status');
        
        // Check if elements exist before modifying them
        function updateElement(element, value) {
            if (element) {
                element.textContent = value;
            }
        }
        
        // Update setpoint display when slider changes
        if (setpointSlider) {
            setpointSlider.addEventListener('input', function() {
                if (setpointValue) {
                    setpointValue.textContent = `${setpointSlider.value}°C`;
                }
            });
        }
        
        // Set temperature when button is clicked
        if (setTemperatureButton) {
            setTemperatureButton.addEventListener('click', function() {
                const setpoint = setpointSlider ? setpointSlider.value : 22.0;
                if (statusElement) {
                    statusElement.textContent = `Setting temperature to ${setpoint}°C...`;
                }
                
                fetch('/api/setpoint', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: `value=${setpoint}`
                })
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        if (statusElement) {
                            statusElement.textContent = `Temperature set to ${data.setpoint}°C`;
                        }
                        setTimeout(() => {
                            fetchSensorData();
                        }, 500);
                    } else {
                        if (statusElement) {
                            statusElement.textContent = `Error: ${data.message}`;
                        }
                    }
                })
                .catch(error => {
                    console.error('Error setting temperature:', error);
                    if (statusElement) {
                        statusElement.textContent = `Error: ${error.message}`;
                    }
                });
            });
        }
        
        // Refresh data when button is clicked
        if (refreshButton) {
            refreshButton.addEventListener('click', fetchSensorData);
        }
        
        // Function to fetch sensor data
        function fetchSensorData() {
            if (statusElement) {
                statusElement.textContent = 'Fetching data...';
            }
            
            fetch('/api/sensor-data')
            .then(response => response.json())
            .then(data => {
                if (temperatureElement) {
                    temperatureElement.textContent = `${data.temperature.toFixed(1)}°C`;
                }
                if (humidityElement) {
                    humidityElement.textContent = `${data.humidity.toFixed(1)}%`;
                }
                if (pressureElement) {
                    pressureElement.textContent = `${data.pressure.toFixed(1)} hPa`;
                }
                if (valveElement) {
                    valveElement.textContent = `${data.valve.toFixed(0)}%`;
                }
                
                // Update slider if setpoint has changed
                if (data.setpoint && setpointSlider) {
                    setpointSlider.value = data.setpoint;
                    if (setpointValue) {
                        setpointValue.textContent = `${data.setpoint.toFixed(1)}°C`;
                    }
                }
                
                if (statusElement) {
                    statusElement.textContent = `Data updated at ${new Date().toLocaleTimeString()}`;
                }
            })
            .catch(error => {
                console.error('Error fetching sensor data:', error);
                if (statusElement) {
                    statusElement.textContent = `Error: ${error.message}`;
                }
            });
        }
        
        // Fetch data on page load
        fetchSensorData();
        
        // Fetch data every 30 seconds
        setInterval(fetchSensorData, 30000);
    });
            
            // Refresh data when button is clicked
            refreshButton.addEventListener('click', fetchSensorData);
            
            // Function to fetch sensor data
            function fetchSensorData() {
                statusElement.textContent = 'Fetching data...';
                
                fetch('/api/sensor-data')
                .then(response => response.json())
                .then(data => {
                    temperatureElement.textContent = `${data.temperature.toFixed(1)}°C`;
                    humidityElement.textContent = `${data.humidity.toFixed(1)}%`;
                    pressureElement.textContent = `${data.pressure.toFixed(1)} hPa`;
                    valveElement.textContent = `${data.valve.toFixed(0)}%`;
                    
                    // Update slider if setpoint has changed
                    if (data.setpoint) {
                        setpointSlider.value = data.setpoint;
                        setpointValue.textContent = `${data.setpoint.toFixed(1)}°C`;
                    }
                    
                    statusElement.textContent = `Data updated at ${new Date().toLocaleTimeString()}`;
                })
                .catch(error => {
                    statusElement.textContent = `Error: ${error.message}`;
                });
            }
            
            // Fetch data on page load
            fetchSensorData();
            
            // Fetch data every 30 seconds
            setInterval(fetchSensorData, 30000);
        });
    </script>
</body>
</html>)";