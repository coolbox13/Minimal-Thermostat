#include "mqtt_manager.h"
#include "knx_manager.h"
#include "config.h"
#include "config_manager.h"
#include "serial_monitor.h"
#include "serial_redirect.h"
#include <ArduinoJson.h>
#include <WiFi.h>

// Redirect Serial to CapturedSerial for web monitor
#define Serial CapturedSerial

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
    _mqttClient.setBufferSize(1536);  // Increased buffer size for discovery messages (climate payload is ~992 bytes)
    
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

    // Publish JSON aggregate if enabled
    ConfigManager* configManager = ConfigManager::getInstance();
    if (configManager && configManager->getMqttJsonAggregateEnabled()) {
        // Get current PID parameters
        float kp = configManager->getPidKp();
        float ki = configManager->getPidKi();
        float kd = configManager->getPidKd();
        
        // Get WiFi RSSI and uptime
        int wifiRSSI = WiFi.RSSI();
        unsigned long uptime = millis() / 1000; // Convert to seconds
        
        publishJsonAggregate(temperature, humidity, pressure, kp, ki, kd, wifiRSSI, uptime);
    }
}

void MQTTManager::updatePIDParameters(float kp, float ki, float kd) {
    if (!_mqttClient.connected()) return;

    // Update Home Assistant with PID parameters
    if (_homeAssistant) {
        _homeAssistant->updatePIDParameters(kp, ki, kd);
    }
}

void MQTTManager::updateDiagnostics(int wifiRSSI, unsigned long uptime) {
    if (!_mqttClient.connected()) return;

    // Update Home Assistant with diagnostic data
    if (_homeAssistant) {
        _homeAssistant->updateDiagnostics(wifiRSSI, uptime);
    }
}

void MQTTManager::publishJsonAggregate(float temperature, float humidity, float pressure,
                                        float kp, float ki, float kd, int wifiRSSI, unsigned long uptime) {
    if (!_mqttClient.connected()) return;

    ConfigManager* configManager = ConfigManager::getInstance();
    if (!configManager) return;

    // Create JSON document (512 bytes should be enough for all data)
    StaticJsonDocument<512> doc;

    // Sensor data
    doc["temperature"] = roundf(temperature * 100) / 100.0f; // Round to 2 decimals
    doc["humidity"] = roundf(humidity * 100) / 100.0f;
    doc["pressure"] = roundf(pressure * 100) / 100.0f;

    // Valve data
    doc["valve_position"] = _valvePosition;
    doc["action"] = (_valvePosition > 0) ? "heating" : "idle";
    doc["heating_state"] = (_valvePosition > 0) ? "ON" : "OFF";

    // PID parameters
    doc["pid"]["kp"] = roundf(kp * 100) / 100.0f; // Round to 2 decimals
    doc["pid"]["ki"] = roundf(ki * 1000) / 1000.0f; // Round to 3 decimals
    doc["pid"]["kd"] = roundf(kd * 1000) / 1000.0f; // Round to 3 decimals
    doc["pid"]["setpoint"] = roundf(configManager->getSetpoint() * 10) / 10.0f; // Round to 1 decimal

    // Control state
    doc["mode"] = configManager->getThermostatEnabled() ? "heat" : "off";
    doc["preset"] = configManager->getCurrentPreset();

    // Diagnostics
    doc["wifi"]["rssi"] = wifiRSSI;
    doc["uptime"] = uptime;
    doc["status"] = "online";

    // Serialize JSON to string
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Publish to 'telegraph' topic
    bool published = _mqttClient.publish("telegraph", jsonPayload.c_str());
    if (!published) {
        Serial.println("Failed to publish JSON aggregate to 'telegraph' topic");
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

            // HA FIX #2: Determine action based on mode AND valve position
            // When mode is "off", action should be "off", not "idle"
            ConfigManager* configManager = ConfigManager::getInstance();
            const char* action;
            if (!configManager->getThermostatEnabled()) {
                action = "off";
            } else if (_valvePosition > 0) {
                action = "heating";
            } else {
                action = "idle";
            }
            _mqttClient.publish("esp32_thermostat/action", action, true);
            Serial.print("Action: ");
            Serial.println(action);

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

    // Handle preset mode
    if (strcmp(topic, "esp32_thermostat/preset/set") == 0) {
        String preset = String(message);
        Serial.print("Setting preset mode to: ");
        Serial.println(preset);

        // Get the temperature for this preset
        extern ConfigManager* configManager;
        if (configManager) {
            float presetTemp = configManager->getPresetTemperature(preset);
            configManager->setCurrentPreset(preset);

            // Update the setpoint to match the preset temperature
            extern void setTemperatureSetpoint(float);
            setTemperatureSetpoint(presetTemp);

            // Publish the preset state back to MQTT
            if (_homeAssistant) {
                _homeAssistant->updatePresetMode(preset.c_str());
            }

            // Also publish the new setpoint
            char setpointStr[8];
            dtostrf(presetTemp, 1, 1, setpointStr);
            _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);

            Serial.print("Preset applied with temperature: ");
            Serial.println(presetTemp);
        }
    }

    // Handle mode command (heat/off)
    if (strcmp(topic, "esp32_thermostat/mode/set") == 0) {
        String mode = String(message);
        Serial.print("Setting mode to: ");
        Serial.println(mode);

        extern ConfigManager* configManager;
        if (configManager) {
            bool enabled = (mode == "heat");
            configManager->setThermostatEnabled(enabled);

            Serial.print("Thermostat ");
            Serial.println(enabled ? "enabled (heat mode)" : "disabled (off mode)");

            // If disabling, set valve to 0
            if (!enabled && _knxManager) {
                _knxManager->setValvePosition(0);
                Serial.println("Valve set to 0% (off mode)");
            }

            // Publish mode state confirmation
            if (_homeAssistant) {
                _homeAssistant->updateMode(mode.c_str());
            }
        }
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
        
        // Get MQTT credentials from ConfigManager
        ConfigManager* configManager = ConfigManager::getInstance();
        String mqttUser = configManager->getMqttUsername();
        String mqttPass = configManager->getMqttPassword();
        
        // Use credentials if provided, otherwise use empty strings (no auth)
        const char* username = (mqttUser.length() > 0) ? mqttUser.c_str() : nullptr;
        const char* password = (mqttPass.length() > 0) ? mqttPass.c_str() : nullptr;
        
        if (_mqttClient.connect(clientId.c_str(), username, password)) {
            Serial.println("MQTT connected");
            
            // Subscribe to topics
            _mqttClient.subscribe(MQTT_TOPIC_VALVE_COMMAND);
            _mqttClient.subscribe("esp32_thermostat/valve/set");
            _mqttClient.subscribe("esp32_thermostat/temperature/set");
            _mqttClient.subscribe("esp32_thermostat/restart");
            
            // Update availability
            if (_homeAssistant) {
                _homeAssistant->updateAvailability(true);

                // HA MQTT FIX: Publish initial diagnostic state immediately after connection
                // This ensures sensors don't show "Unknown" in Home Assistant
                int rssi = WiFi.RSSI();
                unsigned long uptime = millis() / 1000;
                _homeAssistant->updateDiagnostics(rssi, uptime);

                // HA FIX #5: Sync climate state after reconnection
                _homeAssistant->syncClimateState();
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

// HA FIX #5: Sync climate state to Home Assistant
void MQTTManager::syncClimateState() {
    if (!_mqttClient.connected()) return;

    if (_homeAssistant) {
        _homeAssistant->syncClimateState();
    }
}