#include "mqtt_manager.h"
#include "knx_manager.h"
#include "config.h"

// Initialize static instance pointer
MQTTManager* MQTTManager::_instance = nullptr;

MQTTManager::MQTTManager(PubSubClient& mqttClient)
    : _mqttClient(mqttClient), _knxManager(nullptr), _homeAssistant(nullptr), _valvePosition(0) {
    // Store instance for static callback
    _instance = this;
}

MQTTManager::~MQTTManager() {
    if (_homeAssistant) {
        delete _homeAssistant;
    }
}

void MQTTManager::begin() {
    Serial.println("Setting up MQTT...");
    
    // Set server and callback
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setCallback(mqttCallback);
    _mqttClient.setBufferSize(512);  // Increased buffer size for discovery messages
    
    // Try to connect
    reconnect();
    
    // Initialize Home Assistant integration
    _homeAssistant = new HomeAssistant(_mqttClient, "esp32_thermostat");
    _homeAssistant->begin();
    
    Serial.println("MQTT initialized");
}

void MQTTManager::loop() {
    if (!_mqttClient.connected()) {
        reconnect();
    }
    _mqttClient.loop();
}

void MQTTManager::setKNXManager(KNXManager* knxManager) {
    _knxManager = knxManager;
}

void MQTTManager::publishSensorData(float temperature, float humidity, float pressure) {
    if (!_mqttClient.connected()) return;
    
    // Update Home Assistant with all sensor values
    if (_homeAssistant) {
        _homeAssistant->updateStates(temperature, humidity, pressure, _valvePosition);
    }
}

void MQTTManager::setValvePosition(int position) {
    // Constrain position to 0-100%
    position = constrain(position, 0, 100);
    
    if (position != _valvePosition) {
        _valvePosition = position;
        
        // Update Home Assistant if connected
        if (_mqttClient.connected() && _homeAssistant) {
            // For the valve position, create both a plain value
            char valveStr[4];
            itoa(_valvePosition, valveStr, 10);
            
            // Publish to valve position topic
            _mqttClient.publish("esp32_thermostat/valve/position", valveStr);
            
            Serial.print("Published valve position to MQTT: ");
            Serial.println(_valvePosition);
        }
    }
}

int MQTTManager::getValvePosition() const {
    return _valvePosition;
}

void MQTTManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Forward to instance method
    if (_instance) {
        _instance->processMessage(topic, payload, length);
    }
}

// In the processMessage method of your MQTTManager class

void MQTTManager::processMessage(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    char message[length + 1];
    for (unsigned int i = 0; i < length; i++) {
        message[i] = (char)payload[i];
    }
    message[length] = '\0';
    
    Serial.print("MQTT message received [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);
    
    // Handle valve position command
    if (strcmp(topic, MQTT_TOPIC_VALVE_COMMAND) == 0 || 
        strcmp(topic, "esp32_thermostat/valve/set") == 0) {
        
        int position = atoi(message);
        position = constrain(position, 0, 100);
        
        Serial.print("Parsed valve position: ");
        Serial.println(position);
        
        // Update valve position locally
        _valvePosition = position;
        
        // Update KNX if available
        if (_knxManager) {
            _knxManager->setValvePosition(position);
        } else {
            // If KNX manager not available, update MQTT directly
            setValvePosition(position);
        }
    }
    
    // Handle valve mode (for climate entity)
    if (strcmp(topic, "esp32_thermostat/valve/mode") == 0) {
        // Don't republish the mode - this is causing the feedback loop
        // _mqttClient.publish("esp32_thermostat/valve/mode", message, true);
        
        // Just log that we received it
        Serial.print("Valve mode received: ");
        Serial.println(message);
    }
}

bool MQTTManager::isConnected() {
    return _mqttClient.connected();
}

void MQTTManager::reconnect() {
    int attempts = 0;
    while (!_mqttClient.connected() && attempts < 3) {
        Serial.println("Attempting MQTT connection...");
        String clientId = "ESP32Thermostat-";
        clientId += String(random(0xffff), HEX);
        
        if (_mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("MQTT connected");
            
            // Subscribe to topics
            _mqttClient.subscribe(MQTT_TOPIC_VALVE_COMMAND);
            _mqttClient.subscribe("esp32_thermostat/valve/set");
            _mqttClient.subscribe("esp32_thermostat/valve/mode");
            
            // Update availability
            if (_homeAssistant) {
                _homeAssistant->updateAvailability(true);
            }
        } else {
            Serial.print("MQTT connection failed, rc=");
            Serial.print(_mqttClient.state());
            Serial.println(" trying again in 5 seconds");
            delay(5000);
            attempts++;
        }
    }
}