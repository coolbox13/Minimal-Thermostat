#include "wifi_connection.h"
#include "config_manager.h"

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

bool WiFiConnectionManager::begin(unsigned int configPortalTimeout) {
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
    // The next step would be to start the config portal, but we'll let
    // the caller decide whether to do that
    return false;
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
            
            // Update signal strength in history
            _signalHistory[_signalHistoryIndex] = {
                millis(),
                WiFi.RSSI()
            };
            _signalHistoryIndex = (_signalHistoryIndex + 1) % SIGNAL_HISTORY_SIZE;
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
                    unsigned long delayMs = 1000 * (1 << min(_reconnectAttempts, 8));  // Max 256 seconds
                    LOG_D(TAG, "Waiting %lu ms before reconnect attempt", delayMs);
                    delay(delayMs);  // This blocks, but that's intentional for this simple implementation
                }
                
                _reconnectAttempts++;
                
                // Attempt reconnection
                connect();
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
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
        delay(100);
    }
    
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
        LOG_I(TAG, "Signal strength: %d dBm", WiFi.RSSI());
    }
}