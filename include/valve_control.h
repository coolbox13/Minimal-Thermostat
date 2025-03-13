// Add this include at the top of main.cpp
#include "valve_control.h"

// Add this declaration after the other global variables
ValveControl valveControl(mqttClient, knx);

// Add to setup() after setupKNX()
valveControl.begin();
// Register valve control with Home Assistant
valveControl.registerWithHA("esp32_thermostat");

// Update mqttCallback function to handle valve control messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
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
  
  // Let valve control process the message first
  if (valveControl.processMQTTMessage(topic, message)) {
    return; // Message was handled by valve control
  }
  
  // Handle other messages
  if (strcmp(topic, MQTT_TOPIC_VALVE_COMMAND) == 0) {
    int position = atoi(message);
    setValvePosition(position);
  }
}