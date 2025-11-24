// src/web_server.cpp - Fixed version
#include "web_server.h"

// IMPORTANT: Must include serial_redirect.h early to redirect Serial
#include "serial_redirect.h"

#include "LittleFS.h"
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

    // TEST: Send test messages directly to SerialMonitor after WebSocket is ready
    delay(100);  // Give WebSocket time to fully initialize
    SerialMonitor::getInstance().println("TEST 1: Direct call to SerialMonitor::println()");
    Serial.println("TEST 2: Through Serial (redirected to CapturedSerial)");
    SerialMonitor::getInstance().println("TEST 3: Another direct call");
    Serial.println("TEST 4: Another Serial call");

    // Enable CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // Initialize LittleFS with format_if_failed=true and partition name "spiffs"
    // Note: Partition table uses "spiffs" name (required by PlatformIO buildfs tool)
    // Mount at "/littlefs" (can't mount to root "/" - it's reserved)
    if(!LittleFS.begin(true, "/littlefs", 5, "spiffs")) {
        Serial.println("ERROR: Failed to mount LittleFS");
        Serial.println("Common causes:");
        Serial.println("1. First time use - LittleFS needs to be formatted");
        Serial.println("2. Partition scheme doesn't include filesystem");
        Serial.println("3. Flash memory corruption");

        // Create a dummy index.html in memory since LittleFS failed
        setupDefaultRoutes();
        return;
    }

    // Verify LittleFS contents and list files for debugging
    Serial.println("LittleFS mounted successfully. Files in LittleFS:");
    
    // Helper function to list directory contents
    auto listDir = [](const char* dirname) {
        Serial.printf("\nListing directory: %s\n", dirname);
        File root = LittleFS.open(dirname);
        if (!root || !root.isDirectory()) {
            Serial.printf("  Failed to open directory: %s\n", dirname);
            return;
        }
        
        File file = root.openNextFile();
        int count = 0;
        while (file) {
            Serial.printf("  %s %s (%d bytes)\n", 
                file.isDirectory() ? "DIR " : "FILE", 
                file.name(), 
                file.size());
            count++;
            file = root.openNextFile();
        }
        Serial.printf("  Total: %d entries\n", count);
        root.close();
    };
    
    // List root directory
    listDir("/");
    
    // List assets directory if it exists
    File assetsDir = LittleFS.open("/assets");
    if (assetsDir && assetsDir.isDirectory()) {
        listDir("/assets");
        assetsDir.close();
        
        // List js directory if it exists (JS files moved from /assets/js/ to /js/ to avoid path length limit)
        File jsDir = LittleFS.open("/js");
        if (jsDir && jsDir.isDirectory()) {
            listDir("/js");
            jsDir.close();
        }
    }
    
    // Check for index.html
    if (LittleFS.exists("/index.html.gz") || LittleFS.exists("/index.html")) {
        Serial.println("✓ index.html found in LittleFS");
    } else {
        Serial.println("WARNING: index.html not found in LittleFS");
        Serial.println("Please upload filesystem using 'platformio run --target uploadfs'");
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

    // Setup root route to serve index.html (with gzip support)
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Check for gzip version first (space-optimized)
        // LittleFS.exists() and beginResponse() use filesystem-relative paths, not VFS paths
        // When mounted at "/littlefs", filesystem files at "/index.html.gz" become "/littlefs/index.html.gz" in VFS
        // But LittleFS API needs filesystem-relative path: "/index.html.gz"
        if (LittleFS.exists("/index.html.gz")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        } else if (LittleFS.exists("/index.html")) {
            request->send(LittleFS, "/index.html", "text/html");
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

    // SPA routing: All HTML routes should serve index.html (client-side routing)
    // This includes /config, /status, /logs, etc.
    auto spaRouteHandler = [](AsyncWebServerRequest *request) {
        // LittleFS.exists() and beginResponse() use filesystem-relative paths
        if (LittleFS.exists("/index.html.gz")) {
            File file = LittleFS.open("/index.html.gz", "r");
            if (file) {
                size_t fileSize = file.size();
                file.close();
                AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
                response->addHeader("Content-Encoding", "gzip");
                response->addHeader("Content-Length", String(fileSize));
                response->addHeader("Cache-Control", "no-cache"); // Don't cache HTML
                request->send(response);
            } else {
                request->send(500, "text/plain", "Failed to open index.html.gz");
            }
        } else if (LittleFS.exists("/index.html")) {
            File file = LittleFS.open("/index.html", "r");
            if (file) {
                size_t fileSize = file.size();
                file.close();
                AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", "text/html");
                response->addHeader("Content-Length", String(fileSize));
                response->addHeader("Cache-Control", "no-cache"); // Don't cache HTML
                request->send(response);
            } else {
                request->send(500, "text/plain", "Failed to open index.html");
            }
        } else {
            request->send(404, "text/html",
                "<html><body><h1>Frontend not found</h1>"
                "<p>Please upload the data files using 'platformio run --target uploadfs'.</p></body></html>");
        }
    };

    // CRITICAL: Handle /assets/ and /js/ files FIRST, before SPA routes
    // This ensures asset requests don't fall through to SPA handler
    // This is necessary because serveStatic doesn't properly detect MIME types for ES6 modules
    auto assetHandler = [](AsyncWebServerRequest *request) {
        String path = request->url();
        
        // Debug: log the request
        Serial.printf("[ASSET HANDLER] Request URL: %s\n", path.c_str());
        
        // Sanitize path: remove query strings and fragments
        int queryIndex = path.indexOf('?');
        if (queryIndex >= 0) {
            path = path.substring(0, queryIndex);
        }
        int fragmentIndex = path.indexOf('#');
        if (fragmentIndex >= 0) {
            path = path.substring(0, fragmentIndex);
        }

        // Ensure path starts with / (filesystem-relative paths must start with /)
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        
        Serial.printf("[ASSET HANDLER] Processed path: %s\n", path.c_str());

        // Determine correct MIME type based on file extension
        String contentType = "application/octet-stream";
        if (path.endsWith(".js") || path.endsWith(".mjs")) {
            contentType = "application/javascript";
        } else if (path.endsWith(".css")) {
            contentType = "text/css";
        } else if (path.endsWith(".json")) {
            contentType = "application/json";
        } else if (path.endsWith(".png")) {
            contentType = "image/png";
        } else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) {
            contentType = "image/jpeg";
        } else if (path.endsWith(".svg")) {
            contentType = "image/svg+xml";
        } else if (path.endsWith(".ico")) {
            contentType = "image/x-icon";
        } else if (path.endsWith(".woff") || path.endsWith(".woff2")) {
            contentType = "font/woff";
        } else if (path.endsWith(".ttf")) {
            contentType = "font/ttf";
        }

        // IMPORTANT: Use filesystem-relative paths ONLY (no /littlefs prefix!)
        // LittleFS.exists() and beginResponse() use filesystem-relative paths
        // The /littlefs mount point is handled internally by the VFS layer
        String fsPath = path;  // Already filesystem-relative (e.g., "/assets/js/file.js")
        
        // Try gzipped version first (consistent with root route, space-optimized)
        String gzPath = fsPath + ".gz";  // e.g., "/assets/js/file.js.gz"
        
        Serial.printf("[ASSET] Checking: %s\n", gzPath.c_str());
        if (LittleFS.exists(gzPath)) {
            Serial.printf("[ASSET] Found gzip: %s\n", gzPath.c_str());
            File file = LittleFS.open(gzPath, "r");
            if (file) {
                size_t fileSize = file.size();
                file.close();
                AsyncWebServerResponse *response = request->beginResponse(LittleFS, gzPath, contentType);
                response->addHeader("Content-Encoding", "gzip");
                response->addHeader("Content-Length", String(fileSize));
                response->addHeader("Cache-Control", "public, max-age=31536000"); // Cache for 1 year
                request->send(response);
                return;
            } else {
                Serial.printf("[ASSET] Failed to open gzip file: %s\n", gzPath.c_str());
            }
        }

        // Try uncompressed file as fallback
        Serial.printf("[ASSET] Checking uncompressed: %s\n", fsPath.c_str());
        if (LittleFS.exists(fsPath)) {
            Serial.printf("[ASSET] Found uncompressed: %s\n", fsPath.c_str());
            File file = LittleFS.open(fsPath, "r");
            if (file) {
                size_t fileSize = file.size();
                file.close();
                Serial.printf("[ASSET] Serving with Content-Type: %s, Size: %d bytes\n", contentType.c_str(), fileSize);
                AsyncWebServerResponse *response = request->beginResponse(LittleFS, fsPath, contentType);
                response->addHeader("Content-Length", String(fileSize));
                response->addHeader("Cache-Control", "public, max-age=31536000"); // Cache for 1 year
                request->send(response);
                return;
            } else {
                Serial.printf("[ASSET] Failed to open uncompressed file: %s\n", fsPath.c_str());
            }
        }

        // File not found - log for debugging
        Serial.printf("[ASSET] NOT FOUND: %s (tried %s and %s)\n", path.c_str(), gzPath.c_str(), fsPath.c_str());
        request->send(404, "text/plain", "Asset not found: " + path);
    };

    // Handle /assets/* (CSS, images, etc.) and /js/* (JavaScript bundles)
    // JS files moved from /assets/js/ to /js/ to avoid LittleFS path length limit (~31 chars)
    // Register these BEFORE SPA routes to ensure they match first
    _server->on("/assets/*", HTTP_GET, assetHandler);
    _server->on("/js/*", HTTP_GET, assetHandler);

    // Register SPA routes AFTER asset handlers (all should serve index.html for client-side routing)
    _server->on("/config", HTTP_GET, spaRouteHandler);
    _server->on("/status", HTTP_GET, spaRouteHandler);
    _server->on("/logs", HTTP_GET, spaRouteHandler);
    _server->on("/serial", HTTP_GET, spaRouteHandler);
    _server->on("/dashboard", HTTP_GET, spaRouteHandler);

    // Handle gzip files for other static files (manifest.json, sw.js, icons, etc.)
    // serveStatic doesn't handle .gz files, so we need explicit handlers
    auto staticFileHandler = [](AsyncWebServerRequest *request) {
        String path = request->url();
        
        // Sanitize path: remove query strings
        int queryIndex = path.indexOf('?');
        if (queryIndex >= 0) {
            path = path.substring(0, queryIndex);
        }
        
        // Determine MIME type
        String contentType = "application/octet-stream";
        if (path.endsWith(".json")) {
            contentType = "application/json";
        } else if (path.endsWith(".js")) {
            contentType = "application/javascript";
        } else if (path.endsWith(".png")) {
            contentType = "image/png";
        } else if (path.endsWith(".ico")) {
            contentType = "image/x-icon";
        } else if (path.endsWith(".svg")) {
            contentType = "image/svg+xml";
        }
        
        // IMPORTANT: Use filesystem-relative paths ONLY (no /littlefs prefix!)
        // LittleFS.exists() and beginResponse() use filesystem-relative paths
        String fsPath = path;  // Already filesystem-relative (e.g., "/manifest.json")
        
        // Try gzipped version first
        String gzPath = fsPath + ".gz";  // e.g., "/manifest.json.gz"
        if (LittleFS.exists(gzPath)) {
            File file = LittleFS.open(gzPath, "r");
            if (file) {
                size_t fileSize = file.size();
                file.close();
                AsyncWebServerResponse *response = request->beginResponse(LittleFS, gzPath, contentType);
                response->addHeader("Content-Encoding", "gzip");
                response->addHeader("Content-Length", String(fileSize));
                response->addHeader("Cache-Control", "public, max-age=31536000"); // Cache for 1 year
                request->send(response);
                return;
            }
        }
        
        // Try uncompressed
        if (LittleFS.exists(fsPath)) {
            File file = LittleFS.open(fsPath, "r");
            if (file) {
                size_t fileSize = file.size();
                file.close();
                AsyncWebServerResponse *response = request->beginResponse(LittleFS, fsPath, contentType);
                response->addHeader("Content-Length", String(fileSize));
                response->addHeader("Cache-Control", "public, max-age=31536000"); // Cache for 1 year
                request->send(response);
                return;
            }
        }
        
        request->send(404, "text/plain", "File not found: " + path);
    };
    
    // Register handlers for common static files that might be gzipped
    _server->on("/manifest.json", HTTP_GET, staticFileHandler);
    _server->on("/sw.js", HTTP_GET, staticFileHandler);
    _server->on("/favicon-32x32.png", HTTP_GET, staticFileHandler);
    _server->on("/icon-*.png", HTTP_GET, staticFileHandler);
    _server->on("/apple-touch-icon.png", HTTP_GET, staticFileHandler);

    // Note: serveStatic removed - it was causing double prefix issues
    // All file serving is handled by explicit routes above for better control
    // Explicit routes handle gzip support and correct MIME types

    // DEBUG: List all files in LittleFS
    _server->on("/api/debug/files", HTTP_GET, [](AsyncWebServerRequest *request) {
        String output = "Files in LittleFS:\n\n";
        // LittleFS.open() uses paths relative to filesystem root, not VFS mount point
        File root = LittleFS.open("/");
        if (!root) {
            output += "ERROR: Failed to open root directory\n";
            request->send(500, "text/plain", output);
            return;
        }

        File file = root.openNextFile();
        int count = 0;
        while (file) {
            output += file.name();
            output += " (";
            output += String(file.size());
            output += " bytes)\n";
            count++;
            file = root.openNextFile();
        }

        output += "\nTotal files: " + String(count) + "\n";
        output += "LittleFS Total: " + String(LittleFS.totalBytes()) + " bytes\n";
        output += "LittleFS Used: " + String(LittleFS.usedBytes()) + " bytes\n";
        request->send(200, "text/plain", output);
    });

    // API endpoints
    // Sensor data endpoint - support both /api/sensor and /api/sensor-data for compatibility
    auto sensorDataHandler = [](AsyncWebServerRequest *request) {
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
    };
    
    _server->on("/api/sensor", HTTP_GET, sensorDataHandler);  // Frontend uses this
    _server->on("/api/sensor-data", HTTP_GET, sensorDataHandler);  // Legacy endpoint

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
        doc["system"]["firmware_version"] = FIRMWARE_VERSION;

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

        // Timing intervals and configuration
        doc["timing"]["sensor_update_interval"] = configManager->getSensorUpdateInterval();
        doc["timing"]["history_update_interval"] = configManager->getHistoryUpdateInterval();
        doc["timing"]["pid_update_interval"] = configManager->getPidUpdateInterval();
        doc["timing"]["connectivity_check_interval"] = configManager->getConnectivityCheckInterval();
        doc["timing"]["pid_config_write_interval"] = configManager->getPidConfigWriteInterval();
        doc["timing"]["wifi_connect_timeout"] = configManager->getWifiConnectTimeout();
        doc["timing"]["max_reconnect_attempts"] = configManager->getMaxReconnectAttempts();
        doc["timing"]["system_watchdog_timeout"] = configManager->getSystemWatchdogTimeout();
        doc["timing"]["wifi_watchdog_timeout"] = configManager->getWifiWatchdogTimeout();

        // PID configuration
        doc["pid"]["adaptation_enabled"] = configManager->getAdaptationEnabled();
        doc["pid"]["adaptation_interval"] = configManager->getPidAdaptationInterval();

        // Presets
        doc["presets"]["current"] = configManager->getCurrentPreset();
        doc["presets"]["eco"] = configManager->getPresetTemperature("eco");
        doc["presets"]["comfort"] = configManager->getPresetTemperature("comfort");
        doc["presets"]["away"] = configManager->getPresetTemperature("away");
        doc["presets"]["sleep"] = configManager->getPresetTemperature("sleep");
        doc["presets"]["boost"] = configManager->getPresetTemperature("boost");

        // Manual override
        doc["manual_override"]["enabled"] = configManager->getManualOverrideEnabled();
        doc["manual_override"]["position"] = configManager->getManualOverridePosition();
        doc["manual_override"]["timeout"] = configManager->getManualOverrideTimeout();

        // Webhook configuration
        doc["webhook"]["enabled"] = configManager->getWebhookEnabled();
        doc["webhook"]["url"] = configManager->getWebhookUrl();
        doc["webhook"]["temp_low_threshold"] = configManager->getWebhookTempLowThreshold();
        doc["webhook"]["temp_high_threshold"] = configManager->getWebhookTempHighThreshold();

        // Thermostat mode
        doc["thermostat"]["enabled"] = configManager->getThermostatEnabled();

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

    // Logs are now handled by SPA router above - no inline HTML fallback needed

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