#include "wifi_connection.h"
#include "config_manager.h"
#include <ArduinoJson.h>

// Static members initialization
WiFiConnectionManager* WiFiConnectionManager::_instance = nullptr;
const char* WiFiConnectionManager::TAG = "WIFI";

// Singleton access method
WiFiConnectionManager& WiFiConnectionManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new WiFiConnectionManager();
    }
    return *_instance;
}

// Constructor
WiFiConnectionManager::WiFiConnectionManager() 
    : _state(WiFiConnectionState::DISCONNECTED),
      _lastConnectedTime(0),
      _lastStateChangeTime(0),
      _reconnectAttempts(0),
      _configPortalStarted(false),
      _signalHistoryIndex(0) {
    
    // Get config manager instance
    _configManager = ConfigManager::getInstance();
    
    // Initialize signal history
    for (uint8_t i = 0; i < SIGNAL_HISTORY_SIZE; i++) {
        _signalHistory[i] = {0, 0};
    }
    
    // Setup WiFiManager callbacks
    setupWiFiManagerCallbacks();
}

bool WiFiConnectionManager::begin(unsigned int configPortalTimeout, bool startPortalOnFail) {
    LOG_I(TAG, "Initializing WiFi connection manager");
    
    // Configure WiFiManager settings
    _wifiManager.setConfigPortalTimeout(configPortalTimeout);
    _wifiManager.setConnectRetries(5);
    _wifiManager.setCleanConnect(true);  // Disconnect before connecting
    _wifiManager.setBreakAfterConfig(true);  // Exit config portal when connected
    
    // Log current strategy
    LOG_I(TAG, "WiFi manager initialized with %d second portal timeout", configPortalTimeout);
    
    // Get stored credentials if available
    String storedSSID = _configManager->getWifiSSID();
    String storedPass = _configManager->getWifiPassword();
    
    // If we have stored credentials, try to connect
    if (storedSSID.length() > 0 && storedPass.length() > 0) {
        LOG_I(TAG, "Using stored WiFi credentials for SSID: %s", storedSSID.c_str());
        setState(WiFiConnectionState::CONNECTING);
        
        WiFi.begin(storedSSID.c_str(), storedPass.c_str());
        
        // Wait for connection with timeout
        unsigned long startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
            delay(100);
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            LOG_I(TAG, "Connected to WiFi using stored credentials");
            LOG_I(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
            setState(WiFiConnectionState::CONNECTED);
            _lastConnectedTime = millis();
            return true;
        } else {
            LOG_W(TAG, "Failed to connect with stored credentials");
            setState(WiFiConnectionState::DISCONNECTED);
        }
    } else {
        LOG_W(TAG, "No stored WiFi credentials found");
    }
    
    // If we get here, we couldn't connect with stored credentials
    if (startPortalOnFail) {
        LOG_I(TAG, "Starting config portal after failed connection attempt");
        return startConfigPortal("ESP32-Thermostat-AP", configPortalTimeout);
    } else {
        LOG_I(TAG, "Not starting config portal (startPortalOnFail=false)");
        return false;
    }
}

