#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <Arduino.h>

// Forward declarations
class ThermostatState;
class KNXInterface;
class MQTTInterface;

// Source definitions
#define SOURCE_WEB_API 1
#define SOURCE_KNX 2
#define SOURCE_MQTT 3

// Command definitions
#define CMD_SET_TEMPERATURE 1
#define CMD_SET_MODE 2
#define CMD_SET_VALVE 3

// Include after forward declarations to avoid circular dependencies
#include "thermostat_state.h"

class ProtocolManager {
public:
  // Constructor
  ProtocolManager();
  
  // Initialize with thermostat state
  void begin(ThermostatState* state);
  
  // Register protocols
  void registerProtocols(KNXInterface* knx);
  void registerMQTT(MQTTInterface* mqtt);
  
  // Process protocol updates (call in loop)
  void loop();
  
  // Handle incoming commands from any protocol
  bool handleIncomingCommand(int source, int command, float value);
  
  // Send updates
  void sendTemperature(float temperature);
  void sendSetpoint(float setpoint);
  void sendValvePosition(float position);
  void sendMode(ThermostatMode mode);
  
private:
  // Protocol interfaces
  KNXInterface* knxInterface;
  MQTTInterface* mqttInterface;
  ThermostatState* thermostatState;
  
  // Timing control
  unsigned long lastSendTime;
  unsigned long sendInterval;
  
  // Helper functions to send updates to all protocols
  void sendTemperature(float temperature);
  void sendSetpoint(float setpoint);
  void sendValvePosition(float position);
  void sendMode(ThermostatMode mode);
};

#endif // PROTOCOL_MANAGER_H