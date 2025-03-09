#include "mqtt_interface.h"
#include "thermostat_state.h"
#include "protocol_manager.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <memory>

// Define make_unique for C++11 compatibility
#if __cplusplus < 201402L
namespace std {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

// Static instance pointer for callbacks
static MQTTInterface* instance = nullptr;

// Implementation class
class MQTTInterface::Impl {
public:
    // MQTT client
    WiFiClient espClient;
    PubSubClient client;
    
    // Connection settings
    char server[64] = {0};
    uint16_t port = 1883;
    char username[32] = {0};
    char password[32] = {0};
    char clientId[32] = "esp32_thermostat";
    char topicPrefix[64] = "esp32/thermostat/";
    
    // State
    bool connected = false;
    ThermostatStatus lastError = ThermostatStatus::OK;
    char lastErrorMessage[128] = {0};
    unsigned long lastReconnectAttempt = 0;
    ThermostatState* thermostatState = nullptr;
    ProtocolManager* protocolManager = nullptr;
    
    // Topics
    char temperatureTopic[128] = {0};
    char humidityTopic[128] = {0};
    char pressureTopic[128] = {0};
    char setpointTopic[128] = {0};
    char modeTopic[128] = {0};
    char valveTopic[128] = {0};
    char heatingTopic[128] = {0};
    char statusTopic[128] = {0};
    
    Impl() : client(espClient) {
        client.setBufferSize(512); // Increase buffer size for larger messages
        // Initialize with default values
        enabled = false;
        connected = false;
        lastError = ThermostatStatus::OK;
        
        // Set default MQTT topics
        strcpy(topicPrefix, "esp32/thermostat/");
        strcpy(temperatureTopic, "temperature");
        strcpy(humidityTopic, "humidity");
        strcpy(setpointTopic, "setpoint");
        strcpy(modeTopic, "mode");
        strcpy(valveTopic, "valve");
        strcpy(heatingTopic, "heating");
        strcpy(statusTopic, "status");
        
        // Set callback
        client.setCallback([](char* topic, byte* payload, unsigned int length) {
            if (instance) {
                instance->handleMessage(topic, payload, length);
            }
        });
    }

    // Configuration
    bool enabled;
    
    // Message handler
    void handleMessage(char* topic, byte* payload, unsigned int length);
};

// Constructor
MQTTInterface::MQTTInterface() : pimpl(std::make_unique<Impl>()) {
    // Set static instance pointer for callbacks
    instance = this;
}

MQTTInterface::~MQTTInterface() {
    // Disconnect if connected
    if (pimpl->connected) {
        pimpl->client.disconnect();
    }
    // Clean up static instance pointer
    if (instance == this) {
        instance = nullptr;
    }
}

// Connection management
bool MQTTInterface::begin() {
    if (!pimpl->enabled) {
        Serial.println("MQTT disabled, not connecting");
        return false;
    }
    
    Serial.print("Connecting to MQTT broker at ");
    Serial.print(pimpl->server);
    Serial.print(":");
    Serial.println(pimpl->port);
    
    pimpl->client.setServer(pimpl->server, pimpl->port);
    
    return reconnect();
}

void MQTTInterface::loop() {
    if (!pimpl->enabled) {
        return;
    }
    
    // Check if connected
    if (!pimpl->connected) {
        // Try to reconnect every 5 seconds
        static unsigned long lastReconnectAttempt = 0;
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
        return;
    }
    
    // Process MQTT messages
    pimpl->client.loop();
}

bool MQTTInterface::isConnected() const {
    return pimpl->connected;
}

void MQTTInterface::disconnect() {
    pimpl->client.disconnect();
    pimpl->connected = false;
}

bool MQTTInterface::reconnect() {
    if (!pimpl->enabled) {
        return false;
    }
    
    // Try to connect
    bool result = false;
    if (strlen(pimpl->username) > 0) {
        result = pimpl->client.connect(pimpl->clientId, pimpl->username, pimpl->password);
    } else {
        result = pimpl->client.connect(pimpl->clientId);
    }
    
    if (result) {
        Serial.println("Connected to MQTT broker");
        pimpl->connected = true;
        
        // Subscribe to topics
        String setpointTopicFull = String(pimpl->topicPrefix) + pimpl->setpointTopic + "/set";
        String modeTopicFull = String(pimpl->topicPrefix) + pimpl->modeTopic + "/set";
        
        pimpl->client.subscribe(setpointTopicFull.c_str());
        pimpl->client.subscribe(modeTopicFull.c_str());
        
        // Publish initial status
        publish(pimpl->statusTopic, "online", true);
        
        return true;
    } else {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.println(pimpl->client.state());
        pimpl->connected = false;
        pimpl->lastError = ThermostatStatus::ERROR_COMMUNICATION;
        return false;
    }
}

// Configuration
bool MQTTInterface::configure(const JsonDocument& config) {
    if (!config.containsKey("server") || !config.containsKey("port")) {
        Serial.println("Invalid MQTT configuration");
        pimpl->lastError = ThermostatStatus::ERROR_CONFIGURATION;
        return false;
    }
    
    // Copy configuration
    strlcpy(pimpl->server, config["server"] | "localhost", sizeof(pimpl->server));
    pimpl->port = config["port"] | 1883;
    
    if (config.containsKey("username")) {
        strlcpy(pimpl->username, config["username"], sizeof(pimpl->username));
    }
    
    if (config.containsKey("password")) {
        strlcpy(pimpl->password, config["password"], sizeof(pimpl->password));
    }
    
    if (config.containsKey("clientId")) {
        strlcpy(pimpl->clientId, config["clientId"], sizeof(pimpl->clientId));
    }
    
    if (config.containsKey("topicPrefix")) {
        strlcpy(pimpl->topicPrefix, config["topicPrefix"], sizeof(pimpl->topicPrefix));
    }
    
    // Enable MQTT
    pimpl->enabled = config["enabled"] | false;
    
    return true;
}

void MQTTInterface::getConfig(JsonDocument& config) const {
    config["enabled"] = pimpl->enabled;
    config["server"] = pimpl->server;
    config["port"] = pimpl->port;
    config["username"] = pimpl->username;
    config["password"] = pimpl->password;
    config["clientId"] = pimpl->clientId;
    config["topicPrefix"] = pimpl->topicPrefix;
}

// Data transmission
bool MQTTInterface::sendTemperature(float temperature) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.2f", temperature);
    return publish(pimpl->temperatureTopic, buffer);
}

