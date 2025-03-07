#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include "thermostat_state.h"

class MQTTInterface {
public:
  // Constructor
  MQTTInterface();
  
  // Initialize MQTT connection
  bool begin(ThermostatState* state, 
             const char* server, 
             int port = 1883, 
             const char* user = nullptr, 
             const char* password = nullptr,
             const char* clientId = "KNXThermostat");
  
  // Set MQTT server
  void setServer(const char* server, int port = 1883);
  
  // Set MQTT credentials
  void setCredentials(const char* user, const char* password);
  
  // Set client ID
  void setClientId(const char* clientId);
  
  // Process MQTT communication (call in loop)
  void loop();
  
  // Publish values
  void publishTemperature(float temperature);
  void publishHumidity(float humidity);
  void publishPressure(float pressure);
  void publishSetpoint(float setpoint);
  void publishValvePosition(float position);
  void publishMode(ThermostatMode mode);
  
  // Connect/disconnect
  bool connect();
  void disconnect();
  
  // Check connection status
  bool isConnected() const;

private:
  // MQTT client
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  
  // Connection parameters
  char server[40];
  int port;
  char user[24];
  char password[24];
  char clientId[24];
  
  // Reference to thermostat state
  ThermostatState* thermostatState;
  
  // Topic names
  static constexpr const char* TOPIC_TEMPERATURE = "esp_thermostat/temperature";
  static constexpr const char* TOPIC_HUMIDITY = "esp_thermostat/humidity";
  static constexpr const char* TOPIC_PRESSURE = "esp_thermostat/pressure";
  static constexpr const char* TOPIC_SETPOINT = "esp_thermostat/setpoint";
  static constexpr const char* TOPIC_VALVE_POSITION = "esp_thermostat/valvePosition";
  static constexpr const char* TOPIC_MODE = "esp_thermostat/mode";
  static constexpr const char* TOPIC_SETTINGS_BASE = "esp_thermostat/settings/";
  
  // Last connection attempt
  unsigned long lastConnectAttempt;
  static constexpr unsigned long CONNECT_RETRY_INTERVAL = 30000;  // 30 seconds
  
  // MQTT callback handler
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
  void handleCallback(char* topic, byte* payload, unsigned int length);
  
  // Register callbacks
  void registerCallbacks();
};

#endif // MQTT_INTERFACE_H