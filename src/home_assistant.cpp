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
    // Get current timestamp for debugging
    unsigned long now = millis();
    String timestamp = String(now);
    
    Serial.println("Registering entities with Home Assistant at time: " + timestamp);
    
    // Temperature sensor - very simplified config
    String tempTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/temperature/config";
    String tempPayload = "{\"name\":\"Temperature\",\"device_class\":\"temperature\",\"state_topic\":\"esp32_thermostat/temperature\",\"unit_of_measurement\":\"°C\",\"value_template\":\"{{ value }}\",\"timestamp\":\"" + timestamp + "\"}";
    
    bool tempSuccess = _mqttClient.publish(tempTopic.c_str(), tempPayload.c_str(), true);
    Serial.print("Published temperature config: ");
    Serial.println(tempSuccess ? "Success" : "FAILED");
    Serial.print("Topic: ");
    Serial.println(tempTopic);
    Serial.print("Payload: ");
    Serial.println(tempPayload);
    
    // Humidity sensor
    String humTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/humidity/config";
    String humPayload = "{\"name\":\"Humidity\",\"device_class\":\"humidity\",\"state_topic\":\"esp32_thermostat/humidity\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{ value }}\",\"timestamp\":\"" + timestamp + "\"}";
    
    bool humSuccess = _mqttClient.publish(humTopic.c_str(), humPayload.c_str(), true);
    Serial.print("Published humidity config: ");
    Serial.println(humSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(humPayload);
    
    // Pressure sensor
    String presTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pressure/config";
    String presPayload = "{\"name\":\"Pressure\",\"device_class\":\"pressure\",\"state_topic\":\"esp32_thermostat/pressure\",\"unit_of_measurement\":\"hPa\",\"value_template\":\"{{ value }}\",\"timestamp\":\"" + timestamp + "\"}";
    
    bool presSuccess = _mqttClient.publish(presTopic.c_str(), presPayload.c_str(), true);
    Serial.print("Published pressure config: ");
    Serial.println(presSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(presPayload);
    
    // Valve position sensor
    String valveTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/valve/config";
    String valvePayload = "{\"name\":\"Valve Position\",\"state_topic\":\"esp32_thermostat/valve/status\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{ value }}\",\"timestamp\":\"" + timestamp + "\"}";
    
    bool valveSuccess = _mqttClient.publish(valveTopic.c_str(), valvePayload.c_str(), true);
    Serial.print("Published valve config: ");
    Serial.println(valveSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(valvePayload);
    
    // Valve control (as a light with brightness)
    String valveControlTopic = String(HA_DISCOVERY_PREFIX) + "/light/" + _nodeId + "/valve_control/config";
    String valveControlPayload = "{\"name\":\"Valve Control\",\"schema\":\"json\",\"brightness\":true,\"command_topic\":\"esp32_thermostat/valve/set\",\"state_topic\":\"esp32_thermostat/valve/status\",\"brightness_scale\":100,\"icon\":\"mdi:radiator\",\"timestamp\":\"" + timestamp + "\"}";
    
    bool valveControlSuccess = _mqttClient.publish(valveControlTopic.c_str(), valveControlPayload.c_str(), true);
    Serial.print("Published valve control config: ");
    Serial.println(valveControlSuccess ? "Success" : "FAILED");
    Serial.print("Topic: ");
    Serial.println(valveControlTopic);
    Serial.print("Payload: ");
    Serial.println(valveControlPayload);
    
    // Subscribe to the valve control topic
    _mqttClient.subscribe("esp32_thermostat/valve/set");
    Serial.println("Subscribed to valve control topic");
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
    bool published = _mqttClient.publish(_availabilityTopic.c_str(), isOnline ? "online" : "offline", true);
    Serial.print("Published availability status (");
    Serial.print(isOnline ? "online" : "offline");
    Serial.print(") to ");
    Serial.print(_availabilityTopic);
    Serial.println(published ? " - Success" : " - FAILED");
}

// Publish discovery configuration for a single entity - kept for backward compatibility
void HomeAssistant::publishConfig(const char* component, const char* objectId, const char* name, 
                               const char* deviceClass, const char* stateTopic, const char* unit,
                               const char* commandTopic) {
    // This method is no longer used but kept for compatibility
    Serial.println("Warning: Obsolete publishConfig method called");
}