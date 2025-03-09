#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "thermostat_state.h"

// Forward declaration to resolve circular dependency
class ProtocolManager;

class MQTTInterface {
public:
  // Constructor
  MQTTInterface();
  
  // Initialize MQTT communication
  bool begin(ThermostatState* state, 
             const char* server, 
             int port, 
             const char* user, 
             const char* password,
             const char* clientId);
  
  // Register with protocol manager
  void registerProtocolManager(ProtocolManager* manager);
  
  // Process MQTT loop
  void loop();
  
  // Check connection status
  bool isConnected() const;
  
  // Reconnect to MQTT broker if connection lost
  bool reconnect();
  
  // Publish values to MQTT topics
  void publishTemperature(float temperature);
  void publishHumidity(float humidity);
  void publishPressure(float pressure);
  void publishSetpoint(float setpoint);
  void publishValvePosition(float position);
  void publishMode(ThermostatMode mode);
  void publishHeatingStatus(bool isHeating);
  
private:
  // MQTT client
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  
  // Connection parameters
  char server[40];
  int port;
  char username[24];
  char password[24];
  char clientId[24];
  bool connectionActive;
  
  // References to other components
  ThermostatState* thermostatState;
  ProtocolManager* protocolManager;
  
  // Last connection attempt time
  unsigned long lastConnectionAttempt;
  
  // Topic strings
  char topicTemperature[50];
  char topicHumidity[50];
  char topicPressure[50];
  char topicSetpoint[50];
  char topicSetpointSet[50];
  char topicValvePosition[50];
  char topicMode[50];
  char topicModeSet[50];
  char topicHeating[50];
  
  // Initialize topics
  void setupTopics();
  
  // Callback for receiving messages
  static void handleMqttMessage(char* topic, byte* payload, unsigned int length);
  
  // Helper for payload extraction
  static float extractFloatPayload(byte* payload, unsigned int length);
  static int extractIntPayload(byte* payload, unsigned int length);
};

#endif // MQTT_INTERFACE_H