void WiFiConnectionManager::loop() {
    // If we're in CONFIG_PORTAL_ACTIVE state, let WiFiManager handle everything
    if (_state == WiFiConnectionState::CONFIG_PORTAL_ACTIVE) {
        _wifiManager.process();
        return;
    }
    
    // Check current connection status
    if (WiFi.status() == WL_CONNECTED) {
        // We're connected - update state if needed
        if (_state != WiFiConnectionState::CONNECTED) {
            LOG_I(TAG, "WiFi connected (detected in loop)");
            setState(WiFiConnectionState::CONNECTED);
            _lastConnectedTime = millis();
            resetReconnectAttempts();
            
            // Log full connection details
            logWiFiStatus("WiFi connection established");
        }
        
        // Periodically update signal strength history 
        static unsigned long lastSignalUpdate = 0;
        unsigned long now = millis();
        if (now - lastSignalUpdate > 60000) { // Every minute
            lastSignalUpdate = now;
            
            // Update signal strength in history
            _signalHistory[_signalHistoryIndex] = {
                now,
                WiFi.RSSI()
            };
            _signalHistoryIndex = (_signalHistoryIndex + 1) % SIGNAL_HISTORY_SIZE;
            
            // Log signal strength periodically
            LOG_D(TAG, "Signal strength: %d dBm (%d%%)", 
                  WiFi.RSSI(), getSignalQuality());
                  
            // Check for poor signal quality
            if (getSignalQuality() < 30) {
                LOG_W(TAG, "Poor WiFi signal quality detected (%d%%)", getSignalQuality());
            }
        }
    } else {
        // We're not connected
        if (_state == WiFiConnectionState::CONNECTED) {
            LOG_W(TAG, "WiFi connection lost");
            setState(WiFiConnectionState::CONNECTION_LOST);
        } else if (_state == WiFiConnectionState::CONNECTION_LOST) {
            // We lost the connection - decide whether to attempt reconnect
            // Only attempt reconnect if it's been at least 5 seconds since last state change
            if (millis() - _lastStateChangeTime > 5000) {
                LOG_I(TAG, "Attempting to reconnect to WiFi (attempt %d)", _reconnectAttempts + 1);
                
                // Use exponential backoff for reconnection attempts
                // Wait longer between attempts as the number of attempts increases
                if (_reconnectAttempts > 0) {
                    // Calculate delay based on attempt number (exponential backoff)
                    // 1st: 1s, 2nd: 2s, 3rd: 4s, 4th: 8s, 5th: 16s, 6th: 32s, 7th: 64s, 8th: 128s, 9th+: 256s
                    unsigned long delayMs = 1000 * (1 << min(_reconnectAttempts, 8));  // Max 256 seconds
                    LOG_D(TAG, "Waiting %lu ms before reconnect attempt", delayMs);
                    delay(delayMs);  // This blocks, but that's intentional for this simple implementation
                }
                
                _reconnectAttempts++;
                
                // Attempt reconnection
                if (connect()) {
                    LOG_I(TAG, "Reconnection successful!");
                } else {
                    LOG_W(TAG, "Reconnection attempt %d failed", _reconnectAttempts);
                    
                    // If we've reached the maximum number of attempts, reset WiFi and try again
                    if (_reconnectAttempts >= 5) {
                        LOG_W(TAG, "Maximum reconnection attempts reached, resetting WiFi...");
                        WiFi.disconnect(true);  // Disconnect from the network and delete saved credentials
                        delay(1000);
                        
                        // Try to connect with stored credentials
                        String storedSSID = _configManager->getWifiSSID();
                        String storedPass = _configManager->getWifiPassword();
                        
                        if (storedSSID.length() > 0 && storedPass.length() > 0) {
                            LOG_I(TAG, "Trying stored credentials after WiFi reset");
                            WiFi.begin(storedSSID.c_str(), storedPass.c_str());
                            
                            // Wait a bit to see if it works
                            delay(5000);
                            
                            if (WiFi.status() == WL_CONNECTED) {
                                LOG_I(TAG, "Connection established after WiFi reset!");
                                setState(WiFiConnectionState::CONNECTED);
                                _lastConnectedTime = millis();
                                resetReconnectAttempts();
                            } else {
                                LOG_E(TAG, "Failed to connect even after WiFi reset");
                            }
                        }
                    }
                }
            }
        }
    }
}

bool WiFiConnectionManager::connect(unsigned long timeout) {
    LOG_I(TAG, "Connecting to WiFi...");
    
    // Get stored credentials
    String storedSSID = _configManager->getWifiSSID();
    String storedPass = _configManager->getWifiPassword();
    
    if (storedSSID.length() == 0) {
        LOG_E(TAG, "Cannot connect - no SSID configured");
        return false;
    }
    
    // Set state to connecting
    setState(WiFiConnectionState::CONNECTING);
    
    // In case we're already connected, disconnect first
    WiFi.disconnect();
    delay(100);
    
    // Start connection
    WiFi.begin(storedSSID.c_str(), storedPass.c_str());
    
    // Wait for connection with timeout
    unsigned long startTime = millis();
    unsigned long progress = 0;
    LOG_I(TAG, "Waiting for WiFi connection...");
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
        delay(100);
        progress += 100;
        
        // Print progress dot every second
        if (progress % 1000 == 0) {
            Serial.print(".");
        }
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        LOG_I(TAG, "Connected to WiFi");
        LOG_I(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
        setState(WiFiConnectionState::CONNECTED);
        _lastConnectedTime = millis();
        return true;
    } else {
        LOG_W(TAG, "Failed to connect to WiFi");
        setState(WiFiConnectionState::DISCONNECTED);
        return false;
    }
}

