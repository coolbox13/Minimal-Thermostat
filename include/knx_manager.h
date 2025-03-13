#ifndef KNX_MANAGER_H
#define KNX_MANAGER_H

#include <Arduino.h>
#include <esp-knx-ip.h>
#include "config.h"

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
    
    // Send sensor data to KNX
    void sendSensorData(float temperature, float humidity, float pressure);
    
    // Set valve position
    void setValvePosition(int position);
    
    // Get current valve position
    int getValvePosition() const;
    
    // KNX callback function
    static void knxCallback(message_t const &msg, void *arg);

private:
    ESPKNXIP& _knx;
    MQTTManager* _mqttManager;
    int _valvePosition;
    
    // KNX group addresses
    address_t _valveAddress;
    address_t _temperatureAddress;
    address_t _humidityAddress;
    address_t _pressureAddress;
    address_t _testValveAddress;
    
    // Setup KNX addresses
    void setupAddresses();
};

#endif // KNX_MANAGER_H