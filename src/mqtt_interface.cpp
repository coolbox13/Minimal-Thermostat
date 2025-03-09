#include "mqtt_interface.h"

MQTTInterface::MQTTInterface()
    : mqttClient(wifiClient)
    , port(1883)
    , connected(false)
    , lastError(ThermostatStatus::OK)
    , lastReconnectAttempt(0) {
    server[0] = '\0';
    username[0] = '\0';
    password[0] = '\0';
    clientId[0] = '\0';
    strcpy(topicPrefix, "thermostat");  // Default prefix
}

bool MQTTInterface::begin() {
    if (strlen(server) == 0 || strlen(clientId) == 0) {
        lastError = ThermostatStatus::CONFIGURATION_ERROR;
        return false;
    }

    mqttClient.setServer(server, port);
    mqttClient.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
        this->handleMessage(topic, payload, length);
    });

    return connect();
}

void MQTTInterface::loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {  // Try to reconnect every 5 seconds
            lastReconnectAttempt = now;
            connected = false;
            if (connect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
    }
}

bool MQTTInterface::connect() {
    if (strlen(username) > 0) {
        connected = mqttClient.connect(clientId, username, password);
    } else {
        connected = mqttClient.connect(clientId);
    }

    if (connected) {
        // Subscribe to control topics
        String setpointTopic = getFullTopic(TOPIC_SETPOINT);
        String modeTopic = getFullTopic(TOPIC_MODE);
        mqttClient.subscribe(setpointTopic.c_str());
        mqttClient.subscribe(modeTopic.c_str());
        lastError = ThermostatStatus::OK;
    } else {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
    }

    return connected;
}

bool MQTTInterface::isConnected() const {
    return connected && mqttClient.connected();
}

bool MQTTInterface::sendTemperature(float value) {
    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(TOPIC_TEMPERATURE, payload);
}

bool MQTTInterface::sendHumidity(float value) {
    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(TOPIC_HUMIDITY, payload);
}

bool MQTTInterface::sendPressure(float value) {
    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(TOPIC_PRESSURE, payload);
}

bool MQTTInterface::sendSetpoint(float value) {
    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(TOPIC_SETPOINT, payload);
}

bool MQTTInterface::sendValvePosition(float value) {
    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", value);
    return publish(TOPIC_VALVE, payload);
}

bool MQTTInterface::sendMode(ThermostatMode mode) {
    const char* modeStr;
    switch (mode) {
        case MODE_OFF: modeStr = "off"; break;
        case MODE_COMFORT: modeStr = "comfort"; break;
        case MODE_ECO: modeStr = "eco"; break;
        case MODE_AWAY: modeStr = "away"; break;
        case MODE_BOOST: modeStr = "boost"; break;
        case MODE_ANTIFREEZE: modeStr = "antifreeze"; break;
        default: modeStr = "unknown"; break;
    }
    return publish(TOPIC_MODE, modeStr);
}

bool MQTTInterface::sendHeatingState(bool isHeating) {
    return publish(TOPIC_HEATING, isHeating ? "ON" : "OFF");
}

ThermostatStatus MQTTInterface::getLastError() const {
    return lastError;
}

void MQTTInterface::setServer(const char* serverAddress, uint16_t serverPort) {
    strncpy(server, serverAddress, sizeof(server) - 1);
    server[sizeof(server) - 1] = '\0';
    port = serverPort;
}

void MQTTInterface::setCredentials(const char* user, const char* pass) {
    strncpy(username, user, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';
    strncpy(password, pass, sizeof(password) - 1);
    password[sizeof(password) - 1] = '\0';
}

void MQTTInterface::setClientId(const char* id) {
    strncpy(clientId, id, sizeof(clientId) - 1);
    clientId[sizeof(clientId) - 1] = '\0';
}

void MQTTInterface::setTopicPrefix(const char* prefix) {
    strncpy(topicPrefix, prefix, sizeof(topicPrefix) - 1);
    topicPrefix[sizeof(topicPrefix) - 1] = '\0';
}

bool MQTTInterface::publish(const char* topic, const char* payload) {
    if (!isConnected()) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }

    String fullTopic = getFullTopic(topic);
    bool success = mqttClient.publish(fullTopic.c_str(), payload);
    
    if (!success) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
    }
    
    return success;
}

void MQTTInterface::handleMessage(char* topic, uint8_t* payload, unsigned int length) {
    char message[32];
    if (length >= sizeof(message)) {
        length = sizeof(message) - 1;
    }
    memcpy(message, payload, length);
    message[length] = '\0';

    String topicStr = String(topic);
    String setpointTopic = getFullTopic(TOPIC_SETPOINT);
    String modeTopic = getFullTopic(TOPIC_MODE);

    if (topicStr == setpointTopic) {
        float setpoint = atof(message);
        // Update setpoint through protocol manager
        if (protocolManager) {
            protocolManager->handleIncomingCommand(SOURCE_MQTT, CMD_SET_TEMPERATURE, setpoint);
        }
    } else if (topicStr == modeTopic) {
        ThermostatMode mode;
        if (strcmp(message, "off") == 0) mode = MODE_OFF;
        else if (strcmp(message, "comfort") == 0) mode = MODE_COMFORT;
        else if (strcmp(message, "eco") == 0) mode = MODE_ECO;
        else if (strcmp(message, "away") == 0) mode = MODE_AWAY;
        else if (strcmp(message, "boost") == 0) mode = MODE_BOOST;
        else if (strcmp(message, "antifreeze") == 0) mode = MODE_ANTIFREEZE;
        else return;  // Invalid mode

        // Update mode through protocol manager
        if (protocolManager) {
            protocolManager->handleIncomingCommand(SOURCE_MQTT, CMD_SET_MODE, static_cast<float>(mode));
        }
    }
}

String MQTTInterface::getFullTopic(const char* suffix) const {
    String fullTopic = String(topicPrefix);
    fullTopic += suffix;
    return fullTopic;
} 