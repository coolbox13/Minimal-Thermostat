#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include "home_assistant.h"
#include "config.h"

// Forward declaration
class KNXManager;

class MQTTManager {
public:
    MQTTManager(PubSubClient& mqttClient);
    ~MQTTManager();
    
    // Initialize MQTT communication
    void begin();
    
    // Process MQTT messages in the main loop
    void loop();
    
    // Set KNX manager for cross-communication
    void setKNXManager(KNXManager* knxManager);
    
    // Publish sensor data to MQTT
    void publishSensorData(float temperature, float humidity, float pressure);
    
    // Set valve position (from KNX)
    void setValvePosition(int position);
    
    // Get current valve position
    int getValvePosition() const;
    
    // MQTT callback function
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    
    // Check if MQTT is connected
    bool isConnected();
    
    // Reconnect to MQTT if disconnected
    void reconnect();

private:
    PubSubClient& _mqttClient;
    KNXManager* _knxManager;
    HomeAssistant* _homeAssistant;
    int _valvePosition;
    
    // Static pointer to instance for callback
    static MQTTManager* _instance;
    
    // Process incoming MQTT message
    void processMessage(char* topic, byte* payload, unsigned int length);
};

#endif // MQTT_MANAGER_H