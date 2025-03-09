#ifndef THERMOSTAT_STATE_H
#define THERMOSTAT_STATE_H

#include <Arduino.h>
#include <functional>

// Include the ThermostatMode enum from knx_interface.h
#include "knx_interface.h"

// Structure to hold thermostat state
class ThermostatState {
public:
  // Constructor with default values
  ThermostatState();
  
  // Sensor readings
  float currentTemperature;
  float currentHumidity;
  float currentPressure;
  
  // Control values
  float targetTemperature;
  float valvePosition;  // 0-100%
  
  // Operation state
  ThermostatMode operatingMode;
  bool heatingActive;
  
  // Callback function types
  using TemperatureCallback = std::function<void(float)>;
  using HumidityCallback = std::function<void(float)>;
  using PressureCallback = std::function<void(float)>;
  using SetpointCallback = std::function<void(float)>;
  using ValvePositionCallback = std::function<void(float)>;
  using ModeCallback = std::function<void(ThermostatMode)>;
  
  // Callback handlers
  TemperatureCallback onTemperatureChanged;
  HumidityCallback onHumidityChanged;
  PressureCallback onPressureChanged;
  SetpointCallback onSetpointChanged;
  ValvePositionCallback onValvePositionChanged;
  ModeCallback onModeChanged;
  
  // Methods to update values with callback triggers
  void setTemperature(float value);
  void setHumidity(float value);
  void setPressure(float value);
  void setTargetTemperature(float value);
  void setValvePosition(float value);
  void setMode(ThermostatMode mode);
};

#endif // THERMOSTAT_STATE_H