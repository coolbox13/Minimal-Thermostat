#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "interfaces/protocol_interface.h"
#include "protocol_manager.h"

// Forward declarations
class ProtocolManager;

class MQTTInterface : public ProtocolInterface {
public:
    // Constructor
    MQTTInterface();

    // ProtocolInterface implementation
    bool begin() override;
    void loop() override;
    bool isConnected() const override;
    bool sendTemperature(float value) override;
    bool sendHumidity(float value) override;
    bool sendPressure(float value) override;
    bool sendSetpoint(float value) override;
    bool sendValvePosition(float value) override;
    bool sendMode(ThermostatMode mode) override;
    bool sendHeatingState(bool isHeating) override;
    ThermostatStatus getLastError() const override;

    // MQTT specific configuration
    void setServer(const char* server, uint16_t port = 1883);
    void setCredentials(const char* username, const char* password);
    void setClientId(const char* clientId);
    void setTopicPrefix(const char* prefix);
    void registerProtocolManager(ProtocolManager* manager) { protocolManager = manager; }

private:
    // MQTT client
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    // Connection settings
    char server[64];
    uint16_t port;
    char username[32];
    char password[32];
    char clientId[32];
    char topicPrefix[32];
    
    // State
    bool connected;
    ThermostatStatus lastError;
    unsigned long lastReconnectAttempt;
    ProtocolManager* protocolManager;
    
    // Topics
    static constexpr const char* TOPIC_TEMPERATURE = "/temperature";
    static constexpr const char* TOPIC_HUMIDITY = "/humidity";
    static constexpr const char* TOPIC_PRESSURE = "/pressure";
    static constexpr const char* TOPIC_SETPOINT = "/setpoint";
    static constexpr const char* TOPIC_VALVE = "/valve";
    static constexpr const char* TOPIC_MODE = "/mode";
    static constexpr const char* TOPIC_HEATING = "/heating";
    
    // Internal helpers
    bool connect();
    bool publish(const char* topic, const char* payload);
    void handleMessage(char* topic, uint8_t* payload, unsigned int length);
    static void mqttCallback(char* topic, uint8_t* payload, unsigned int length);
    
    // Topic management
    String getFullTopic(const char* suffix) const;
};

#endif // MQTT_INTERFACE_H