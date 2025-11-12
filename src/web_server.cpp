// src/web_server.cpp - Fixed version
#include "web_server.h"
#include "SPIFFS.h"
#include <Update.h>
#include "bme280_sensor.h"
#include "valve_control.h"
#include "adaptive_pid_controller.h"
#include "persistence_manager.h"
#include "event_log.h"
#include "history_manager.h"
#include "webhook_manager.h"
#include "config_manager.h"
#include "ntp_manager.h"
#include "sensor_health_monitor.h"
#include "valve_health_monitor.h"
#include "serial_monitor.h"

WebServerManager* WebServerManager::_instance = nullptr;

WebServerManager* WebServerManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new WebServerManager();
    }
    return _instance;
}

WebServerManager::WebServerManager() : _server(nullptr) {}

void WebServerManager::setKnxAddressChangedCallback(KnxAddressChangedCallback callback) {
    _knxAddressChangedCallback = callback;
    Serial.println("KNX address changed callback registered");
}

void WebServerManager::begin(AsyncWebServer* server) {
    _server = server;

    // Print persistence values during bootup
    PersistenceManager::getInstance()->printStoredValues();

    // Initialize serial monitor WebSocket
    SerialMonitor::getInstance().begin(_server);
    Serial.println("Serial monitor WebSocket initialized");

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

// Helper method to handle KNX address changes
void WebServerManager::handleKNXAddressChange(const JsonDocument& jsonDoc, bool oldUseTestSetting) {
    if (!jsonDoc.containsKey("knx") || !jsonDoc["knx"].containsKey("use_test")) {
        return;
    }
    bool newUseTestSetting = jsonDoc["knx"]["use_test"].as<bool>();
    if (oldUseTestSetting != newUseTestSetting && _knxAddressChangedCallback) {
        Serial.println("KNX address setting changed, triggering callback");
        _knxAddressChangedCallback();
    }
}
void WebServerManager::handlePIDParameterUpdates(const JsonDocument& jsonDoc) {
    if (!jsonDoc.containsKey("pid")) {
        return;
    }
    if (jsonDoc["pid"].containsKey("kp")) {
        float kp = ConfigManager::roundToPrecision(jsonDoc["pid"]["kp"].as<float>(), 2);
        setPidKp(kp);
        Serial.print("Rounded Kp value: ");
        Serial.println(kp, 2);
    }
    if (jsonDoc["pid"].containsKey("ki")) {
        float ki = ConfigManager::roundToPrecision(jsonDoc["pid"]["ki"].as<float>(), 3);
        setPidKi(ki);
        Serial.print("Rounded Ki value: ");
        Serial.println(ki, 3);
    }
    if (jsonDoc["pid"].containsKey("kd")) {
        float kd = ConfigManager::roundToPrecision(jsonDoc["pid"]["kd"].as<float>(), 3);
        setPidKd(kd);
        Serial.print("Rounded Kd value: ");
        Serial.println(kd, 3);
    }
    if (jsonDoc["pid"].containsKey("setpoint")) {
        float setpoint = ConfigManager::roundToPrecision(jsonDoc["pid"]["setpoint"].as<float>(), 1);
        setTemperatureSetpoint(setpoint);
        Serial.print("Rounded setpoint value: ");
        Serial.println(setpoint, 1);
    }
    Serial.println("PID parameters updated from web interface with precision-controlled values");
}

void WebServerManager::handleNTPUpdate(const JsonDocument& jsonDoc) {
    if (!jsonDoc.containsKey("network")) {
        return;
    }
    
    // Update NTP manager if settings changed and WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
        NTPManager& ntpManager = NTPManager::getInstance();
        ConfigManager* configManager = ConfigManager::getInstance();
        
        bool ntpChanged = false;
        
        if (jsonDoc["network"].containsKey("ntp_server")) {
            String ntpServer = configManager->getNtpServer();
            ntpManager.setNTPServer(ntpServer.c_str());
            ntpChanged = true;
        }
        
        if (jsonDoc["network"].containsKey("ntp_timezone_offset")) {
            int timezoneOffset = configManager->getNtpTimezoneOffset();
            ntpManager.setTimezoneOffset(timezoneOffset);
            ntpChanged = true;
        }
        
        if (jsonDoc["network"].containsKey("ntp_daylight_offset")) {
            int daylightOffset = configManager->getNtpDaylightOffset();
            ntpManager.setDaylightOffset(daylightOffset);
            ntpChanged = true;
        }
        
        // Re-sync time if NTP settings changed
        if (ntpChanged) {
            Serial.println("NTP settings updated, re-syncing time...");
            if (ntpManager.syncTime(10000)) {
                String timeStr = ntpManager.getFormattedTime();
                Serial.print("Time re-synchronized: ");
                Serial.println(timeStr);
            } else {
                Serial.println("NTP time re-synchronization failed");
            }
        }
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

    // Historical data endpoint
    _server->on("/api/history", HTTP_GET, [](AsyncWebServerRequest *request) {
        HistoryManager* historyManager = HistoryManager::getInstance();

        // Check for maxPoints parameter
        int maxPoints = 0;
        if (request->hasParam("maxPoints")) {
            maxPoints = request->getParam("maxPoints")->value().toInt();
        }

        DynamicJsonDocument doc(8192);  // Large buffer for history data
        historyManager->getHistoryJson(doc, maxPoints);

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // System status dashboard endpoint
    _server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        extern BME280Sensor bme280;
        extern float temperature;
        extern float humidity;
        extern float pressure;
        extern AdaptivePID_Input g_pid_input;

        ConfigManager* configManager = ConfigManager::getInstance();

        DynamicJsonDocument doc(2048);

        // System information
        doc["system"]["uptime"] = millis() / 1000; // seconds
        doc["system"]["free_heap"] = ESP.getFreeHeap();
        doc["system"]["total_heap"] = ESP.getHeapSize();

        // Flash memory - pre-calculated values
        uint32_t freeFlash = ESP.getFreeSketchSpace();
        uint32_t usedFlash = ESP.getSketchSize();
        uint32_t totalOtaPartition = freeFlash + usedFlash;
        uint8_t flashPercent = (freeFlash * 100) / totalOtaPartition;

        doc["system"]["free_flash_kb"] = freeFlash / 1024;
        doc["system"]["used_flash_kb"] = usedFlash / 1024;
        doc["system"]["ota_partition_kb"] = totalOtaPartition / 1024;
        doc["system"]["flash_percent_free"] = flashPercent;

        doc["system"]["chip_model"] = ESP.getChipModel();
        doc["system"]["chip_revision"] = ESP.getChipRevision();
        doc["system"]["cpu_freq_mhz"] = ESP.getCpuFreqMHz();

        // WiFi information
        if (WiFi.status() == WL_CONNECTED) {
            doc["wifi"]["connected"] = true;
            doc["wifi"]["ssid"] = WiFi.SSID();
            doc["wifi"]["rssi"] = WiFi.RSSI();
            doc["wifi"]["ip"] = WiFi.localIP().toString();
            doc["wifi"]["mac"] = WiFi.macAddress();

            // Signal quality (convert RSSI to percentage)
            int rssi = WiFi.RSSI();
            int quality;
            if (rssi >= -50) quality = 100;
            else if (rssi <= -100) quality = 0;
            else quality = 2 * (rssi + 100);
            doc["wifi"]["quality"] = quality;
        } else {
            doc["wifi"]["connected"] = false;
            doc["wifi"]["ssid"] = "";
            doc["wifi"]["rssi"] = 0;
            doc["wifi"]["quality"] = 0;
        }

        // Sensor information
        doc["sensor"]["temperature"] = temperature;
        doc["sensor"]["humidity"] = humidity;
        doc["sensor"]["pressure"] = pressure;

        // PID Controller information
        doc["pid"]["setpoint"] = g_pid_input.setpoint_temp;
        doc["pid"]["valve_position"] = g_pid_input.valve_feedback;
        doc["pid"]["kp"] = configManager->getPidKp();
        doc["pid"]["ki"] = configManager->getPidKi();
        doc["pid"]["kd"] = configManager->getPidKd();
        doc["pid"]["deadband"] = configManager->getPidDeadband();

        // Diagnostic information
        doc["diagnostics"]["last_reboot_reason"] = configManager->getLastRebootReason();
        doc["diagnostics"]["reboot_count"] = configManager->getRebootCount();
        doc["diagnostics"]["consecutive_watchdog_reboots"] = configManager->getConsecutiveWatchdogReboots();

        // Configuration
        doc["mqtt"]["server"] = configManager->getMqttServer();
        doc["mqtt"]["port"] = configManager->getMqttPort();
        doc["knx"]["address"] = String(configManager->getKnxArea()) + "." +
                                 String(configManager->getKnxLine()) + "." +
                                 String(configManager->getKnxMember());

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Sensor health status endpoint (Item #9)
    _server->on("/api/sensor-health", HTTP_GET, [](AsyncWebServerRequest *request) {
        SensorHealthMonitor* sensorHealth = SensorHealthMonitor::getInstance();

        DynamicJsonDocument doc(512);

        doc["healthy"] = sensorHealth->isSensorHealthy();
        doc["consecutive_failures"] = sensorHealth->getConsecutiveFailures();
        doc["total_readings"] = sensorHealth->getTotalReadings();
        doc["failed_readings"] = sensorHealth->getFailedReadings();
        doc["failure_rate"] = sensorHealth->getFailureRate();
        doc["last_good_reading_time"] = sensorHealth->getLastGoodReadingTime();
        doc["last_good_value"] = sensorHealth->getLastGoodValue();

        // Calculate time since last good reading
        unsigned long timeSinceGood = millis() - sensorHealth->getLastGoodReadingTime();
        doc["seconds_since_good_reading"] = timeSinceGood / 1000;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Valve health status endpoint (Item #10)
    _server->on("/api/valve-health", HTTP_GET, [](AsyncWebServerRequest *request) {
        ValveHealthMonitor* valveHealth = ValveHealthMonitor::getInstance();

        DynamicJsonDocument doc(512);

        doc["healthy"] = valveHealth->isValveHealthy();
        doc["average_error"] = valveHealth->getAverageError();
        doc["max_error"] = valveHealth->getMaxError();
        doc["stuck_count"] = valveHealth->getStuckCount();
        doc["consecutive_stuck"] = valveHealth->getConsecutiveStuckCount();
        doc["last_commanded"] = valveHealth->getLastCommandedPosition();
        doc["last_actual"] = valveHealth->getLastActualPosition();
        doc["last_error"] = valveHealth->getLastError();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Set temperature setpoint
    _server->on("/api/setpoint", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value", true)) {
            float setpoint = request->getParam("value", true)->value().toFloat();

            // CRITICAL FIX: Validate setpoint range (Audit Fix #2)
            // Must match ConfigManager validation (5-30°C)
            if (isnan(setpoint) || setpoint < 5.0f || setpoint > 30.0f) {
                request->send(400, "application/json",
                    "{\"success\":false,\"message\":\"Setpoint must be between 5°C and 30°C\"}");
                return;
            }

            setTemperatureSetpoint(setpoint);
            request->send(200, "application/json", "{\"success\":true,\"setpoint\":" + String(setpoint) + "}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing value parameter\"}");
        }
    });

    // Manual valve override - Enable/Disable
    _server->on("/api/manual-override", HTTP_POST, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();

        if (!request->hasParam("enabled", true)) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing enabled parameter\"}");
            return;
        }

        bool enabled = request->getParam("enabled", true)->value() == "true" ||
                      request->getParam("enabled", true)->value() == "1";

        // If enabling, require position parameter
        if (enabled) {
            if (!request->hasParam("position", true)) {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing position parameter\"}");
                return;
            }

            int position = request->getParam("position", true)->value().toInt();
            if (position < 0 || position > 100) {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"Position must be 0-100\"}");
                return;
            }

            configManager->setManualOverridePosition((uint8_t)position);
            configManager->setManualOverrideActivationTime(millis());
            configManager->setManualOverrideEnabled(true);

            request->send(200, "application/json",
                "{\"success\":true,\"enabled\":true,\"position\":" + String(position) + "}");
        } else {
            // Disable manual override
            configManager->setManualOverrideEnabled(false);
            request->send(200, "application/json", "{\"success\":true,\"enabled\":false}");
        }
    });

    // Get manual override status
    _server->on("/api/manual-override", HTTP_GET, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();

        StaticJsonDocument<256> doc;
        doc["enabled"] = configManager->getManualOverrideEnabled();
        doc["position"] = configManager->getManualOverridePosition();
        doc["timeout"] = configManager->getManualOverrideTimeout();

        if (configManager->getManualOverrideEnabled()) {
            unsigned long activationTime = configManager->getManualOverrideActivationTime();
            unsigned long elapsed = (millis() - activationTime) / 1000;
            doc["elapsed_seconds"] = elapsed;

            uint32_t timeout = configManager->getManualOverrideTimeout();
            if (timeout > 0) {
                doc["remaining_seconds"] = (timeout > elapsed) ? (timeout - elapsed) : 0;
            }
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Get current configuration
    _server->on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();
        // Increased from 1024 to 2048 to accommodate webhook URL (up to 512 chars)
        // and other configuration fields. Total estimated size: ~1500 bytes max
        DynamicJsonDocument doc(2048);

        configManager->getJson(doc);

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Export configuration as downloadable JSON file
    _server->on("/api/config/export", HTTP_GET, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();
        // Increased from 1024 to 2048 to match /api/config endpoint
        DynamicJsonDocument doc(2048);

        configManager->getJson(doc);

        String response;
        serializeJsonPretty(doc, response);

        // Generate filename with timestamp
        char filename[50];
        snprintf(filename, sizeof(filename), "thermostat-config-%lu.json", millis() / 1000);

        // Use application/json with Content-Disposition header to trigger download
        AsyncWebServerResponse *downloadResponse = request->beginResponse(200, "application/json", response);
        downloadResponse->addHeader("Content-Disposition", String("attachment; filename=\"") + filename + "\"");
        request->send(downloadResponse);
    });

    // Import configuration from uploaded JSON file
    _server->on("/api/config/import", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            // Response will be sent after upload processing
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            static String fileContent;

            if (index == 0) {
                fileContent = "";
            }

            // Append data to buffer
            for (size_t i = 0; i < len; i++) {
                fileContent += (char)data[i];
            }

            // Process when upload is complete
            if (final) {
                ConfigManager* configManager = ConfigManager::getInstance();
                // Increased from 1024 to 2048 to match export endpoint
                DynamicJsonDocument doc(2048);

                DeserializationError error = deserializeJson(doc, fileContent);
                if (error) {
                    request->send(400, "application/json",
                        "{\"success\":false,\"message\":\"Invalid JSON file: " + String(error.c_str()) + "\"}");
                    return;
                }

                String errorMessage;
                bool success = configManager->setFromJson(doc, errorMessage);
                if (success) {
                    request->send(200, "application/json",
                        "{\"success\":true,\"message\":\"Configuration imported successfully\"}");
                } else {
                    request->send(500, "application/json",
                        "{\"success\":false,\"message\":\"" + errorMessage + "\"}");
                }
            }
        }
    );

    // Update configuration
    _server->on("/api/config", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            // Response will be sent after processing is complete via the onBody handler
        },
        NULL, // Upload handler is NULL
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            // Increased from 1024 to 2048 to accommodate webhook URL (up to 512 chars)
            // and other configuration fields. Total estimated size: ~1500 bytes max
            static DynamicJsonDocument jsonDoc(2048);
            static String jsonBuffer;

            if (index == 0) {
                jsonBuffer = "";
            }

            // Add data to buffer with size check
            if (jsonBuffer.length() + len < 2048) {
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
                ConfigManager* configManager = ConfigManager::getInstance();
                bool oldUseTestSetting = configManager ? configManager->getUseTestAddresses() : false;
                DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
                if (error) {
                    request->send(400, "application/json",
                        "{\"success\":false,\"message\":\"Invalid JSON: " + String(error.c_str()) + "\"}");
                    return;
                }
                String errorMessage;
                bool success = configManager->setFromJson(jsonDoc, errorMessage);
                if (success) {
                    this->handleKNXAddressChange(jsonDoc, oldUseTestSetting);
                    this->handlePIDParameterUpdates(jsonDoc);
                    this->handleNTPUpdate(jsonDoc);
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

    // Test webhook endpoint
    _server->on("/api/webhook/test", HTTP_POST, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();
        WebhookManager webhookManager;

        // Configure webhook from current settings
        webhookManager.configure(configManager->getWebhookUrl(), configManager->getWebhookEnabled());

        // Send a test event
        bool success = webhookManager.sendEvent("test_event", "Test from ESP32 Thermostat",
                                                  "This is a test webhook", "Testing webhook integration");

        if (success) {
            request->send(200, "application/json",
                "{\"success\":true,\"message\":\"Test webhook sent successfully\"}");
        } else {
            request->send(500, "application/json",
                "{\"success\":false,\"message\":\"Failed to send webhook. Check URL and network connection.\"}");
        }
    });

    // Get event logs
    _server->on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        String logsJson = EventLog::getInstance().getEntriesJSON();
        request->send(200, "application/json", logsJson);
    });

    // Clear event logs
    _server->on("/api/logs/clear", HTTP_POST, [](AsyncWebServerRequest *request) {
        EventLog::getInstance().clear();
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Logs cleared\"}");
    });

    // Redirect /logs to /logs.html
    _server->on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/logs.html");
    });

    // Serve logs.html (inline fallback if not in SPIFFS)
    _server->on("/logs.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (SPIFFS.exists("/logs.html")) {
            request->send(SPIFFS, "/logs.html", "text/html");
        } else {
            // Inline HTML fallback
            request->send(200, "text/html",
                "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
                "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">"
                "<title>Event Logs - ESP32 KNX Thermostat</title>"
                "<link rel=\"stylesheet\" href=\"style.css\">"
                "<style>.log-table{width:100%;border-collapse:collapse;margin-top:10px}"
                ".log-table th{background-color:#2c3e50;color:white;padding:10px;text-align:left;border-bottom:2px solid #34495e}"
                ".log-table td{padding:8px;border-bottom:1px solid #ecf0f1}.log-table tr:hover{background-color:#f8f9fa}"
                ".log-level{display:inline-block;padding:3px 8px;border-radius:3px;font-size:.85em;font-weight:bold}"
                ".log-level-ERROR{background-color:#e74c3c;color:white}.log-level-WARNING{background-color:#f39c12;color:white}"
                ".log-level-INFO{background-color:#3498db;color:white}.log-level-DEBUG{background-color:#95a5a6;color:white}"
                ".log-level-VERBOSE{background-color:#bdc3c7;color:#2c3e50}.log-timestamp{font-family:monospace;color:#7f8c8d}"
                ".log-tag{font-family:monospace;font-weight:bold;color:#2c3e50}.log-message{color:#2c3e50}"
                ".controls{display:flex;gap:10px;margin-bottom:15px;align-items:center}"
                ".filter-select{padding:8px;border:1px solid #bdc3c7;border-radius:4px;font-size:14px}"
                ".no-logs{text-align:center;padding:30px;color:#7f8c8d;font-style:italic}.log-count{color:#7f8c8d;font-size:.9em}</style>"
                "</head><body><header><h1>Event Logs</h1></header><div class=\"container\"><div class=\"card\">"
                "<h2 class=\"card-title\">System Event Logs</h2><div class=\"controls\">"
                "<button id=\"refresh-logs\">Refresh</button><button id=\"clear-logs\" style=\"background-color:#e74c3c\">Clear All Logs</button>"
                "<select id=\"filter-level\" class=\"filter-select\"><option value=\"all\">All Levels</option>"
                "<option value=\"ERROR\">Errors Only</option><option value=\"WARNING\">Warnings & Errors</option>"
                "<option value=\"INFO\">Info & Above</option></select><span class=\"log-count\" id=\"log-count\">0 entries</span></div>"
                "<div id=\"logs-container\"><table class=\"log-table\"><thead><tr><th>Time</th><th>Level</th><th>Tag</th><th>Message</th></tr></thead>"
                "<tbody id=\"logs-body\"><tr><td colspan=\"4\" class=\"no-logs\">Loading logs...</td></tr></tbody></table></div></div>"
                "<div class=\"card\"><h2 class=\"card-title\">Navigation</h2><div class=\"control-row\"><a href=\"/\"><button>Dashboard</button></a></div>"
                "<div class=\"control-row\"><a href=\"/config.html\"><button>Configuration</button></a></div></div></div>"
                "<script>let allLogs=[];async function fetchLogs(){try{const r=await fetch('/api/logs');if(!r.ok)throw new Error('Failed');allLogs=await r.json();displayLogs()}catch(e){console.error('Error:',e);document.getElementById('logs-body').innerHTML='<tr><td colspan=\"4\" class=\"no-logs\">Error loading logs</td></tr>'}}"
                "function displayLogs(){const f=document.getElementById('filter-level').value;const b=document.getElementById('logs-body');"
                "let logs=allLogs;if(f!=='all'){const p={'ERROR':1,'WARNING':2,'INFO':3,'DEBUG':4,'VERBOSE':5};const m=p[f]||5;"
                "logs=allLogs.filter(l=>(p[l.level]||5)<=m)}document.getElementById('log-count').textContent=`${logs.length} of ${allLogs.length} entries`;"
                "b.innerHTML='';if(logs.length===0){b.innerHTML='<tr><td colspan=\"4\" class=\"no-logs\">No logs to display</td></tr>';return}"
                "logs.sort((a,b)=>b.timestamp-a.timestamp);logs.forEach(l=>{const r=b.insertRow();const t=r.insertCell();t.className='log-timestamp';"
                "t.textContent=formatTimestamp(l.timestamp);const lc=r.insertCell();const s=document.createElement('span');"
                "s.className=`log-level log-level-${l.level}`;s.textContent=l.level;lc.appendChild(s);const tc=r.insertCell();"
                "tc.className='log-tag';tc.textContent=l.tag;const mc=r.insertCell();mc.className='log-message';mc.textContent=l.message})}"
                "function formatTimestamp(ms){const s=Math.floor(ms/1000);const m=Math.floor(s/60);const h=Math.floor(m/60);const d=Math.floor(h/24);"
                "if(d>0)return `${d}d ${h%24}h ${m%60}m`;else if(h>0)return `${h}h ${m%60}m ${s%60}s`;else if(m>0)return `${m}m ${s%60}s`;else return `${s}s`}"
                "async function clearLogs(){if(!confirm('Clear all logs?'))return;try{const r=await fetch('/api/logs/clear',{method:'POST'});"
                "if(!r.ok)throw new Error('Failed');allLogs=[];displayLogs();alert('Logs cleared')}catch(e){console.error('Error:',e);alert('Failed to clear logs')}}"
                "document.getElementById('refresh-logs').addEventListener('click',fetchLogs);document.getElementById('clear-logs').addEventListener('click',clearLogs);"
                "document.getElementById('filter-level').addEventListener('change',displayLogs);fetchLogs();setInterval(fetchLogs,10000);</script></body></html>");
        }
    });

    // Factory reset - clear all settings and reboot
    _server->on("/api/factory-reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        ConfigManager* configManager = ConfigManager::getInstance();

        if (configManager->factoryReset()) {
            request->send(200, "application/json",
                "{\"success\":true,\"message\":\"Factory reset completed. Rebooting...\"}");
            // Schedule reboot after response is sent
            delay(500);
            ESP.restart();
        } else {
            request->send(500, "application/json",
                "{\"success\":false,\"message\":\"Factory reset failed\"}");
        }
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