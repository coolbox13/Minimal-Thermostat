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
    
    // NEW: Update setpoint temperature
    void updateSetpointTemperature(float setpoint);
    
    // NEW: Update thermostat mode
    void updateMode(const char* mode);

private:
    PubSubClient& _mqttClient;
    String _nodeId;
    String _availabilityTopic;
    
    // Kept for backward compatibility - not used anymore
    void publishConfig(const char* component, const char* objectId, const char* name, 
                      const char* deviceClass, const char* stateTopic, const char* unit = nullptr,
                      const char* commandTopic = nullptr);
};

#endif // HOME_ASSISTANT_H