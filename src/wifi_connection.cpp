#include "wifi_connection.h"
#include "watchdog_manager.h"
#include "config_manager.h"
#include <ArduinoJson.h>
#include "serial_monitor.h"
#include "serial_redirect.h"

// Redirect Serial to CapturedSerial for web monitor
#define Serial CapturedSerial

// Try to include ESP32Ping if available
#if __has_include(<ESP32Ping.h>)
  #include <ESP32Ping.h>
  #define HAS_ESP32PING
#endif

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
      _lastSignalCheck(0),
      _reconnectAttempts(0),
      _configPortalStarted(false),
      _signalHistoryIndex(0),
      _maxReconnectAttempts(10),  // Default to 10 attempts
      _disableWatchdogDuringOperations(false),
      _reconnectionInProgress(false),
      _nextSubscriptionId(1),
      _watchdogManager(nullptr) {
    
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
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_CONNECT_TIMEOUT_MS) {
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

void WiFiConnectionManager::setWatchdogManager(WatchdogManager* watchdogManager) {
    this->_watchdogManager = watchdogManager;
    LOG_I(TAG, "Watchdog manager set");
}

bool WiFiConnectionManager::connect(unsigned long timeout) {
    LOG_I(TAG, "Connecting to WiFi...");
    
    // Mark that reconnection is in progress
    _reconnectionInProgress = true;
    
    // Pause watchdog during connection if available and enabled
    if (_watchdogManager && _disableWatchdogDuringOperations) {
        LOG_D(TAG, "Pausing watchdog during WiFi connection");
        _watchdogManager->pauseWatchdogs(timeout + 5000); // Add 5 seconds buffer
    }
    
    // Get stored credentials
    String storedSSID = _configManager->getWifiSSID();
    String storedPass = _configManager->getWifiPassword();
    
    if (storedSSID.length() == 0) {
        LOG_E(TAG, "Cannot connect - no SSID configured");
        _reconnectionInProgress = false;
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
        _reconnectionInProgress = false;
        
        // Reset WiFi watchdog on successful connection
        if (_watchdogManager) {
            _watchdogManager->resetWiFiWatchdog();
        }
        
        return true;
    } else {
        LOG_W(TAG, "Failed to connect to WiFi");
        setState(WiFiConnectionState::DISCONNECTED);
        _reconnectionInProgress = false;
        return false;
    }
}

bool WiFiConnectionManager::startConfigPortal(const char* apName, unsigned int timeout) {
    LOG_I(TAG, "Starting WiFi configuration portal");
    
    // Disable WiFi watchdog during config portal if available
    if (_watchdogManager && _disableWatchdogDuringOperations) {
        LOG_D(TAG, "Disabling WiFi watchdog during config portal");
        _watchdogManager->enableWiFiWatchdog(false);
    }
    
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

void WiFiConnectionManager::loop() {
    // If we're in CONFIG_PORTAL_ACTIVE state, let WiFiManager handle everything
    if (_state == WiFiConnectionState::CONFIG_PORTAL_ACTIVE) {
        _wifiManager.process();
        return;
    }
    
    // Check current connection status and handle reconnection logic
    handleConnectionStatus();
    
    // Handle periodic tasks like signal strength monitoring
    handlePeriodicTasks();
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

int WiFiConnectionManager::registerEventCallback(WiFiEventCallback callback, WiFiEventFilter filter) {
    EventSubscription subscription;
    subscription.callback = callback;
    subscription.filter = filter;
    subscription.id = _nextSubscriptionId++;
    
    _eventSubscriptions.push_back(subscription);
    LOG_D(TAG, "Registered WiFi event callback with ID: %d", subscription.id);
    
    return subscription.id;
}

bool WiFiConnectionManager::unregisterEventCallback(int subscriptionId) {
    for (auto it = _eventSubscriptions.begin(); it != _eventSubscriptions.end(); ++it) {
        if (it->id == subscriptionId) {
            _eventSubscriptions.erase(it);
            LOG_D(TAG, "Unregistered WiFi event callback ID: %d", subscriptionId);
            return true;
        }
    }
    
    LOG_W(TAG, "Failed to unregister WiFi event callback ID: %d (not found)", subscriptionId);
    return false;
}

void WiFiConnectionManager::triggerEvent(WiFiEventType type, const String& message) {
    // Create event with proper initialization
    WiFiConnectionEvent event;
    event.type = type;
    event.oldState = _state;
    event.newState = _state;
    event.message = message;
    event.timestamp = millis();
    
    // Set WiFi-specific fields conditionally based on connection status
    if (WiFi.status() == WL_CONNECTED) {
        event.ssid = WiFi.SSID();
        event.signalStrength = getSignalStrength();
        event.signalQuality = getSignalQuality();
        
        // Network info
        event.networkInfo.ip = WiFi.localIP();
        event.networkInfo.gateway = WiFi.gatewayIP();
        event.networkInfo.subnet = WiFi.subnetMask();
        event.networkInfo.dns1 = WiFi.dnsIP(0);
        event.networkInfo.dns2 = WiFi.dnsIP(1);
    } else {
        event.ssid = "";
        event.signalStrength = 0;
        event.signalQuality = 0;
        
        // Initialize network info with empty values
        event.networkInfo.ip = IPAddress(0, 0, 0, 0);
        event.networkInfo.gateway = IPAddress(0, 0, 0, 0);
        event.networkInfo.subnet = IPAddress(0, 0, 0, 0);
        event.networkInfo.dns1 = IPAddress(0, 0, 0, 0);
        event.networkInfo.dns2 = IPAddress(0, 0, 0, 0);
    }
    
    // Special case for connecting event
    if (type == WiFiEventType::CONNECTING) {
        event.connecting.reconnectAttempt = _reconnectAttempts;
    }
    
    // Notify all subscribers whose filter accepts this event
    for (const auto& subscription : _eventSubscriptions) {
        if (subscription.filter(event)) {
            subscription.callback(event);
        }
    }
    
    // Log the event
    LOG_D(TAG, "WiFi event: %s - %s", getEventTypeName(type), message.c_str());
}

void WiFiConnectionManager::registerStateCallback(WiFiStateCallback callback) {
    _stateCallbacks.push_back(callback);
    
    // Create an adapter from the legacy callback to the new event system
    auto adapter = [callback](const WiFiConnectionEvent& event) {
        callback(event.newState, event.oldState);
    };
    
    // Only trigger for events that change state
    auto filter = [](const WiFiConnectionEvent& event) {
        return event.newState != event.oldState;
    };
    
    // Register the adapter with the new event system
    registerEventCallback(adapter, filter);
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

void WiFiConnectionManager::setMaxReconnectAttempts(int maxAttempts) {
    _maxReconnectAttempts = maxAttempts;
    LOG_I(TAG, "Max reconnect attempts set to %d", maxAttempts);
}

int WiFiConnectionManager::getMaxReconnectAttempts() const {
    return _maxReconnectAttempts;
}

void WiFiConnectionManager::setDisableWatchdogDuringOperations(bool disable) {
    _disableWatchdogDuringOperations = disable;
    LOG_I(TAG, "Watchdog during operations %s", disable ? "disabled" : "enabled");
}

bool WiFiConnectionManager::isWatchdogDisabledDuringOperations() const {
    return _disableWatchdogDuringOperations;
}

bool WiFiConnectionManager::testInternetConnectivity() {
    if (_state != WiFiConnectionState::CONNECTED) {
        return false;
    }
    
    IPAddress ip;
    if (!ip.fromString(TEST_HOST)) {
        LOG_E(TAG, "Failed to parse test host IP");
        return false;
    }
    
    bool success = false;
    
#ifdef HAS_ESP32PING
    // Use the global Ping object from the ESP32Ping library
    success = Ping.ping(ip, 3);  // Try 3 times
#else
    // Fallback if the library isn't available
    LOG_W(TAG, "ESP32Ping library not available, using simple connectivity test");
    
    // Simple connectivity test using a TCP connection
    WiFiClient client;
    client.setTimeout(2000); // 2 second timeout
    success = client.connect(ip, 53); // Try connecting to DNS port
    client.stop();
#endif
    
    LOG_I(TAG, "Internet connectivity test %s", success ? "passed" : "failed");
    return success;
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
    doc["max_reconnect_attempts"] = _maxReconnectAttempts;
    
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
    doc["watchdog_disabled_during_operations"] = _disableWatchdogDuringOperations;
    doc["reconnection_in_progress"] = _reconnectionInProgress;
    
    String result;
    serializeJson(doc, result);
    return result;
}

void WiFiConnectionManager::setState(WiFiConnectionState newState) {
    if (_state != newState) {
        // Store old state for callback
        WiFiConnectionState oldState = _state;
        
        // Get state names for better logging
        const char* oldStateName = getStateName(oldState);
        const char* newStateName = getStateName(newState);
        
        LOG_I(TAG, "WiFi state changing: %s -> %s", oldStateName, newStateName);
        
        // Additional detailed logging based on specific transitions
        if (newState == WiFiConnectionState::CONNECTED) {
            // Log connection details
            String ssid = WiFi.SSID();
            String ip = WiFi.localIP().toString();
            int rssi = WiFi.RSSI();
            
            LOG_I(TAG, "Connected to: %s (IP: %s, RSSI: %d dBm, Quality: %d%%)", 
                  ssid.c_str(), ip.c_str(), rssi, getSignalQuality());
            
            // Log time since last connection attempt
            if (_state == WiFiConnectionState::CONNECTING) {
                unsigned long connectionTime = millis() - _lastStateChangeTime;
                LOG_I(TAG, "Connection established in %lu ms after %d attempts", 
                      connectionTime, _reconnectAttempts);
            }
            
            // Reset reconnect attempts on successful connection
            _reconnectAttempts = 0;
            _lastConnectedTime = millis();
        }
        else if (newState == WiFiConnectionState::CONNECTING) {
            LOG_I(TAG, "Attempting to connect to WiFi (Attempt %d of %d)", 
                  _reconnectAttempts + 1, 
                  _maxReconnectAttempts == 0 ? 0 : _maxReconnectAttempts);
        }
        else if (newState == WiFiConnectionState::CONNECTION_LOST) {
            unsigned long connectedDuration = millis() - _lastConnectedTime;
            LOG_W(TAG, "WiFi connection lost after %lu ms of connectivity", connectedDuration);
        }
        
        _state = newState;
        _lastStateChangeTime = millis();
        
        // Notify callbacks about state change
        for (auto callback : _stateCallbacks) {
            callback(newState, oldState);
        }
        
        // Trigger event for new event system
        WiFiEventType eventType;
        if (_state == WiFiConnectionState::CONNECTED) {
            eventType = WiFiEventType::CONNECTED;
        } else if (_state == WiFiConnectionState::DISCONNECTED) {
            eventType = WiFiEventType::DISCONNECTED;
        } else if (_state == WiFiConnectionState::CONNECTION_LOST) {
            eventType = WiFiEventType::CONNECTION_LOST;
        } else if (_state == WiFiConnectionState::CONNECTING) {
            eventType = WiFiEventType::CONNECTING;
        } else {
            // Use STATE_CHANGED as a fallback
            eventType = WiFiEventType::STATE_CHANGED;
        }
        
        String message = String("State changed to: ") + newStateName;
        triggerEvent(eventType, message);
    }
}

const char* WiFiConnectionManager::getStateName(WiFiConnectionState state) {
    switch (state) {
        case WiFiConnectionState::DISCONNECTED: return "DISCONNECTED";
        case WiFiConnectionState::CONNECTING: return "CONNECTING";
        case WiFiConnectionState::CONNECTED: return "CONNECTED";
        case WiFiConnectionState::CONFIG_PORTAL_ACTIVE: return "CONFIG_PORTAL_ACTIVE";
        case WiFiConnectionState::CONNECTION_LOST: return "CONNECTION_LOST";
        default: return "UNKNOWN";
    }
}

const char* WiFiConnectionManager::getEventTypeName(WiFiEventType type) {
    switch (type) {
        case WiFiEventType::CONNECTED: return "CONNECTED";
        case WiFiEventType::DISCONNECTED: return "DISCONNECTED";
        case WiFiEventType::CONNECTING: return "CONNECTING";
        case WiFiEventType::CONNECTION_LOST: return "CONNECTION_LOST";
        case WiFiEventType::CONNECTION_FAILED: return "CONNECTION_FAILED";
        case WiFiEventType::CONFIG_PORTAL_STARTED: return "CONFIG_PORTAL_STARTED";
        case WiFiEventType::CONFIG_PORTAL_STOPPED: return "CONFIG_PORTAL_STOPPED";
        case WiFiEventType::INTERNET_CONNECTED: return "INTERNET_CONNECTED";
        case WiFiEventType::INTERNET_LOST: return "INTERNET_LOST";
        case WiFiEventType::SIGNAL_CHANGED: return "SIGNAL_CHANGED";
        case WiFiEventType::STATE_CHANGED: return "STATE_CHANGED";
        case WiFiEventType::CREDENTIALS_SAVED: return "CREDENTIALS_SAVED";
        default: return "UNKNOWN";
    }
}

void WiFiConnectionManager::setupWiFiManagerCallbacks() {
    // Called when WiFiManager enters AP mode
    _wifiManager.setAPCallback([this](WiFiManager* wifiManager) {
        setState(WiFiConnectionState::CONFIG_PORTAL_ACTIVE);
        _configPortalStarted = true;
        
        // Disable WiFi watchdog during config portal
        if (_watchdogManager && _disableWatchdogDuringOperations) {
            LOG_D(TAG, "Disabling WiFi watchdog during config portal (callback)");
            _watchdogManager->enableWiFiWatchdog(false);
        }
        
        triggerEvent(WiFiEventType::CONFIG_PORTAL_STARTED, "Configuration portal started");
    });
    
    // Called when WiFiManager has saved a new configuration
    _wifiManager.setSaveConfigCallback([this]() {
        LOG_I(TAG, "New WiFi configuration saved");
        
        triggerEvent(WiFiEventType::CREDENTIALS_SAVED, "WiFi credentials saved");
        
        // Save the credentials to our config manager
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

int WiFiConnectionManager::getConnectionQuality() {
    // Calculate average RSSI from signal history
    int sum = 0;
    int count = 0;
    
    for (size_t i = 0; i < SIGNAL_HISTORY_SIZE; i++) {
        if (_signalHistory[i].timestamp > 0) {
            sum += _signalHistory[i].rssi;
            count++;
        }
    }
    
    if (count == 0) return 0;
    
    int avgRSSI = sum / count;
    
    // Calculate stability (standard deviation)
    float variance = 0;
    for (size_t i = 0; i < SIGNAL_HISTORY_SIZE; i++) {
        if (_signalHistory[i].timestamp > 0) {
            variance += pow(_signalHistory[i].rssi - avgRSSI, 2);
        }
    }
    variance /= count;
    float stability = sqrt(variance);
    
    // Quality score (0-100)
    int qualityScore;
    if (avgRSSI >= QUALITY_THRESHOLD_GOOD) {
        qualityScore = 100;
    } else if (avgRSSI >= QUALITY_THRESHOLD_FAIR) {
        qualityScore = 75;
    } else {
        qualityScore = 50;
    }
    
    // Reduce score based on stability
    if (stability > 5) {
        qualityScore -= static_cast<int>(stability);
    }
    
    // Ensure score stays within bounds
    qualityScore = std::max(0, std::min(100, qualityScore));
    
    LOG_D(TAG, "Connection Quality: %d%% (RSSI: %d, Stability: %.1f)", 
          qualityScore, avgRSSI, stability);
    
    return qualityScore;
}

void WiFiConnectionManager::checkAndLogSignalStrength() {
    if (millis() - _lastSignalCheck >= SIGNAL_CHECK_INTERVAL) {
        _lastSignalCheck = millis();
        
        if (_state == WiFiConnectionState::CONNECTED) {
            int rssi = WiFi.RSSI();
            int quality = getSignalQuality();
            
            LOG_I(TAG, "WiFi Signal Strength - RSSI: %d dBm, Quality: %d%%", rssi, quality);
            
            // Store in signal history
            _signalHistory[_signalHistoryIndex].timestamp = millis();
            _signalHistory[_signalHistoryIndex].rssi = rssi;
            _signalHistoryIndex = (_signalHistoryIndex + 1) % SIGNAL_HISTORY_SIZE;
            
            // Compare with previous reading and trigger event if significant change
            int prevIndex = (_signalHistoryIndex == 0) ? SIGNAL_HISTORY_SIZE - 1 : _signalHistoryIndex - 1;
            if (_signalHistory[prevIndex].timestamp > 0) {
                int prevRSSI = _signalHistory[prevIndex].rssi;
                if (abs(rssi - prevRSSI) > 5) { // 5 dBm threshold for significant change
                    String message = "Signal strength changed from " + String(prevRSSI) + 
                                     " to " + String(rssi) + " dBm";
                    triggerEvent(WiFiEventType::SIGNAL_CHANGED, message);
                }
            }
        }
    }
}

void WiFiConnectionManager::handleConnectionStatus() {
    // Check if WiFi is still connected
    if (_state == WiFiConnectionState::CONNECTED && WiFi.status() != WL_CONNECTED) {
        int wifiStatus = WiFi.status();
        LOG_W(TAG, "WiFi connection lost, WiFi.status()=%d", wifiStatus);
        LOG_D(TAG, "Last RSSI before disconnect: %d dBm, uptime: %lu ms",
              WiFi.RSSI(), millis() - _lastConnectedTime);
        setState(WiFiConnectionState::CONNECTION_LOST);
        triggerEvent(WiFiEventType::CONNECTION_LOST, "WiFi connection lost unexpectedly");
    }
    
    // Attempt reconnection if we're in a disconnected state
    if ((_state == WiFiConnectionState::DISCONNECTED || _state == WiFiConnectionState::CONNECTION_LOST) 
        && !_reconnectionInProgress) {
        
        // Check if we've reached max reconnection attempts
        if (_maxReconnectAttempts > 0 && _reconnectAttempts >= _maxReconnectAttempts) {
            LOG_W(TAG, "Maximum reconnection attempts (%d) reached", _maxReconnectAttempts);
            // Don't attempt further reconnection until reset
            return;
        }
        
        // Increment reconnection counter
        _reconnectAttempts++;
        
        // Attempt to reconnect
        LOG_I(TAG, "Attempting reconnection (attempt %d of %d)", 
              _reconnectAttempts, _maxReconnectAttempts == 0 ? 0 : _maxReconnectAttempts);
        
        connect(WIFI_RECONNECT_TIMEOUT_MS);
    }
}

void WiFiConnectionManager::handlePeriodicTasks() {
    // Check and log signal strength periodically
    checkAndLogSignalStrength();
    
    // Test internet connectivity periodically (every 5 minutes)
    static unsigned long lastConnectivityCheck = 0;
    if (_state == WiFiConnectionState::CONNECTED &&
        millis() - lastConnectivityCheck > INTERNET_CHECK_INTERVAL_MS) {
        
        lastConnectivityCheck = millis();
        bool hasInternet = testInternetConnectivity();
        
        if (hasInternet) {
            triggerEvent(WiFiEventType::INTERNET_CONNECTED, "Internet connectivity confirmed");
        } else {
            triggerEvent(WiFiEventType::INTERNET_LOST, "Internet connectivity lost");
            LOG_W(TAG, "Device is connected to WiFi but internet connectivity test failed");
        }
    }
}