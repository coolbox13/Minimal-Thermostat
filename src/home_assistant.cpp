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
    
    // Temperature sensor (still needed for raw temperature data)
    String tempTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/temperature/config";
    String tempPayload = "{\"name\":\"Temperature\",\"unique_id\":\"" + String(_nodeId) + "_temperature\",\"device_class\":\"temperature\",\"state_topic\":\"esp32_thermostat/temperature\",\"unit_of_measurement\":\"°C\",\"value_template\":\"{{ value }}\",\"availability_topic\":\"esp32_thermostat/status\",\"device\":" + deviceInfo + ",\"timestamp\":\"" + timestamp + "\"}";
    
    bool tempSuccess = _mqttClient.publish(tempTopic.c_str(), tempPayload.c_str(), true);
    Serial.print("Published temperature config: ");
    Serial.println(tempSuccess ? "Success" : "FAILED");
    
    // Humidity sensor
    String humTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/humidity/config";
    String humPayload = "{\"name\":\"Humidity\",\"unique_id\":\"" + String(_nodeId) + "_humidity\",\"device_class\":\"humidity\",\"state_topic\":\"esp32_thermostat/humidity\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{ value }}\",\"availability_topic\":\"esp32_thermostat/status\",\"device\":" + deviceInfo + ",\"timestamp\":\"" + timestamp + "\"}";
    
    bool humSuccess = _mqttClient.publish(humTopic.c_str(), humPayload.c_str(), true);
    Serial.print("Published humidity config: ");
    Serial.println(humSuccess ? "Success" : "FAILED");
    
    // Pressure sensor
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
    
    // Valve position sensor - for raw data
    String valvePosTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/valve_position/config";
    String valuePosPayload = "{";
    valuePosPayload += "\"name\":\"Valve Position\",";
    valuePosPayload += "\"unique_id\":\"" + _nodeId + "_valve_position\",";
    valuePosPayload += "\"state_topic\":\"esp32_thermostat/valve/position\",";
    valuePosPayload += "\"unit_of_measurement\":\"%\",";
    valuePosPayload += "\"value_template\":\"{{ value }}\",";
    valuePosPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    valuePosPayload += "\"device\":" + deviceInfo + ",";
    valuePosPayload += "\"timestamp\":\"" + timestamp + "\"";
    valuePosPayload += "}";
    
    bool valvePosSuccess = _mqttClient.publish(valvePosTopic.c_str(), valuePosPayload.c_str(), true);
    Serial.print("Published valve position config: ");
    Serial.println(valvePosSuccess ? "Success" : "FAILED");
    
    // NEW: Climate entity for thermostat
    String climateTopic = String(HA_DISCOVERY_PREFIX) + "/climate/" + _nodeId + "/thermostat/config";
    String climatePayload = "{";
    climatePayload += "\"name\":\"KNX Thermostat\",";
    climatePayload += "\"unique_id\":\"" + String(_nodeId) + "_thermostat\",";
    climatePayload += "\"modes\":[\"off\",\"heat\"],";
    climatePayload += "\"mode_command_topic\":\"esp32_thermostat/mode/set\",";
    climatePayload += "\"mode_state_topic\":\"esp32_thermostat/mode/state\",";
    climatePayload += "\"temperature_command_topic\":\"esp32_thermostat/temperature/set\",";
    climatePayload += "\"temperature_state_topic\":\"esp32_thermostat/temperature/setpoint\",";
    climatePayload += "\"current_temperature_topic\":\"esp32_thermostat/temperature\",";
    climatePayload += "\"temperature_unit\":\"C\",";
    climatePayload += "\"min_temp\":\"15\",";
    climatePayload += "\"max_temp\":\"30\",";
    climatePayload += "\"temp_step\":\"0.5\",";
    climatePayload += "\"action_topic\":\"esp32_thermostat/action\",";
    climatePayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    climatePayload += "\"device\":" + deviceInfo;
    climatePayload += "}";
    
    delay(100); // Small delay to ensure MQTT client can process
    bool climateSuccess = _mqttClient.publish(climateTopic.c_str(), climatePayload.c_str(), true);
    Serial.print("Published climate config: ");
    Serial.println(climateSuccess ? "Success" : "FAILED");
    
    // Subscribe to the thermostat control topics
    _mqttClient.subscribe("esp32_thermostat/mode/set");
    _mqttClient.subscribe("esp32_thermostat/temperature/set");
    Serial.println("Subscribed to thermostat control topics");
    
    // Publish initial states
    _mqttClient.publish("esp32_thermostat/mode/state", "heat", true);
    
    // Publish initial setpoint from PID config
    char setpointStr[8];
    dtostrf(PID_SETPOINT, 1, 1, setpointStr);
    _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
    
    // Add restart command option
    _mqttClient.subscribe("esp32_thermostat/restart");
}

// Send state updates for each entity
void HomeAssistant::updateStates(float temperature, float humidity, float pressure, int valvePosition) {
    // Convert values to strings and publish
    char tempStr[8];
    dtostrf(temperature, 1, 2, tempStr);
    _mqttClient.publish("esp32_thermostat/temperature", tempStr);
    
    char humStr[8];
    dtostrf(humidity, 1, 2, humStr);
    _mqttClient.publish("esp32_thermostat/humidity", humStr);
    
    char presStr[8];
    dtostrf(pressure, 1, 2, presStr);
    _mqttClient.publish("esp32_thermostat/pressure", presStr);
    
    // For the valve position
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    _mqttClient.publish("esp32_thermostat/valve/position", valveStr);
    
    // Update action state based on valve position
    if (valvePosition > 0) {
        _mqttClient.publish("esp32_thermostat/action", "heating");
    } else {
        _mqttClient.publish("esp32_thermostat/action", "idle");
    }
    
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

// NEW: Update the setpoint temperature
void HomeAssistant::updateSetpointTemperature(float setpoint) {
    char setpointStr[8];
    dtostrf(setpoint, 1, 1, setpointStr);
    _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
}

// NEW: Update the thermostat mode
void HomeAssistant::updateMode(const char* mode) {
    _mqttClient.publish("esp32_thermostat/mode/state", mode, true);
}