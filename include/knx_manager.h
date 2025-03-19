#ifndef KNX_MANAGER_H
#define KNX_MANAGER_H

#include <Arduino.h>
#include <mutex>
#include <queue>
// Update the include path to use your local version
#include "esp-knx-ip/esp-knx-ip.h"
#include "config.h"
#include "logger.h"
#include "config_manager.h"

// Message structure for thread-safe queue
struct KnxMessage {
    enum MessageType {
        VALVE_POSITION,
        SEND_SENSOR_DATA
    };
    
    MessageType type;
    float value1; // Temperature or valve position
    float value2; // Humidity
    float value3; // Pressure
};

// Forward declaration
class MQTTManager;

class KNXManager {
public:
    KNXManager(ESPKNXIP& knx);
    
    // Initialize KNX communication
    void begin();
    
    // Process KNX messages in the main loop
    void loop();
    
    // Set MQTT manager for cross-communication
    void setMQTTManager(MQTTManager* mqttManager);
    
    // Send sensor data to KNX (thread-safe)
    void sendSensorData(float temperature, float humidity, float pressure);
    
    // Set valve position (thread-safe)
    void setValvePosition(int position);
    
    // Get current valve position (thread-safe)
    int getValvePosition() const;
    
    // KNX callback function
    static void knxCallback(message_t const &msg, void *arg);
    
    // Get the ESPKNXIP instance
    ESPKNXIP& getKnx() { return _knx; }
    
    // Reload KNX addresses from config
    void reloadAddresses();
    
    // Get current address mode
    bool isUsingTestAddresses() const;

private:
    ESPKNXIP& _knx;
    MQTTManager* _mqttManager;
    int _valvePosition;
    ConfigManager* _configManager;
    
    // Thread safety
    mutable std::mutex _mutex;
    std::queue<KnxMessage> _messageQueue;
    
    // Process queued messages
    void processQueue();
    
    // KNX group addresses
    address_t _valveAddress;
    address_t _temperatureAddress;
    address_t _humidityAddress;
    address_t _pressureAddress;
    address_t _testValveAddress;
    
    // Setup KNX addresses
    void setupAddresses();
    
    // Direct implementation for thread-safety
    void _setValvePosition(int position);
    void _sendSensorData(float temperature, float humidity, float pressure);
};

#endif // KNX_MANAGER_H