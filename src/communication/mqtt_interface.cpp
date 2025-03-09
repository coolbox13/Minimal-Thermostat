#include "mqtt_interface.h"
#include "thermostat_state.h"
#include "protocol_manager.h"
#include <ArduinoJson.h>

// Implementation class
class MQTTInterface::Impl {
public:
    // MQTT client
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    // Connection settings
    char server[64] = {0};
    uint16_t port = 1883;
    char username[32] = {0};
    char password[32] = {0};
    char clientId[32] = {0};
    char topicPrefix[32] = {0};
    
    // State
    bool connected = false;
    ThermostatStatus lastError = ThermostatStatus::OK;
    const char* lastErrorMsg = nullptr;
    unsigned long lastReconnectAttempt = 0;
    ThermostatState* thermostatState = nullptr;
    ProtocolManager* protocolManager = nullptr;
    
    // Topics
    static constexpr const char* TOPIC_TEMPERATURE = "/temperature";
    static constexpr const char* TOPIC_HUMIDITY = "/humidity";
    static constexpr const char* TOPIC_PRESSURE = "/pressure";
    static constexpr const char* TOPIC_SETPOINT = "/setpoint";
    static constexpr const char* TOPIC_VALVE = "/valve";
    static constexpr const char* TOPIC_MODE = "/mode";
    static constexpr const char* TOPIC_HEATING = "/heating";

    Impl() : mqttClient(wifiClient) {
        mqttClient.setBufferSize(512); // Increase buffer size for larger messages
    }

    // Helper to store instance pointer for callbacks
    static MQTTInterface* instance;
};

// Static instance pointer initialization
MQTTInterface* MQTTInterface::Impl::instance = nullptr;

// Constructor
MQTTInterface::MQTTInterface() : pimpl(std::make_unique<Impl>()) {
    Impl::instance = this;
    pimpl->mqttClient.setCallback(mqttCallback);
}

// Connection management
bool MQTTInterface::begin() {
    if (!validateConfig()) {
        return false;
    }
    
    pimpl->mqttClient.setServer(pimpl->server, pimpl->port);
    return reconnect();
}

