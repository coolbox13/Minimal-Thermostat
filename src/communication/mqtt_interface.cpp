#include "mqtt_interface.h"
#include "protocol_manager.h"

MQTTInterface::MQTTInterface() : 
  mqttClient(wifiClient),
  port(1883),
  connectionActive(false),
  thermostatState(nullptr),
  protocolManager(nullptr),
  lastConnectionAttempt(0) {
  
  // Initialize empty strings
  server[0] = '\0';
  username[0] = '\0';
  password[0] = '\0';
  clientId[0] = '\0';
}

bool MQTTInterface::begin(ThermostatState* state, 
                          const char* serverStr, 
                          int portNum, 
                          const char* user, 
                          const char* pass,
                          const char* client) {
  
  thermostatState = state;
  
  // Store connection parameters
  strlcpy(server, serverStr, sizeof(server));
  port = portNum;
  strlcpy(username, user, sizeof(username));
  strlcpy(password, pass, sizeof(password));
  strlcpy(clientId, client, sizeof(clientId));
  
  // Configure MQTT client
  mqttClient.setServer(server, port);
  mqttClient.setCallback(handleMqttMessage);
  mqttClient.setBufferSize(512); // Increase buffer size for larger messages
  
  // Setup topics
  setupTopics();
  
  // Try to connect
  bool result = reconnect();
  
  Serial.println(result ? "MQTT connected successfully" : "MQTT connection failed");
  return result;
}

void MQTTInterface::registerProtocolManager(ProtocolManager* manager) {
  protocolManager = manager;
}

void MQTTInterface::loop() {
  // Check if connected and maintain connection
  if (!mqttClient.connected()) {
    unsigned long currentTime = millis();
    // Try to reconnect every 5 seconds
    if (currentTime - lastConnectionAttempt > 5000) {
      lastConnectionAttempt = currentTime;
      reconnect();
    }
  }
  
  // Process MQTT messages
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

bool MQTTInterface::isConnected() const {
  return mqttClient.connected();
}

bool MQTTInterface::reconnect() {
  // Don't try to connect if no server specified
  if (server[0] == '\0') {
    return false;
  }
  
  Serial.print("Attempting MQTT connection to ");
  Serial.print(server);
  Serial.print(":");
  Serial.print(port);
  Serial.print(" as ");
  Serial.print(clientId);
  Serial.println("...");
  
  // Configure connection with or without credentials
  bool connected = false;
  if (username[0] != '\0') {
    connected = mqttClient.connect(clientId, username, password);
  } else {
    connected = mqttClient.connect(clientId);
  }
  
  if (connected) {
    Serial.println("Connected to MQTT broker");
    connectionActive = true;
    
    // Subscribe to command topics
    mqttClient.subscribe(topicSetpointSet);
    mqttClient.subscribe(topicModeSet);
    
    // Publish initial state
    if (thermostatState) {
      publishTemperature(thermostatState->currentTemperature);
      publishHumidity(thermostatState->currentHumidity);
      publishPressure(thermostatState->currentPressure);
      publishSetpoint(thermostatState->targetTemperature);
      publishValvePosition(thermostatState->valvePosition);
      publishMode(thermostatState->operatingMode);
      publishHeatingStatus(thermostatState->heatingActive);
    }
  } else {
    connectionActive = false;
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqttClient.state());
  }
  
  return connectionActive;
}

void MQTTInterface::setupTopics() {
  // Construct topic strings based on client ID for unique identification
  snprintf(topicTemperature, sizeof(topicTemperature), "thermostat/%s/temperature", clientId);
  snprintf(topicHumidity, sizeof(topicHumidity), "thermostat/%s/humidity", clientId);
  snprintf(topicPressure, sizeof(topicPressure), "thermostat/%s/pressure", clientId);
  snprintf(topicSetpoint, sizeof(topicSetpoint), "thermostat/%s/setpoint", clientId);
  snprintf(topicSetpointSet, sizeof(topicSetpointSet), "thermostat/%s/setpoint/set", clientId);
  snprintf(topicValvePosition, sizeof(topicValvePosition), "thermostat/%s/valve", clientId);
  snprintf(topicMode, sizeof(topicMode), "thermostat/%s/mode", clientId);
  snprintf(topicModeSet, sizeof(topicModeSet), "thermostat/%s/mode/set", clientId);
  snprintf(topicHeating, sizeof(topicHeating), "thermostat/%s/heating", clientId);
}

