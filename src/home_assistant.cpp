#include "home_assistant.h"
#include "config.h"

// Define discovery topic prefix according to Home Assistant standard
#define HA_DISCOVERY_PREFIX "homeassistant"

// Constructor
HomeAssistant::HomeAssistant(PubSubClient& mqttClient, const char* nodeId) 
    : _mqttClient(mqttClient), _nodeId(nodeId) {
    
    // Set up availability topic
    _availabilityTopic = String("thermostat/") + _nodeId + "/status";
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
    // Temperature sensor
    publishConfig(
        "sensor",                      // component type
        "temperature",                 // object id 
        "Temperature",                 // friendly name
        "temperature",                 // device class
        MQTT_TOPIC_TEMPERATURE,        // state topic
        "°C"                           // unit of measurement
    );
    
    // Humidity sensor
    publishConfig(
        "sensor", 
        "humidity", 
        "Humidity", 
        "humidity", 
        MQTT_TOPIC_HUMIDITY,
        "%"
    );
    
    // Pressure sensor
    publishConfig(
        "sensor", 
        "pressure", 
        "Pressure", 
        "pressure", 
        MQTT_TOPIC_PRESSURE,
        "hPa"
    );
    
    // Valve position
    publishConfig(
        "sensor", 
        "valve", 
        "Valve Position", 
        "None", 
        MQTT_TOPIC_VALVE_STATUS,
        "%"
    );
}

// Send state updates for each entity
void HomeAssistant::updateStates(float temperature, float humidity, float pressure, int valvePosition) {
    // Convert values to strings and publish
    char tempStr[8];
    dtostrf(temperature, 1, 2, tempStr);
    _mqttClient.publish(MQTT_TOPIC_TEMPERATURE, tempStr);
    
    char humStr[8];
    dtostrf(humidity, 1, 2, humStr);
    _mqttClient.publish(MQTT_TOPIC_HUMIDITY, humStr);
    
    char presStr[8];
    dtostrf(pressure, 1, 2, presStr);
    _mqttClient.publish(MQTT_TOPIC_PRESSURE, presStr);
    
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    _mqttClient.publish(MQTT_TOPIC_VALVE_STATUS, valveStr);
}

// Update availability status
void HomeAssistant::updateAvailability(bool isOnline) {
    _mqttClient.publish(_availabilityTopic.c_str(), isOnline ? "online" : "offline", true);
}

// Publish discovery configuration for a single entity
void HomeAssistant::publishConfig(const char* component, const char* objectId, const char* name, 
                               const char* deviceClass, const char* stateTopic, const char* unit,
                               const char* commandTopic) {
    
    // Create discovery topic
    String discoveryTopic = String(HA_DISCOVERY_PREFIX) + "/" + component + "/" + 
                           _nodeId + "/" + objectId + "/config";
    
    // Build configuration JSON
    String payload = "{";
    
    // Required fields
    payload += "\"name\":\"" + String(name) + "\"";
    payload += ",\"unique_id\":\"" + _nodeId + "_" + objectId + "\"";
    payload += ",\"state_topic\":\"" + String(stateTopic) + "\"";
    
    // Add availability information
    payload += ",\"availability_topic\":\"" + _availabilityTopic + "\"";
    
    // Optional fields
    if (deviceClass && strcmp(deviceClass, "None") != 0) {
        payload += ",\"device_class\":\"" + String(deviceClass) + "\"";
    }
    
    if (unit) {
        payload += ",\"unit_of_measurement\":\"" + String(unit) + "\"";
    }
    
    if (commandTopic) {
        payload += ",\"command_topic\":\"" + String(commandTopic) + "\"";
    }
    
    // Device information
    payload += ",\"device\":{";
    payload += "\"identifiers\":[\"" + _nodeId + "\"]";
    payload += ",\"name\":\"ESP32 KNX Thermostat\"";
    payload += ",\"model\":\"ESP32-KNX-Thermostat\"";
    payload += ",\"manufacturer\":\"DIY\"";
    payload += "}";
    
    payload += "}";
    
    // Publish with retain flag
    _mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
    
    Serial.print("Published Home Assistant discovery for ");
    Serial.println(name);
}

// Helper for publishing JSON messages
void HomeAssistant::publishJson(const char* topic, const char* payload) {
    _mqttClient.publish(topic, payload, true);
}