bool WiFiConnectionManager::startConfigPortal(const char* apName, unsigned int timeout) {
    LOG_I(TAG, "Starting WiFi configuration portal");
    
    // Update state
    setState(WiFiConnectionState::CONFIG_PORTAL_ACTIVE);
    _configPortalStarted = true;
    
    // Start the config portal
    bool connected = _wifiManager.startConfigPortal(apName);
    
    // Portal has closed - update state based on connection status
    _configPortalStarted = false;
    if (connected) {
        LOG_I(TAG, "Connected to WiFi through config portal");
        setState(WiFiConnectionState::CONNECTED);
        _lastConnectedTime = millis();
        resetReconnectAttempts();
        
        // Get and save the new credentials
        _configManager->setWifiSSID(WiFi.SSID());
        _configManager->setWifiPassword(WiFi.psk());
        LOG_I(TAG, "New WiFi credentials saved");
        
        return true;
    } else {
        LOG_W(TAG, "Config portal closed without successful connection");
        setState(WiFiConnectionState::DISCONNECTED);
        return false;
    }
}

WiFiConnectionState WiFiConnectionManager::getState() const {
    return _state;
}

int WiFiConnectionManager::getSignalStrength() const {
    if (_state != WiFiConnectionState::CONNECTED) {
        return 0;
    }
    return WiFi.RSSI();
}

int WiFiConnectionManager::getSignalQuality() const {
    if (_state != WiFiConnectionState::CONNECTED) {
        return 0;
    }
    
    // Convert RSSI to quality percentage
    // RSSI range is typically -100 dBm (worst) to -30 dBm (best)
    int rssi = WiFi.RSSI();
    
    if (rssi <= -100) {
        return 0;
    } else if (rssi >= -30) {
        return 100;
    } else {
        // Linear conversion from RSSI to quality percentage
        return 2 * (rssi + 100);
    }
}

unsigned long WiFiConnectionManager::getTimeSinceLastConnection() const {
    if (_lastConnectedTime == 0) {
        return 0;  // Never connected
    }
    return millis() - _lastConnectedTime;
}

void WiFiConnectionManager::registerStateCallback(WiFiStateCallback callback) {
    _stateCallbacks.push_back(callback);
}

bool WiFiConnectionManager::isConfigPortalActive() const {
    return _state == WiFiConnectionState::CONFIG_PORTAL_ACTIVE;
}

WiFiManager& WiFiConnectionManager::getWiFiManager() {
    return _wifiManager;
}

int WiFiConnectionManager::getReconnectAttempts() const {
    return _reconnectAttempts;
}

void WiFiConnectionManager::resetReconnectAttempts() {
    _reconnectAttempts = 0;
}

String WiFiConnectionManager::getConnectionDetailsJson(bool includeHistory) {
    StaticJsonDocument<512> doc;
    
    // Connection state
    const char* stateNames[] = {
        "DISCONNECTED",
        "CONNECTING",
        "CONNECTED",
        "CONFIG_PORTAL_ACTIVE",
        "CONNECTION_LOST"
    };
    doc["state"] = stateNames[static_cast<int>(_state)];
    
    // Only include WiFi details if connected
    if (_state == WiFiConnectionState::CONNECTED) {
        doc["ssid"] = WiFi.SSID();
        doc["ip"] = WiFi.localIP().toString();
        doc["gateway"] = WiFi.gatewayIP().toString();
        doc["subnet"] = WiFi.subnetMask().toString();
        doc["mac"] = WiFi.macAddress();
        doc["rssi"] = WiFi.RSSI();
        doc["signal_quality"] = getSignalQuality();
        doc["channel"] = WiFi.channel();
    }
    
    // Include state timing
    doc["last_connected"] = _lastConnectedTime;
    doc["time_since_last_connected"] = getTimeSinceLastConnection();
    doc["last_state_change"] = _lastStateChangeTime;
    doc["reconnect_attempts"] = _reconnectAttempts;
    
    // Include signal history if requested
    if (includeHistory && _state == WiFiConnectionState::CONNECTED) {
        JsonArray history = doc.createNestedArray("signal_history");
        for (uint8_t i = 0; i < SIGNAL_HISTORY_SIZE; i++) {
            if (_signalHistory[i].timestamp > 0) {
                JsonObject entry = history.createNestedObject();
                entry["timestamp"] = _signalHistory[i].timestamp;
                entry["rssi"] = _signalHistory[i].rssi;
            }
        }
    }
    
    // Internet connectivity
    doc["internet_connectivity"] = testInternetConnectivity();
    
    String result;
    serializeJson(doc, result);
    return result;
}

