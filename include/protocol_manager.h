#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <Arduino.h>
#include "thermostat_state.h"
#include "knx_interface.h"

// Source definitions
#define SOURCE_WEB_API 1
#define SOURCE_KNX 2
#define SOURCE_MQTT 3

// Command definitions
#define CMD_SET_TEMPERATURE 1
#define CMD_SET_MODE 2
#define CMD_SET_VALVE 3

// Forward declarations if needed
class MQTTInterface;

class ProtocolManager {
public:
  // Constructor
  ProtocolManager();
  
  // Initialize with thermostat state
  void begin(ThermostatState* state);
  
  // Register protocols
  void registerProtocols(KNXInterface* knx);
  
  // Optional MQTT registration
  void registerMQTT(MQTTInterface* mqtt);
  
  // Process communications
  void loop();
  
  // Handle incoming commands from any source
  bool handleIncomingCommand(int source, int command, float value);
  
  // Send updates
  void sendTemperature(float temperature);
  void sendSetpoint(float setpoint);
  void sendValvePosition(float position);
  void sendMode(ThermostatMode mode);
  
private:
  // References to interfaces
  KNXInterface* knxInterface;
  MQTTInterface* mqttInterface;
  
  // Thermostat state reference
  ThermostatState* thermostatState;
  
  // Latest send time
  unsigned long lastSendTime;
  unsigned long sendInterval;
};

#endif // PROTOCOL_MANAGER_H