#ifndef MOCK_PUBSUBCLIENT_H
#define MOCK_PUBSUBCLIENT_H

#include "Arduino.h"
#include "WiFi.h"
#include <functional>
#include <string>
#include <map>

// MQTT connection states
#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

// Callback function type
typedef std::function<void(char*, uint8_t*, unsigned int)> MQTT_CALLBACK_SIGNATURE;

/**
 * Mock implementation of PubSubClient (MQTT) for testing
 * Simulates MQTT broker connectivity and message publishing/subscription
 */
class PubSubClient {
private:
    WiFiClient* _client;
    const char* _server;
    uint16_t _port;
    int _state;
    MQTT_CALLBACK_SIGNATURE _callback;

    // Test storage
    std::map<std::string, std::string> _publishedMessages;
    std::map<std::string, bool> _subscriptions;
    bool _shouldFailConnect;

public:
    PubSubClient()
        : _client(nullptr)
        , _server("")
        , _port(1883)
        , _state(MQTT_DISCONNECTED)
        , _shouldFailConnect(false) {}

    PubSubClient(WiFiClient& client)
        : _client(&client)
        , _server("")
        , _port(1883)
        , _state(MQTT_DISCONNECTED)
        , _shouldFailConnect(false) {}

    /**
     * Set the MQTT server details
     */
    PubSubClient& setServer(const char* domain, uint16_t port) {
        _server = domain;
        _port = port;
        return *this;
    }

    /**
     * Set the callback function for incoming messages
     */
    PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE callback) {
        _callback = callback;
        return *this;
    }

    /**
     * Set the WiFi client
     */
    PubSubClient& setClient(WiFiClient& client) {
        _client = &client;
        return *this;
    }

    /**
     * Connect to MQTT broker
     */
    bool connect(const char* id) {
        if (_shouldFailConnect) {
            _state = MQTT_CONNECT_FAILED;
            return false;
        }
        _state = MQTT_CONNECTED;
        return true;
    }

    /**
     * Connect to MQTT broker with credentials
     */
    bool connect(const char* id, const char* user, const char* pass) {
        if (_shouldFailConnect) {
            _state = MQTT_CONNECT_FAILED;
            return false;
        }
        _state = MQTT_CONNECTED;
        return true;
    }

    /**
     * Connect with will message
     */
    bool connect(const char* id, const char* willTopic, uint8_t willQos,
                 bool willRetain, const char* willMessage) {
        if (_shouldFailConnect) {
            _state = MQTT_CONNECT_FAILED;
            return false;
        }
        _state = MQTT_CONNECTED;
        return true;
    }

    /**
     * Disconnect from MQTT broker
     */
    void disconnect() {
        _state = MQTT_DISCONNECTED;
    }

    /**
     * Publish a message to a topic
     */
    bool publish(const char* topic, const char* payload) {
        if (_state != MQTT_CONNECTED) {
            return false;
        }
        _publishedMessages[topic] = payload;
        return true;
    }

    /**
     * Publish a message with QoS and retain
     */
    bool publish(const char* topic, const uint8_t* payload,
                 unsigned int length, bool retained = false) {
        if (_state != MQTT_CONNECTED) {
            return false;
        }
        std::string payloadStr((const char*)payload, length);
        _publishedMessages[topic] = payloadStr;
        return true;
    }

    /**
     * Subscribe to a topic
     */
    bool subscribe(const char* topic) {
        if (_state != MQTT_CONNECTED) {
            return false;
        }
        _subscriptions[topic] = true;
        return true;
    }

    /**
     * Subscribe to a topic with QoS
     */
    bool subscribe(const char* topic, uint8_t qos) {
        return subscribe(topic);
    }

    /**
     * Unsubscribe from a topic
     */
    bool unsubscribe(const char* topic) {
        _subscriptions.erase(topic);
        return true;
    }

    /**
     * Process incoming messages (call in loop)
     */
    bool loop() {
        return _state == MQTT_CONNECTED;
    }

    /**
     * Check if connected to broker
     */
    bool connected() {
        return _state == MQTT_CONNECTED;
    }

    /**
     * Get connection state
     */
    int state() {
        return _state;
    }

    // ===== Test Control Methods =====

    /**
     * Simulate receiving a message on a subscribed topic
     */
    void simulateMessage(const char* topic, const char* payload) {
        if (_callback && _subscriptions.count(topic)) {
            _callback((char*)topic, (uint8_t*)payload, strlen(payload));
        }
    }

    /**
     * Get published message for a topic
     */
    std::string getPublishedMessage(const char* topic) {
        if (_publishedMessages.count(topic)) {
            return _publishedMessages[topic];
        }
        return "";
    }

    /**
     * Check if a topic has been published to
     */
    bool wasPublished(const char* topic) {
        return _publishedMessages.count(topic) > 0;
    }

    /**
     * Check if subscribed to a topic
     */
    bool isSubscribed(const char* topic) {
        return _subscriptions.count(topic) > 0;
    }

    /**
     * Clear all published messages
     */
    void clearPublishedMessages() {
        _publishedMessages.clear();
    }

    /**
     * Set whether connect should fail
     */
    void setMockShouldFailConnect(bool shouldFail) {
        _shouldFailConnect = shouldFail;
        if (shouldFail) {
            _state = MQTT_DISCONNECTED;
        }
    }

    /**
     * Reset mock to default state
     */
    void resetMock() {
        _state = MQTT_DISCONNECTED;
        _publishedMessages.clear();
        _subscriptions.clear();
        _shouldFailConnect = false;
    }
};

#endif // MOCK_PUBSUBCLIENT_H
