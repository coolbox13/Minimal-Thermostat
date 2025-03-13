#include "home_assistant.h"
#include "config.h"

// Define discovery topic prefix according to Home Assistant standard
#define HA_DISCOVERY_PREFIX "homeassistant"

// Constructor
HomeAssistant::HomeAssistant(PubSubClient& mqttClient, const char* nodeId) 
    : _mqttClient(mqttClient), _nodeId(nodeId) {
    
    // Set up availability topic
    _availabilityTopic = String("esp32_thermostat/status");
}

// Initialize Home Assistant auto discovery
void HomeAssistant::begin() {
    Serial.println("Initializing Home Assistant auto discovery...");
    
    // First ensure we're connected
    if (!_mqttClient.connected()) {
        Serial.println("MQTT client not connected, cannot initialize Home Assistant discovery");
        return;
    }
    
    // Register auto-discovery information
    registerEntities();
    
    // Set initial availability
    updateAvailability(true);
    
    Serial.println("Home Assistant auto discovery initialized");
}

// Register all entities for auto discovery
void HomeAssistant::registerEntities() {
    // Temperature sensor - very simplified config
    String tempTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/temperature/config";
    String tempPayload = "{\"name\":\"Temperature\",\"device_class\":\"temperature\",\"state_topic\":\"esp32_thermostat/temperature\",\"unit_of_measurement\":\"°C\",\"value_template\":\"{{ value }}\"}";
    
    bool tempSuccess = _mqttClient.publish(tempTopic.c_str(), tempPayload.c_str(), true);
    Serial.print("Published temperature config: ");
    Serial.println(tempSuccess ? "Success" : "FAILED");
    Serial.print("Topic: ");
    Serial.println(tempTopic);
    
    // Humidity sensor
    String humTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/humidity/config";
    String humPayload = "{\"name\":\"Humidity\",\"device_class\":\"humidity\",\"state_topic\":\"esp32_thermostat/humidity\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{ value }}\"}";
    
    bool humSuccess = _mqttClient.publish(humTopic.c_str(), humPayload.c_str(), true);
    Serial.print("Published humidity config: ");
    Serial.println(humSuccess ? "Success" : "FAILED");
    
    // Pressure sensor
    String presTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pressure/config";
    String presPayload = "{\"name\":\"Pressure\",\"device_class\":\"pressure\",\"state_topic\":\"esp32_thermostat/pressure\",\"unit_of_measurement\":\"hPa\",\"value_template\":\"{{ value }}\"}";
    
    bool presSuccess = _mqttClient.publish(presTopic.c_str(), presPayload.c_str(), true);
    Serial.print("Published pressure config: ");
    Serial.println(presSuccess ? "Success" : "FAILED");
    
    // Valve position
    String valveTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/valve/config";
    String valvePayload = "{\"name\":\"Valve Position\",\"state_topic\":\"esp32_thermostat/valve/status\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{ value }}\"}";
    
    bool valveSuccess = _mqttClient.publish(valveTopic.c_str(), valvePayload.c_str(), true);
    Serial.print("Published valve config: ");
    Serial.println(valveSuccess ? "Success" : "FAILED");
}

// Send state updates for each entity
void HomeAssistant::updateStates(float temperature, float humidity, float pressure, int valvePosition) {
    // Convert values to strings and publish to the actual topics you're using
    char tempStr[8];
    dtostrf(temperature, 1, 2, tempStr);
    _mqttClient.publish("esp32_thermostat/temperature", tempStr);
    
    char humStr[8];
    dtostrf(humidity, 1, 2, humStr);
    _mqttClient.publish("esp32_thermostat/humidity", humStr);
    
    char presStr[8];
    dtostrf(pressure, 1, 2, presStr);
    _mqttClient.publish("esp32_thermostat/pressure", presStr);
    
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    _mqttClient.publish("esp32_thermostat/valve/status", valveStr);
    
    // Also publish a general "online" status message
    _mqttClient.publish("esp32_thermostat/status", "online", true);
}

// Update availability status
void HomeAssistant::updateAvailability(bool isOnline) {
    _mqttClient.publish(_availabilityTopic.c_str(), isOnline ? "online" : "offline", true);
}

// Publish discovery configuration for a single entity - Not used in simplified version
void HomeAssistant::publishConfig(const char* component, const char* objectId, const char* name, 
                               const char* deviceClass, const char* stateTopic, const char* unit,
                               const char* commandTopic) {
    // This function is not used in the simplified implementation
}

// Helper for publishing JSON messages
void HomeAssistant::publishJson(const char* topic, const char* payload) {
    _mqttClient.publish(topic, payload, true);
}