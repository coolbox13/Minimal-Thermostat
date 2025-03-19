#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <functional>
#include <vector>
#include "config.h"
#include "logger.h"

/**
 * @file wifi_connection.h
 * @brief Manages WiFi connection for the ESP32 KNX Thermostat
 * 
 * This class encapsulates all WiFi-related functionality including
 * initialization, connection management, reconnection attempts,
 * and integration with WiFiManager for configuration portal.
 */

// Forward declarations
class ConfigManager;

/**
 * @brief WiFi connection states
 */
enum class WiFiConnectionState {
    DISCONNECTED,      // Not connected to WiFi
    CONNECTING,        // Attempting to connect
    CONNECTED,         // Successfully connected to WiFi
    CONFIG_PORTAL_ACTIVE, // WiFi configuration portal is active
    CONNECTION_LOST    // Connection was established but then lost
};

/**
 * @brief Data structure for storing signal strength history
 */
struct SignalStrengthRecord {
    unsigned long timestamp;
    int rssi;
};

/**
 * @brief Callback function type for WiFi events
 */
typedef std::function<void(WiFiConnectionState newState, WiFiConnectionState oldState)> WiFiStateCallback;

class WiFiConnectionManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton instance
     */
    static WiFiConnectionManager& getInstance();

    /**
     * @brief Initialize WiFi connection
     * @param configPortalTimeout Timeout for config portal in seconds (0 = no timeout)
     * @return True if successfully initialized
     */
    bool begin(unsigned int configPortalTimeout = 180);

    /**
     * @brief Process WiFi events (should be called in loop)
     */
    void loop();

    /**
     * @brief Attempt to connect to WiFi
     * @param timeout Connection timeout in milliseconds
     * @return True if connected successfully
     */
    bool connect(unsigned long timeout = 10000);

    /**
     * @brief Start WiFi configuration portal
     * @param apName Name for the access point (optional)
     * @param timeout Timeout in seconds (0 = no timeout)
     * @return True if connected after portal
     */
    bool startConfigPortal(const char* apName = "ESP32-Thermostat-AP", unsigned int timeout = 0);

    /**
     * @brief Get current WiFi connection state
     * @return Current connection state
     */
    WiFiConnectionState getState() const;

    /**
     * @brief Get current WiFi signal strength
     * @return RSSI value or 0 if not connected
     */
    int getSignalStrength() const;

    /**
     * @brief Get time since last successful connection
     * @return Time in milliseconds
     */
    unsigned long getTimeSinceLastConnection() const;

    /**
     * @brief Register a callback for state changes
     * @param callback Function to call on state change
     */
    void registerStateCallback(WiFiStateCallback callback);

    /**
     * @brief Check if configuration portal is currently active
     * @return True if active
     */
    bool isConfigPortalActive() const;

    /**
     * @brief Get WiFiManager instance for advanced configuration
     * @return Reference to WiFiManager instance
     */
    WiFiManager& getWiFiManager();

    /**
     * @brief Get number of reconnection attempts since last success
     * @return Reconnection attempts count
     */
    int getReconnectAttempts() const;

    /**
     * @brief Reset reconnection attempt counter
     */
    void resetReconnectAttempts();

private:
    // Private constructor for singleton
    WiFiConnectionManager();
    
    // Delete copy constructor and assignment operator
    WiFiConnectionManager(const WiFiConnectionManager&) = delete;
    WiFiConnectionManager& operator=(const WiFiConnectionManager&) = delete;
    
    // Private methods
    void setState(WiFiConnectionState newState);
    void setupWiFiManagerCallbacks();
    void logWiFiStatus(const char* message);
    
    // Member variables
    static WiFiConnectionManager* _instance;
    WiFiManager _wifiManager;
    ConfigManager* _configManager;
    
    // State tracking
    WiFiConnectionState _state;
    unsigned long _lastConnectedTime;
    unsigned long _lastStateChangeTime;
    int _reconnectAttempts;
    bool _configPortalStarted;
    
    // Signal strength tracking
    static const uint8_t SIGNAL_HISTORY_SIZE = 10;
    SignalStrengthRecord _signalHistory[SIGNAL_HISTORY_SIZE];
    uint8_t _signalHistoryIndex;
    
    // Callbacks
    std::vector<WiFiStateCallback> _stateCallbacks;
    
    // Constants
    static const char* TAG;  // For logging
};

#endif // WIFI_CONNECTION_H