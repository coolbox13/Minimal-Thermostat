#ifndef THERMOSTAT_STATE_H
#define THERMOSTAT_STATE_H

#include <Arduino.h>
#include <functional>

// Operating modes for the thermostat
enum ThermostatMode {
  MODE_OFF = 0,
  MODE_COMFORT = 1,
  MODE_ECO = 2,
  MODE_AWAY = 3,
  MODE_BOOST = 4,
  MODE_ANTIFREEZE = 5
};

// Structure to hold thermostat state
struct ThermostatState {
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
  SetpointCallback onSetpointChanged;
  ValvePositionCallback onValvePositionChanged;
  ModeCallback onModeChanged;
  
  // Constructor with default values
  ThermostatState() :
    currentTemperature(20.0),
    currentHumidity(50.0),
    currentPressure(1013.0),
    targetTemperature(21.0),
    valvePosition(0.0),
    operatingMode(MODE_COMFORT),
    heatingActive(false),
    onTemperatureChanged(nullptr),
    onHumidityChanged(nullptr),
    onSetpointChanged(nullptr),
    onValvePositionChanged(nullptr),
    onModeChanged(nullptr)
  {}
  
  // Methods to update values with callback triggers
  void setTemperature(float value) {
    if (value != currentTemperature) {
      currentTemperature = value;
      if (onTemperatureChanged) {
        onTemperatureChanged(value);
      }
    }
  }
  
  void setHumidity(float value) {
    if (value != currentHumidity) {
      currentHumidity = value;
      if (onHumidityChanged) {
        onHumidityChanged(value);
      }
    }
  }
  
  void setPressure(float value) {
    currentPressure = value;
  }
  
  void setTargetTemperature(float value) {
    if (value != targetTemperature) {
      targetTemperature = value;
      if (onSetpointChanged) {
        onSetpointChanged(value);
      }
    }
  }
  
  void setValvePosition(float value) {
    // Ensure value is within bounds
    value = constrain(value, 0.0, 100.0);
    
    if (value != valvePosition) {
      valvePosition = value;
      heatingActive = (value > 0);
      if (onValvePositionChanged) {
        onValvePositionChanged(value);
      }
    }
  }
  
  void setMode(ThermostatMode mode) {
    if (mode != operatingMode) {
      operatingMode = mode;
      if (onModeChanged) {
        onModeChanged(mode);
      }
    }
  }
};

#endif // THERMOSTAT_STATE_H