bool MQTTInterface::sendHumidity(float humidity) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.2f", humidity);
    return publish(pimpl->humidityTopic, buffer);
}

bool MQTTInterface::sendPressure(float value) {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(getFullTopic("pressure").c_str(), payload);
}

bool MQTTInterface::sendSetpoint(float setpoint) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.2f", setpoint);
    return publish(pimpl->setpointTopic, buffer);
}

bool MQTTInterface::sendValvePosition(float position) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.2f", position);
    return publish(pimpl->valveTopic, buffer);
}

bool MQTTInterface::sendMode(ThermostatMode mode) {
    const char* modeStr = getThermostatModeName(mode);
    return publish(pimpl->modeTopic, modeStr);
}

bool MQTTInterface::sendHeatingState(bool isHeating) {
    return publish(pimpl->heatingTopic, isHeating ? "ON" : "OFF");
}

// Error handling
ThermostatStatus MQTTInterface::getLastError() const {
    return pimpl->lastError;
}

const char* MQTTInterface::getLastErrorMessage() const {
    return pimpl->lastErrorMessage;
}

void MQTTInterface::clearError() {
    pimpl->lastError = ThermostatStatus::OK;
    pimpl->lastErrorMessage[0] = '\0';
}

// Protocol registration
void MQTTInterface::registerCallbacks(ThermostatState* state, ProtocolManager* manager) {
    pimpl->thermostatState = state;
    pimpl->protocolManager = manager;
}

void MQTTInterface::unregisterCallbacks() {
    pimpl->thermostatState = nullptr;
    pimpl->protocolManager = nullptr;
}

// MQTT specific configuration
void MQTTInterface::setServer(const char* server) {
    strlcpy(pimpl->server, server, sizeof(pimpl->server));
}

void MQTTInterface::setPort(uint16_t port) {
    pimpl->port = port;
}

void MQTTInterface::setUsername(const char* username) {
    strlcpy(pimpl->username, username, sizeof(pimpl->username));
}

void MQTTInterface::setPassword(const char* password) {
    strlcpy(pimpl->password, password, sizeof(pimpl->password));
}

void MQTTInterface::setClientId(const char* clientId) {
    strlcpy(pimpl->clientId, clientId, sizeof(pimpl->clientId));
}

void MQTTInterface::setTopicPrefix(const char* prefix) {
    strlcpy(pimpl->topicPrefix, prefix, sizeof(pimpl->topicPrefix));
}

// Internal helpers
bool MQTTInterface::validateConnection() const {
    if (strlen(pimpl->server) == 0) {
        return false;
    }
    if (pimpl->port == 0) {
        return false;
    }
    if (strlen(pimpl->clientId) == 0) {
        return false;
    }
    return true;
}

bool MQTTInterface::validateTopics() const {
    return strlen(pimpl->topicPrefix) > 0;
}

bool MQTTInterface::publish(const char* topic, const char* payload, bool retain) {
    if (!pimpl->enabled || !pimpl->connected) {
        Serial.println("MQTT not connected, can't publish");
        pimpl->lastError = ThermostatStatus::ERROR_COMMUNICATION;
        return false;
    }
    
    String fullTopic = String(pimpl->topicPrefix) + topic;
    bool result = pimpl->client.publish(fullTopic.c_str(), payload, retain);
    if (!result) {
        Serial.print("Failed to publish to ");
        Serial.println(fullTopic);
        pimpl->lastError = ThermostatStatus::ERROR_COMMUNICATION;
    }
    
    return result;
}

void MQTTInterface::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (!pimpl->protocolManager) return;
    
    String topicStr(topic);
    String setpointTopic = getFullTopic("setpoint");
    String modeTopic = getFullTopic("mode");
    
    if (topicStr == setpointTopic) {
        float setpoint = atof(String((char*)payload, length).c_str());
        pimpl->protocolManager->handleIncomingCommand(
            getCommandSource(),
            CommandType::CMD_SETPOINT,
            setpoint
        );
    } else if (topicStr == modeTopic) {
        int mode = atoi(String((char*)payload, length).c_str());
        pimpl->protocolManager->handleIncomingCommand(
            getCommandSource(),
            CommandType::CMD_MODE,
            static_cast<float>(mode)
        );
    }
}

String MQTTInterface::getFullTopic(const char* suffix) const {
    String fullTopic = pimpl->topicPrefix;
    fullTopic += suffix;
    return fullTopic;
}

void MQTTInterface::setEnabled(bool enabled) {
    pimpl->enabled = enabled;
    
    if (enabled && !pimpl->connected) {
        reconnect();
    } else if (!enabled && pimpl->connected) {
        pimpl->client.disconnect();
        pimpl->connected = false;
    }
}

bool MQTTInterface::isEnabled() const {
    return pimpl->enabled;
}