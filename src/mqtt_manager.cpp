#include "mqtt_manager.h"
#include "knx_manager.h"
#include "config.h"

// Initialize static instance pointer
MQTTManager* MQTTManager::_instance = nullptr;

MQTTManager::MQTTManager(PubSubClient& mqttClient)
    : _mqttClient(mqttClient), _knxManager(nullptr), _valvePosition(0) {
    // Store instance for static callback
    _instance = this;
}

MQTTManager::~MQTTManager() {
    // HomeAssistant will be cleaned up automatically by unique_ptr
    // Reset static instance if it points to this instance
    if (_instance == this) {
        _instance = nullptr;
    }
}

void MQTTManager::begin() {
    Serial.println("Setting up MQTT...");
    
    // Set server and callback
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setCallback(mqttCallback);
    _mqttClient.setBufferSize(1024);  // Increased buffer size for discovery messages
    
    // Try to connect
    reconnect();
    
    // Initialize Home Assistant integration using unique_ptr
    _homeAssistant = std::unique_ptr<HomeAssistant>(new HomeAssistant(_mqttClient, "esp32_thermostat"));
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
    
    // Handle temperature setpoint
    if (strcmp(topic, "esp32_thermostat/temperature/set") == 0) {
        float setpoint = atof(message);
        Serial.print("Setting temperature setpoint to: ");
        Serial.println(setpoint);
        
        // Update PID controller setpoint
        extern void setTemperatureSetpoint(float);
        setTemperatureSetpoint(setpoint);
        
        // Publish the new setpoint back to MQTT
        char setpointStr[8];
        dtostrf(setpoint, 1, 1, setpointStr);
        _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
    }
    
    // Handle system restart command
    if (strcmp(topic, "esp32_thermostat/restart") == 0) {
        Serial.println("Restart command received via MQTT");
        _mqttClient.publish("esp32_thermostat/status", "Restarting...", true);
        delay(500);
        ESP.restart();
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
            _mqttClient.subscribe("esp32_thermostat/temperature/set");
            _mqttClient.subscribe("esp32_thermostat/restart");
            
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