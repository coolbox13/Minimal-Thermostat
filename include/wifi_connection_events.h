#ifndef WIFI_CONNECTION_EVENTS_H
#define WIFI_CONNECTION_EVENTS_H

#include <Arduino.h>
#include <functional>

/**
 * @file wifi_connection_events.h
 * @brief Event definitions and callback types for WiFi connection events
 * 
 * This file defines the event types and callback function signatures used
 * by the WiFiConnectionManager class to notify subscribers about WiFi events.
 */

// Forward declaration of state enum from wifi_connection.h
enum class WiFiConnectionState;

/**
 * @brief Types of WiFi connection events
 */
// Add STATE_CHANGED to the enum if it's missing
enum class WiFiEventType {
    CONNECTED,
    DISCONNECTED,
    CONNECTING,
    CONNECTION_LOST,
    CONNECTION_FAILED,
    CONFIG_PORTAL_STARTED,
    CONFIG_PORTAL_STOPPED,
    INTERNET_CONNECTED,
    INTERNET_LOST,
    SIGNAL_CHANGED,
    STATE_CHANGED,
    CREDENTIALS_SAVED  // Add this missing value
};

// Forward declare the network info struct to avoid union issues
struct NetworkInfo {
    IPAddress ip;            // IP address
    IPAddress gateway;       // Gateway IP
    IPAddress subnet;        // Subnet mask
    IPAddress dns1;          // Primary DNS
    IPAddress dns2;          // Secondary DNS
};

struct ConnectingInfo {
    int reconnectAttempt;    // Current reconnection attempt number
};

/**
 * @brief Data structure for WiFi connection events
 */
struct WiFiConnectionEvent {
    WiFiEventType type;              // Type of event
    WiFiConnectionState oldState;    // Previous connection state
    WiFiConnectionState newState;    // New connection state
    String ssid;                     // SSID (if applicable)
    String message;                  // Optional descriptive message
    int signalStrength;              // Signal strength in dBm (if applicable)
    int signalQuality;               // Signal quality 0-100% (if applicable)
    unsigned long timestamp;         // Event timestamp (millis)
    
    // Additional data for specific events (use pointers instead of union)
    ConnectingInfo connecting;       // Connection attempt info
    NetworkInfo networkInfo;         // Network information when connected
};

/**
 * @brief Callback function type for WiFi connection events
 */
typedef std::function<void(const WiFiConnectionEvent& event)> WiFiEventCallback;

/**
 * @brief Legacy callback type for simple state changes
 */
typedef std::function<void(WiFiConnectionState newState, WiFiConnectionState oldState)> WiFiStateCallback;

/**
 * @brief Predefined event filter that accepts all events
 */
inline bool acceptAllEvents(const WiFiConnectionEvent& event) {
    return true;
}

/**
 * @brief Callback function type with event filtering
 */
typedef std::function<bool(const WiFiConnectionEvent&)> WiFiEventFilter;

/**
 * @brief Event subscription information
 */
struct EventSubscription {
    WiFiEventCallback callback;
    WiFiEventFilter filter;
    int id;
};

#endif // WIFI_CONNECTION_EVENTS_H