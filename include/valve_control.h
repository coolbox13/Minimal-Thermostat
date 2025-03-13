#ifndef VALVE_CONTROL_H
#define VALVE_CONTROL_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <esp-knx-ip.h>

class ValveControl {
public:
    ValveControl(PubSubClient& mqttClient, ESPKNXIP& knx);
    
    // Initialize valve control
    void begin();
    
    // Set valve position (0-100%)
    void setPosition(int position);
    
    // Update valve status from KNX
    void updateStatus(int status);
    
    // Get current valve position
    int getPosition();
    
    // Get current valve status from KNX
    int getStatus();
    
    // Register with Home Assistant
    void registerWithHA(const char* nodeId);
    
    // Process incoming MQTT messages
    bool processMQTTMessage(const char* topic, const char* payload);

private:
    PubSubClient& _mqttClient;
    ESPKNXIP& _knx;
    
    // Configuration
    address_t _valveSetAddress;      // KNX address for valve control
    address_t _valveStatusAddress;   // KNX address for valve status
    address_t _testValveAddress;     // Test KNX address (10/2/2)
    
    // State
    int _targetPosition;  // Target valve position (0-100%)
    int _actualStatus;    // Actual status reported by KNX
    
    // Topics
    String _positionTopic;     // MQTT topic for position command
    String _statusTopic;       // MQTT topic for status
    
    // Publish valve position to MQTT
    void publishStatus();
    
    // Send position to KNX (test address only)
    void sendToKNX(int position);
};

#endif // VALVE_CONTROL_H