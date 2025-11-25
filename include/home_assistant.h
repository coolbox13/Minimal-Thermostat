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

    // Update setpoint temperature
    void updateSetpointTemperature(float setpoint);

    // Update thermostat mode
    void updateMode(const char* mode);

    // Update preset mode
    void updatePresetMode(const char* preset);

    // Update PID parameters
    void updatePIDParameters(float kp, float ki, float kd);

    // Update system diagnostics (WiFi signal strength, uptime)
    void updateDiagnostics(int wifiRSSI, unsigned long uptime);

    // Update manual valve override status
    void updateManualOverride(bool enabled, int position);

    // HA FIX #5: Sync all climate state to HA (mode, preset, setpoint, action)
    // Call this periodically or after any state change to keep HA in sync
    void syncClimateState();

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