void MQTTInterface::publishTemperature(float temperature) {
  if (!mqttClient.connected()) return;
  
  char payload[10];
  snprintf(payload, sizeof(payload), "%.2f", temperature);
  mqttClient.publish(topicTemperature, payload, true);
}

void MQTTInterface::publishHumidity(float humidity) {
  if (!mqttClient.connected()) return;
  
  char payload[10];
  snprintf(payload, sizeof(payload), "%.2f", humidity);
  mqttClient.publish(topicHumidity, payload, true);
}

void MQTTInterface::publishPressure(float pressure) {
  if (!mqttClient.connected()) return;
  
  char payload[10];
  snprintf(payload, sizeof(payload), "%.2f", pressure);
  mqttClient.publish(topicPressure, payload, true);
}

void MQTTInterface::publishSetpoint(float setpoint) {
  if (!mqttClient.connected()) return;
  
  char payload[10];
  snprintf(payload, sizeof(payload), "%.2f", setpoint);
  mqttClient.publish(topicSetpoint, payload, true);
}

void MQTTInterface::publishValvePosition(float position) {
  if (!mqttClient.connected()) return;
  
  char payload[10];
  snprintf(payload, sizeof(payload), "%.2f", position);
  mqttClient.publish(topicValvePosition, payload, true);
}

void MQTTInterface::publishMode(ThermostatMode mode) {
  if (!mqttClient.connected()) return;
  
  char payload[5];
  snprintf(payload, sizeof(payload), "%d", mode);
  mqttClient.publish(topicMode, payload, true);
}

void MQTTInterface::publishHeatingStatus(bool isHeating) {
  if (!mqttClient.connected()) return;
  
  mqttClient.publish(topicHeating, isHeating ? "ON" : "OFF", true);
}

void MQTTInterface::handleMqttMessage(char* topic, byte* payload, unsigned int length) {
  // This static callback needs access to object instance
  // Let's use PubSubClient's client reference to get our instance
  PubSubClient* client = (PubSubClient*)PubSubClient::getInternalClient();
  if (!client) return;
  
  // Get this object from the client
  MQTTInterface* instance = (MQTTInterface*)client->getInternalData();
  if (!instance || !instance->thermostatState || !instance->protocolManager) return;
  
  // Check which topic received a message
  if (strcmp(topic, instance->topicSetpointSet) == 0) {
    // Process setpoint command
    float newSetpoint = extractFloatPayload(payload, length);
    if (newSetpoint > 0 && newSetpoint < 40) {  // Sanity check
      Serial.printf("MQTT: Received setpoint command: %.2f°C\n", newSetpoint);
      
      instance->handleSetpointCallback(topic, payload);
    }
  } else if (strcmp(topic, instance->topicModeSet) == 0) {
    // Process mode command
    int modeValue = extractIntPayload(payload, length);
    if (modeValue >= 0 && modeValue <= 5) {  // Sanity check
      Serial.printf("MQTT: Received mode command: %d\n", modeValue);
      
      instance->handleModeCallback(topic, payload);
    }
  }
}

float MQTTInterface::extractFloatPayload(byte* payload, unsigned int length) {
  // Create null-terminated string from payload
  char message[64];
  if (length > 63) length = 63;
  memcpy(message, payload, length);
  message[length] = '\0';
  
  // Convert to float
  return atof(message);
}

int MQTTInterface::extractIntPayload(byte* payload, unsigned int length) {
  // Create null-terminated string from payload
  char message[64];
  if (length > 63) length = 63;
  memcpy(message, payload, length);
  message[length] = '\0';
  
  // Convert to integer
  return atoi(message);
}

void MQTTInterface::handleSetpointCallback(const char* topic, const char* payload) {
    float setpoint = atof(payload);
    if (protocolManager) {
        protocolManager->handleIncomingCommand(
            CommandSource::SOURCE_MQTT,
            CommandType::CMD_SET_TEMPERATURE,
            setpoint
        );
    }
}

void MQTTInterface::handleModeCallback(const char* topic, const char* payload) {
    int modeValue = atoi(payload);
    if (protocolManager) {
        protocolManager->handleIncomingCommand(
            CommandSource::SOURCE_MQTT,
            CommandType::CMD_SET_MODE,
            static_cast<float>(modeValue)
        );
    }
}