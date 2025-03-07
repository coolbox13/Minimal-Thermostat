#include <Arduino.h>
#include "config_manager.h"
#include "sensor_interface.h"
#include "knx_interface.h"
#include "mqtt_interface.h"
#include "web_interface.h"
#include "pid_controller.h"
#include "thermostat_state.h"

// Main components
ConfigManager configManager;
ThermostatState thermostatState;
SensorInterface sensorInterface;
KNXInterface knxInterface;
MQTTInterface mqttInterface;
WebInterface webInterface;
PIDController pidController;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n\nStarting ESP32-KNX-Thermostat...");
  
  // Initialize file system and configuration
  configManager.begin();
  
  // Setup WiFi connection
  configManager.setupWiFi();
  
  // Initialize thermostat components with configuration
  sensorInterface.begin(&thermostatState);
  knxInterface.begin(
    configManager.getKnxPhysicalArea(),
    configManager.getKnxPhysicalLine(),
    configManager.getKnxPhysicalMember()
  );
  
  // Register KNX group addresses
  knxInterface.setTemperatureGA(
    configManager.getKnxTempArea(),
    configManager.getKnxTempLine(),
    configManager.getKnxTempMember()
  );
  knxInterface.setSetpointGA(
    configManager.getKnxSetpointArea(),
    configManager.getKnxSetpointLine(),
    configManager.getKnxSetpointMember()
  );
  knxInterface.setValvePositionGA(
    configManager.getKnxValveArea(),
    configManager.getKnxValveLine(),
    configManager.getKnxValveMember()
  );
  
  // Register KNX callbacks
  knxInterface.registerCallbacks(&thermostatState);
  
  // Setup MQTT
  mqttInterface.begin(
    &thermostatState,
    configManager.getMqttServer(),
    configManager.getMqttPort(),
    configManager.getMqttUser(),
    configManager.getMqttPassword(),
    configManager.getMqttClientId()
  );
  
  // Initialize PID controller
  pidController.begin(
    &thermostatState,
    configManager.getKp(),
    configManager.getKi(),
    configManager.getKd()
  );
  
  // Setup web interface last (after all other components are initialized)
  webInterface.begin(&thermostatState, &configManager, &sensorInterface, &knxInterface, &mqttInterface, &pidController);
  
  // Set initial target temperature from configuration
  thermostatState.setTargetTemperature(configManager.getSetpoint());
  
  Serial.println("Setup completed");
}

void loop() {
  // Process web server requests
  webInterface.handle();
  
  // Process KNX communication
  knxInterface.loop();
  
  // Process MQTT communication
  mqttInterface.loop();
  
  // Update sensor readings periodically
  sensorInterface.update();
  
  // Run PID controller
  pidController.update();
  
  // Allow WiFi and system tasks to run
  yield();
}