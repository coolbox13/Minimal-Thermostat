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
    
    // Create device info once (used for all entities)
    String deviceInfo = "{";
    deviceInfo += "\"identifiers\":[\"" + _nodeId + "\"],";
    deviceInfo += "\"name\":\"ESP32 KNX Thermostat\",";
    deviceInfo += "\"model\":\"ESP32-KNX-Thermostat\",";
    deviceInfo += "\"manufacturer\":\"DIY\",";
    deviceInfo += "\"sw_version\":\"1.0\"";
    deviceInfo += "}";
    
    // Temperature sensor - with unique_id and device info
    String tempTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/temperature/config";
    String tempPayload = "{";
    tempPayload += "\"name\":\"Temperature\",";
    tempPayload += "\"unique_id\":\"" + _nodeId + "_temperature\",";
    tempPayload += "\"device_class\":\"temperature\",";
    tempPayload += "\"state_topic\":\"esp32_thermostat/temperature\",";
    tempPayload += "\"unit_of_measurement\":\"°C\",";
    tempPayload += "\"value_template\":\"{{ value }}\",";
    tempPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    tempPayload += "\"device\":" + deviceInfo + ",";
    tempPayload += "\"timestamp\":\"" + timestamp + "\"";
    tempPayload += "}";
    
    bool tempSuccess = _mqttClient.publish(tempTopic.c_str(), tempPayload.c_str(), true);
    Serial.print("Published temperature config: ");
    Serial.println(tempSuccess ? "Success" : "FAILED");
    Serial.print("Topic: ");
    Serial.println(tempTopic);
    Serial.print("Payload: ");
    Serial.println(tempPayload);
    
    // Humidity sensor - with unique_id and device info
    String humTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/humidity/config";
    String humPayload = "{";
    humPayload += "\"name\":\"Humidity\",";
    humPayload += "\"unique_id\":\"" + _nodeId + "_humidity\",";
    humPayload += "\"device_class\":\"humidity\",";
    humPayload += "\"state_topic\":\"esp32_thermostat/humidity\",";
    humPayload += "\"unit_of_measurement\":\"%\",";
    humPayload += "\"value_template\":\"{{ value }}\",";
    humPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    humPayload += "\"device\":" + deviceInfo + ",";
    humPayload += "\"timestamp\":\"" + timestamp + "\"";
    humPayload += "}";
    
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
    
    /// Valve control as a proper valve entity
String valveControlTopic = String(HA_DISCOVERY_PREFIX) + "/valve/" + _nodeId + "/valve_control/config";
String valveControlPayload = "{";
valveControlPayload += "\"name\":\"Radiator Valve\",";
valveControlPayload += "\"unique_id\":\"" + _nodeId + "_valve_control\",";
valveControlPayload += "\"command_topic\":\"esp32_thermostat/valve/set\",";
valveControlPayload += "\"state_topic\":\"esp32_thermostat/valve/position\",";
valveControlPayload += "\"position_topic\":\"esp32_thermostat/valve/position\",";
valveControlPayload += "\"set_position_topic\":\"esp32_thermostat/valve/set\",";
valveControlPayload += "\"position_template\":\"{{ value }}\",";
valveControlPayload += "\"value_template\":\"{{ value }}\",";
valveControlPayload += "\"min_position\":0,";
valveControlPayload += "\"max_position\":100,";
valveControlPayload += "\"position_open\":100,";
valveControlPayload += "\"position_closed\":0,";
valveControlPayload += "\"payload_open\":\"100\",";
valveControlPayload += "\"payload_close\":\"0\",";
valveControlPayload += "\"payload_stop\":\"50\",";
valveControlPayload += "\"state_open\":\"100\",";
valveControlPayload += "\"state_closed\":\"0\",";
valveControlPayload += "\"reports_position\":true,";
valveControlPayload += "\"icon\":\"mdi:radiator-valve\",";
valveControlPayload += "\"device_class\":\"valve\",";
valveControlPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
valveControlPayload += "\"device\":" + deviceInfo + ",";
valveControlPayload += "\"timestamp\":\"" + timestamp + "\"";
valveControlPayload += "}";
    
    bool valveSuccess = _mqttClient.publish(valveTopic.c_str(), valvePayload.c_str(), true);
    Serial.print("Published valve position config: ");
    Serial.println(valveSuccess ? "Success" : "FAILED");
    Serial.print("Payload: ");
    Serial.println(valvePayload);
    
    // Valve control as a number entity (cleaner than a light entity)
    String valveControlTopic = String(HA_DISCOVERY_PREFIX) + "/number/" + _nodeId + "/valve_control/config";
    String valveControlPayload = "{";
    valveControlPayload += "\"name\":\"Valve Control\",";
    valveControlPayload += "\"unique_id\":\"" + _nodeId + "_valve_control\",";
    valveControlPayload += "\"command_topic\":\"esp32_thermostat/valve/set\",";
    valveControlPayload += "\"state_topic\":\"esp32_thermostat/valve/status\",";
    valveControlPayload += "\"min\":0,";
    valveControlPayload += "\"max\":100,";
    valveControlPayload += "\"step\":1,";
    valveControlPayload += "\"unit_of_measurement\":\"%\",";
    valveControlPayload += "\"icon\":\"mdi:radiator\",";
    valveControlPayload += "\"mode\":\"slider\",";
    valveControlPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    valveControlPayload += "\"device\":" + deviceInfo + ",";
    valveControlPayload += "\"timestamp\":\"" + timestamp + "\"";
    valveControlPayload += "}";
    
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
    
    // For the valve position, create both a plain value and a JSON formatted value
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    
    // For new number entity (plain number)
    _mqttClient.publish("esp32_thermostat/valve/position", valveStr);
    
    // For old light entity (JSON format)
    char jsonValveStr[50];
    sprintf(jsonValveStr, "{\"state\":\"%s\",\"brightness\":%d}", 
            valvePosition > 0 ? "ON" : "OFF", valvePosition);
    _mqttClient.publish("esp32_thermostat/valve/status", jsonValveStr, true);
    
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