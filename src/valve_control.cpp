#include "valve_control.h"

// Constructor
ValveControl::ValveControl(PubSubClient& mqttClient, ESPKNXIP& knx)
    : _mqttClient(mqttClient), _knx(knx), _targetPosition(0), _actualStatus(0) {
    
    // Default topics
    _positionTopic = "esp32_thermostat/valve/set";
    _statusTopic = "esp32_thermostat/valve/status";
}

// Initialize valve control
void ValveControl::begin() {
    Serial.println("Initializing valve control...");
    
    // Set up KNX addresses
    _valveSetAddress = _knx.GA_to_address(10, 5, 3);     // Real valve control address
    _valveStatusAddress = _knx.GA_to_address(10, 5, 8);  // Real valve status address
    _testValveAddress = _knx.GA_to_address(10, 2, 2);    // Test valve address
    
    // Initial status update
    publishStatus();
    
    Serial.println("Valve control initialized");
}

// Set valve position (0-100%)
void ValveControl::setPosition(int position) {
    // Constrain position to 0-100%
    position = constrain(position, 0, 100);
    
    if (position != _targetPosition) {
        _targetPosition = position;
        Serial.print("Setting valve position to: ");
        Serial.print(_targetPosition);
        Serial.println("%");
        
        // Send to test KNX address
        sendToKNX(_targetPosition);
        
        // Update MQTT status
        publishStatus();
    }
}

// Update valve status from KNX
void ValveControl::updateStatus(int status) {
    if (status != _actualStatus) {
        _actualStatus = status;
        Serial.print("Valve status updated from KNX: ");
        Serial.print(_actualStatus);
        Serial.println("%");
        
        // Update MQTT status
        publishStatus();
    }
}

// Get current target valve position
int ValveControl::getPosition() {
    return _targetPosition;
}

// Get current valve status from KNX
int ValveControl::getStatus() {
    return _actualStatus;
}

// Register with Home Assistant
void ValveControl::registerWithHA(const char* nodeId) {
    if (!_mqttClient.connected()) return;
    
    // Create discovery topics
    String discoveryPrefix = "homeassistant";
    
    // Valve position sensor config
    String sensorTopic = discoveryPrefix + "/sensor/" + nodeId + "/valve_position/config";
    String sensorPayload = "{";
    sensorPayload += "\"name\":\"Valve Position\",";
    sensorPayload += "\"state_topic\":\"" + _statusTopic + "\",";
    sensorPayload += "\"unit_of_measurement\":\"%\",";
    sensorPayload += "\"icon\":\"mdi:valve\"";
    sensorPayload += "}";
    
    _mqttClient.publish(sensorTopic.c_str(), sensorPayload.c_str(), true);
    
    // Valve control slider config
    String sliderTopic = discoveryPrefix + "/number/" + nodeId + "/valve_control/config";
    String sliderPayload = "{";
    sliderPayload += "\"name\":\"Valve Control\",";
    sliderPayload += "\"command_topic\":\"" + _positionTopic + "\",";
    sliderPayload += "\"state_topic\":\"" + _statusTopic + "\",";
    sliderPayload += "\"min\":0,";
    sliderPayload += "\"max\":100,";
    sliderPayload += "\"step\":1,";
    sliderPayload += "\"unit_of_measurement\":\"%\",";
    sliderPayload += "\"icon\":\"mdi:radiator\"";
    sliderPayload += "}";
    
    _mqttClient.publish(sliderTopic.c_str(), sliderPayload.c_str(), true);
    
    Serial.println("Valve control registered with Home Assistant");
}

// Process incoming MQTT messages
bool ValveControl::processMQTTMessage(const char* topic, const char* payload) {
    // Check if this is a valve control message
    if (strcmp(topic, _positionTopic.c_str()) == 0) {
        int position = atoi(payload);
        setPosition(position);
        return true;
    }
    
    return false; // Not handled by this module
}

// Publish valve position to MQTT
void ValveControl::publishStatus() {
    if (!_mqttClient.connected()) return;
    
    char valveStr[4];
    itoa(_targetPosition, valveStr, 10);
    _mqttClient.publish(_statusTopic.c_str(), valveStr, true);
}

// Send position to KNX (test address only for now)
void ValveControl::sendToKNX(int position) {
    // Send to the TEST KNX address, not the real valve address
    _knx.write_1byte_int(_testValveAddress, position);
    
    Serial.print("Sent valve position to KNX test address (");
    Serial.print(_testValveAddress.ga.main);
    Serial.print("/");
    Serial.print(_testValveAddress.ga.middle);
    Serial.print("/");  
    Serial.print(_testValveAddress.ga.sub);
    Serial.print("): ");
    Serial.println(position);
}