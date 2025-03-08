#ifndef KNX_INTERFACE_H
#define KNX_INTERFACE_H

#include <Arduino.h>
#include "esp-knx-ip.h"
#include "thermostat_state.h"

// Forward declaration to resolve circular dependency
class ProtocolManager;

class KNXInterface {
public:
  // Constructor
  KNXInterface();
  
  // Initialize KNX communication
  bool begin(int area = 1, int line = 1, int member = 201);
  
  // Register with thermostat state and protocol manager
  void registerCallbacks(ThermostatState* state, ProtocolManager* protocolManager);
  
  // Set group addresses for different datapoints
  void setTemperatureGA(int area, int line, int member);
  void setSetpointGA(int area, int line, int member);
  void setValvePositionGA(int area, int line, int member);
  void setModeGA(int area, int line, int member);
  
  // Process KNX loop
  void loop();
  
  // Send values to KNX bus
  bool sendTemperature(float temperature);
  bool sendSetpoint(float setpoint);
  bool sendValvePosition(float position);
  bool sendMode(ThermostatMode mode);
  
private:
  // Helper to convert our GA format to knx library format
  address_t convertToKnxGA(int area, int line, int member);
  
  // Store GA values
  address_t temperatureGA;
  address_t setpointGA;
  address_t valvePositionGA;
  address_t modeGA;
  
  // References to other components
  ThermostatState* thermostatState;
  ProtocolManager* protocolManager;
  
  // KNX callback IDs for the knx library
  callback_id_t setpointCallbackId;
  callback_id_t modeCallbackId;
  
  // Static callback handlers
  static void handleSetpointCallback(message_t const &msg, void *arg);
  static void handleModeCallback(message_t const &msg, void *arg);
};

#endif // KNX_INTERFACE_H