void WiFiConnectionManager::setState(WiFiConnectionState newState) {
    if (newState == _state) {
        return;  // No change
    }
    
    WiFiConnectionState oldState = _state;
    _state = newState;
    _lastStateChangeTime = millis();
    
    // Log the state change
    const char* stateNames[] = {
        "DISCONNECTED",
        "CONNECTING",
        "CONNECTED",
        "CONFIG_PORTAL_ACTIVE",
        "CONNECTION_LOST"
    };
    
    LOG_I(TAG, "WiFi state changed: %s -> %s", 
          stateNames[static_cast<int>(oldState)], 
          stateNames[static_cast<int>(newState)]);
    
    // Notify callbacks
    for (auto callback : _stateCallbacks) {
        callback(newState, oldState);
    }
}

void WiFiConnectionManager::setupWiFiManagerCallbacks() {
    // Called when WiFiManager enters AP mode
    _wifiManager.setAPCallback([this](WiFiManager* wifiManager) {
        setState(WiFiConnectionState::CONFIG_PORTAL_ACTIVE);
        _configPortalStarted = true;
        LOG_I(TAG, "WiFi configuration portal started");
        LOG_I(TAG, "Connect to AP: %s to configure WiFi", WiFi.softAPSSID().c_str());
        LOG_I(TAG, "AP IP address: %s", WiFi.softAPIP().toString().c_str());
    });
    
    // Called when WiFiManager has saved a new configuration
    _wifiManager.setSaveConfigCallback([this]() {
        LOG_I(TAG, "New WiFi configuration saved");
        
        // This will be called before the connection is actually established,
        // so we don't update the state yet. The state will be updated in
        // loop() once the connection is established.
        
        // We do want to save the credentials to our config manager
        _configManager->setWifiSSID(WiFi.SSID());
        _configManager->setWifiPassword(WiFi.psk());
        
        // Reset reconnect attempts since we have new credentials
        resetReconnectAttempts();
        
        // Mark config portal as no longer active
        _configPortalStarted = false;
    });
}

void WiFiConnectionManager::logWiFiStatus(const char* message) {
    LOG_I(TAG, "%s: %s", message, WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    if (WiFi.status() == WL_CONNECTED) {
        LOG_I(TAG, "SSID: %s", WiFi.SSID().c_str());
        LOG_I(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
        LOG_I(TAG, "Signal strength: %d dBm (%d%%)", WiFi.RSSI(), getSignalQuality());
        LOG_I(TAG, "Channel: %d", WiFi.channel());
        LOG_I(TAG, "MAC address: %s", WiFi.macAddress().c_str());
        
        // Test internet connectivity
        if (testInternetConnectivity()) {
            LOG_I(TAG, "Internet connectivity: OK");
        } else {
            LOG_W(TAG, "Internet connectivity: FAILED");
        }
    } else {
        // Log WiFi status code for debugging
        const char* statusText;
        switch (WiFi.status()) {
            case WL_IDLE_STATUS: statusText = "Idle"; break;
            case WL_NO_SSID_AVAIL: statusText = "No SSID available"; break;
            case WL_SCAN_COMPLETED: statusText = "Scan completed"; break;
            case WL_CONNECT_FAILED: statusText = "Connection failed"; break;
            case WL_CONNECTION_LOST: statusText = "Connection lost"; break;
            case WL_DISCONNECTED: statusText = "Disconnected"; break;
            default: statusText = "Unknown status"; break;
        }
        LOG_I(TAG, "WiFi status: %s (%d)", statusText, WiFi.status());
    }
}

bool WiFiConnectionManager::testInternetConnectivity() {
    // Implement a simple DNS lookup to test internet connectivity
    IPAddress result;
    
    LOG_D(TAG, "Testing internet connectivity with DNS lookup...");
    
    // Try 3 common domains with a short timeout
    if (WiFi.hostByName("google.com", result, 2000)) {
        LOG_D(TAG, "DNS lookup successful: google.com -> %s", result.toString().c_str());
        return true;
    }
    
    if (WiFi.hostByName("cloudflare.com", result, 2000)) {
        LOG_D(TAG, "DNS lookup successful: cloudflare.com -> %s", result.toString().c_str());
        return true;
    }
    
    if (WiFi.hostByName("amazon.com", result, 2000)) {
        LOG_D(TAG, "DNS lookup successful: amazon.com -> %s", result.toString().c_str());
        return true;
    }
    
    LOG_W(TAG, "DNS lookup failed for all test domains");
    return false;
}