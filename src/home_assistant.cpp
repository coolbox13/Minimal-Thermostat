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
    
    // Create device info JSON string that will be reused for all entities
    String deviceInfo = "{\"identifiers\":[\"" + String(_nodeId) + "\"],\"name\":\"ESP32 KNX Thermostat\",\"model\":\"ESP32-KNX-Thermostat\",\"manufacturer\":\"DIY\",\"sw_version\":\"1.0\"}";
    
    // Temperature sensor - very simplified config
    String tempTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/temperature/config";
    String tempPayload = "{\"name\":\"Temperature\",\"unique_id\":\"" + String(_nodeId) + "_temperature\",\"device_class\":\"temperature\",\"state_topic\":\"esp32_thermostat/temperature\",\"unit_of_measurement\":\"°C\",\"value_template\":\"{{ value }}\",\"availability_topic\":\"esp32_thermostat/status\",\"device\":" + deviceInfo + ",\"timestamp\":\"" + timestamp + "\"}";
    
    bool tempSuccess = _mqttClient.publish(tempTopic.c_str(), tempPayload.c_str(), true);
    Serial.print("Published temperature config: ");
    Serial.println(tempSuccess ? "Success" : "FAILED");
    Serial.print("Topic: ");
    Serial.println(tempTopic);
    Serial.print("Payload: ");
    Serial.println(tempPayload);
    
    // Humidity sensor
    String humTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/humidity/config";
    String humPayload = "{\"name\":\"Humidity\",\"unique_id\":\"" + String(_nodeId) + "_humidity\",\"device_class\":\"humidity\",\"state_topic\":\"esp32_thermostat/humidity\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{ value }}\",\"availability_topic\":\"esp32_thermostat/status\",\"device\":" + deviceInfo + ",\"timestamp\":\"" + timestamp + "\"}";
    
    bool humSuccess = _mqttClient.publish(humTopic.c_str(), humPayload.c_str(), true);
    Serial.print("Published humidity config: ");
    Serial.println(humSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(humPayload);
    
    // Pressure sensor - with unique_id and device info
    String presTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pressure/config";
    String presPayload = "{";
    presPayload += "\"name\":\"Pressure\",";
    presPayload += "\"unique_id\":\"" + _nodeId + "_pressure\",";
    presPayload += "\"device_class\":\"pressure\",";
    presPayload += "\"state_topic\":\"esp32_thermostat/pressure\",";
    presPayload += "\"unit_of_measurement\":\"hPa\",";
    presPayload += "\"value_template\":\"{{ value }}\",";
    presPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    presPayload += "\"device\":" + deviceInfo + ",";
    presPayload += "\"timestamp\":\"" + timestamp + "\"";
    presPayload += "}";
    
    bool presSuccess = _mqttClient.publish(presTopic.c_str(), presPayload.c_str(), true);
    Serial.print("Published pressure config: ");
    Serial.println(presSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(presPayload);
    
    // In the registerEntities method, update the valve position sensor:
    
    // Valve position sensor - with unique_id and device info
    String valvePosTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/valve_position/config";
    String valuePosPayload = "{";
    valuePosPayload += "\"name\":\"Valve Position\",";
    valuePosPayload += "\"unique_id\":\"" + _nodeId + "_valve_position\",";
    valuePosPayload += "\"state_topic\":\"esp32_thermostat/valve/position\",";  // Make sure this matches where you publish
    valuePosPayload += "\"unit_of_measurement\":\"%\",";
    valuePosPayload += "\"value_template\":\"{{ value }}\",";  // Simple template that just passes the value
    valuePosPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    valuePosPayload += "\"device\":" + deviceInfo + ",";
    valuePosPayload += "\"timestamp\":\"" + timestamp + "\"";
    valuePosPayload += "}";
    
    bool valvePosSuccess = _mqttClient.publish(valvePosTopic.c_str(), valuePosPayload.c_str(), true);
    Serial.print("Published valve position config: ");
    Serial.println(valvePosSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(valuePosPayload);
    
    // Valve control - update to use the deviceInfo variable
    // Replace the climate entity with a number entity:
    
    // Valve control - use number entity which is simpler
    String valveControlTopic = String(HA_DISCOVERY_PREFIX) + "/number/" + _nodeId + "/valve_control/config";
    String valveControlPayload = "{";
    valveControlPayload += "\"name\":\"Valve Control\",";
    valveControlPayload += "\"unique_id\":\"" + String(_nodeId) + "_valve_control\",";
    valveControlPayload += "\"command_topic\":\"esp32_thermostat/valve/set\",";
    valveControlPayload += "\"state_topic\":\"esp32_thermostat/valve/position\",";  // Use the same topic as the sensor
    valveControlPayload += "\"min\":0,";
    valveControlPayload += "\"max\":100,";
    valveControlPayload += "\"step\":1,";
    valveControlPayload += "\"unit_of_measurement\":\"%\",";
    valveControlPayload += "\"icon\":\"mdi:radiator\",";
    valveControlPayload += "\"mode\":\"slider\",";
    valveControlPayload += "\"availability_topic\":\"esp32_thermostat/status\",";
    valveControlPayload += "\"device\":" + deviceInfo;
    valveControlPayload += "}";
    
    // Try publishing with a delay to ensure MQTT client has time to process
    delay(100);
    bool valveControlSuccess = _mqttClient.publish(valveControlTopic.c_str(), valveControlPayload.c_str(), true);
    Serial.print("Published valve control config: ");
    Serial.println(valveControlSuccess ? "Success" : "FAILED");
    Serial.print("Topic: ");
    Serial.println(valveControlTopic);
    Serial.print("Payload: ");
    Serial.println(valveControlPayload);
    
    // Subscribe to the valve control topic
    _mqttClient.subscribe("esp32_thermostat/valve/set");
    _mqttClient.subscribe("esp32_thermostat/valve/mode");
    Serial.println("Subscribed to valve control topics");
    
    // In the registerEntities method of your HomeAssistant class
    
    // Publish initial mode only once during registration
    _mqttClient.publish("esp32_thermostat/valve/mode", "heat", true);
    
    // Make sure you're not publishing this repeatedly in updateStates or other methods
}

// Send state updates for each entity
// In the updateStates method, modify how you publish valve position data:

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
    
    // For the valve position, ensure we're sending consistent formats
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    
    // For the sensor entity - send ONLY the numeric value
    _mqttClient.publish("esp32_thermostat/valve/position", valveStr);
    
    // For the climate entity - also send the numeric value
    // This ensures both entities receive compatible data
    _mqttClient.publish("esp32_thermostat/valve/status", valveStr);
    
    // Also publish a general "online" status message
    _mqttClient.publish(_availabilityTopic.c_str(), "online", true);
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