void MQTTInterface::loop() {
    if (!pimpl->mqttClient.connected()) {
        unsigned long now = millis();
        if (now - pimpl->lastReconnectAttempt > 5000) {
            pimpl->lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        pimpl->mqttClient.loop();
    }
}

bool MQTTInterface::isConnected() const {
    return pimpl->mqttClient.connected();
}

void MQTTInterface::disconnect() {
    pimpl->mqttClient.disconnect();
    pimpl->connected = false;
}

bool MQTTInterface::reconnect() {
    if (pimpl->mqttClient.connected()) {
        return true;
    }

    if (strlen(pimpl->username) > 0) {
        pimpl->connected = pimpl->mqttClient.connect(pimpl->clientId, pimpl->username, pimpl->password);
    } else {
        pimpl->connected = pimpl->mqttClient.connect(pimpl->clientId);
    }

    if (pimpl->connected) {
        setupSubscriptions();
        pimpl->lastError = ThermostatStatus::OK;
        pimpl->lastErrorMsg = nullptr;
    } else {
        pimpl->lastError = ThermostatStatus::MQTT_CONNECTION_ERROR;
        pimpl->lastErrorMsg = "Failed to connect to MQTT broker";
    }

    return pimpl->connected;
}

// Configuration
bool MQTTInterface::configure(const JsonDocument& config) {
    if (!config.containsKey("server") || !config.containsKey("port")) {
        pimpl->lastError = ThermostatStatus::INVALID_CONFIG;
        pimpl->lastErrorMsg = "Missing required MQTT configuration";
        return false;
    }

    strlcpy(pimpl->server, config["server"].as<const char*>(), sizeof(pimpl->server));
    pimpl->port = config["port"].as<uint16_t>();

    if (config.containsKey("username")) {
        strlcpy(pimpl->username, config["username"].as<const char*>(), sizeof(pimpl->username));
    }
    if (config.containsKey("password")) {
        strlcpy(pimpl->password, config["password"].as<const char*>(), sizeof(pimpl->password));
    }
    if (config.containsKey("clientId")) {
        strlcpy(pimpl->clientId, config["clientId"].as<const char*>(), sizeof(pimpl->clientId));
    }
    if (config.containsKey("topicPrefix")) {
        strlcpy(pimpl->topicPrefix, config["topicPrefix"].as<const char*>(), sizeof(pimpl->topicPrefix));
    }

    return validateConfig();
}

bool MQTTInterface::validateConfig() const {
    return validateConnection() && validateTopics();
}

void MQTTInterface::getConfig(JsonDocument& config) const {
    config["server"] = pimpl->server;
    config["port"] = pimpl->port;
    config["username"] = pimpl->username;
    config["password"] = pimpl->password;
    config["clientId"] = pimpl->clientId;
    config["topicPrefix"] = pimpl->topicPrefix;
}

// Data transmission
bool MQTTInterface::sendTemperature(float value) {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(getFullTopic(Impl::TOPIC_TEMPERATURE).c_str(), payload);
}

bool MQTTInterface::sendHumidity(float value) {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(getFullTopic(Impl::TOPIC_HUMIDITY).c_str(), payload);
}

bool MQTTInterface::sendPressure(float value) {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(getFullTopic(Impl::TOPIC_PRESSURE).c_str(), payload);
}

bool MQTTInterface::sendSetpoint(float value) {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(getFullTopic(Impl::TOPIC_SETPOINT).c_str(), payload);
}

bool MQTTInterface::sendValvePosition(float value) {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(getFullTopic(Impl::TOPIC_VALVE).c_str(), payload);
}

bool MQTTInterface::sendMode(ThermostatMode mode) {
    return publish(getFullTopic(Impl::TOPIC_MODE).c_str(), String(static_cast<int>(mode)).c_str());
}

bool MQTTInterface::sendHeatingState(bool isHeating) {
    return publish(getFullTopic(Impl::TOPIC_HEATING).c_str(), isHeating ? "1" : "0");
}

// Error handling
ThermostatStatus MQTTInterface::getLastError() const {
    return pimpl->lastError;
}

const char* MQTTInterface::getLastErrorMessage() const {
    return pimpl->lastErrorMsg;
}

void MQTTInterface::clearError() {
    pimpl->lastError = ThermostatStatus::OK;
    pimpl->lastErrorMsg = nullptr;
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
void MQTTInterface::setServer(const char* server, uint16_t port) {
    strlcpy(pimpl->server, server, sizeof(pimpl->server));
    pimpl->port = port;
}

void MQTTInterface::setCredentials(const char* username, const char* password) {
    strlcpy(pimpl->username, username, sizeof(pimpl->username));
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
    if (!pimpl->mqttClient.connected()) {
        pimpl->lastError = ThermostatStatus::MQTT_CONNECTION_ERROR;
        pimpl->lastErrorMsg = "Not connected to MQTT broker";
        return false;
    }

    if (!pimpl->mqttClient.publish(topic, payload, retain)) {
        pimpl->lastError = ThermostatStatus::MQTT_PUBLISH_ERROR;
        pimpl->lastErrorMsg = "Failed to publish MQTT message";
        return false;
    }

    return true;
}

void MQTTInterface::setupSubscriptions() {
    // Subscribe to setpoint and mode topics for remote control
    pimpl->mqttClient.subscribe(getFullTopic(Impl::TOPIC_SETPOINT).c_str());
    pimpl->mqttClient.subscribe(getFullTopic(Impl::TOPIC_MODE).c_str());
}

void MQTTInterface::cleanupSubscriptions() {
    pimpl->mqttClient.unsubscribe(getFullTopic(Impl::TOPIC_SETPOINT).c_str());
    pimpl->mqttClient.unsubscribe(getFullTopic(Impl::TOPIC_MODE).c_str());
}

void MQTTInterface::handleMessage(char* topic, uint8_t* payload, unsigned int length) {
    if (!pimpl->protocolManager) {
        return;
    }

    // Null terminate the payload
    char* payloadStr = new char[length + 1];
    memcpy(payloadStr, payload, length);
    payloadStr[length] = '\0';

    String topicStr(topic);
    String setpointTopic = getFullTopic(Impl::TOPIC_SETPOINT);
    String modeTopic = getFullTopic(Impl::TOPIC_MODE);

    if (topicStr == setpointTopic) {
        float setpoint = atof(payloadStr);
        pimpl->protocolManager->handleIncomingCommand(CommandType::CMD_SET_TEMPERATURE, setpoint, getCommandSource());
    } else if (topicStr == modeTopic) {
        int mode = atoi(payloadStr);
        pimpl->protocolManager->handleIncomingCommand(CommandType::CMD_SET_MODE, mode, getCommandSource());
    }

    delete[] payloadStr;
}

void MQTTInterface::mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    // Use the static instance pointer to route the callback
    if (Impl::instance) {
        Impl::instance->handleMessage(topic, payload, length);
    }
}

String MQTTInterface::getFullTopic(const char* suffix) const {
    String fullTopic = pimpl->topicPrefix;
    fullTopic += suffix;
    return fullTopic;
}