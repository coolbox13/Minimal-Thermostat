#ifndef HOME_ASSISTANT_H
#define HOME_ASSISTANT_H

#include <Arduino.h>
#include <PubSubClient.h>

class HomeAssistant {
public:
    HomeAssistant(PubSubClient& mqttClient, const char* nodeId);
    
    // Initialize Home Assistant auto discovery
    void begin();
    
    // Register device entities with Home Assistant
    void registerEntities();
    
    // Update availability status
    void updateAvailability(bool isOnline);
    
    // Send state updates for all sensors
    void updateStates(float temperature, float humidity, float pressure, int valvePosition);

private:
    PubSubClient& _mqttClient;
    String _nodeId;
    String _availabilityTopic;
    
    // Publish discovery configuration
    void publishConfig(const char* component, const char* objectId, const char* name, 
                      const char* deviceClass, const char* stateTopic, const char* unit = nullptr,
                      const char* commandTopic = nullptr);
    
    // Publish JSON message
    void publishJson(const char* topic, const char* payload);
};

#endif // HOME_ASSISTANT_H