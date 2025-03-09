#ifndef KNX_INTERFACE_H
#define KNX_INTERFACE_H

#include <Arduino.h>
#include <esp-knx-ip.h>

// Forward declaration to avoid circular dependencies
class ThermostatState;

// Enum for thermostat modes
enum ThermostatMode {
  MODE_OFF = 0,
  MODE_COMFORT = 1,
  MODE_ECO = 2,
  MODE_AWAY = 3,
  MODE_BOOST = 4,
  MODE_ANTIFREEZE = 5
};

class KNXInterface {
public:
  // Constructor
  KNXInterface();
  
  // Initialize KNX communication
  bool begin(int area = 1, int line = 1, int member = 201);
  
  // Register with thermostat state
  void registerCallbacks(ThermostatState* state);
  
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
  // Helper to calculate group address
  address_t calculateGA(int area, int line, int member);
  
  // KNX callbacks
  static void handleSetpointCallback(message_t const &msg, void *arg);
  static void handleModeCallback(message_t const &msg, void *arg);
  
  // Group addresses
  address_t temperatureGA;
  address_t setpointGA;
  address_t valvePositionGA;
  address_t modeGA;
  
  // Thermostat state reference
  ThermostatState* thermostatState;
  
  // Callback IDs
  callback_id_t setpointCallbackId;
  callback_id_t modeCallbackId;
  
  // Helper for registering with KNX callbacks
  void registerKNXCallbacks();
};

#endif // KNX_INTERFACE_H