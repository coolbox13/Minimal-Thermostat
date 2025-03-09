// Basic includes
#include <Arduino.h>

// Include the ThermostatState first since other components depend on it
#include "thermostat_state.h"

// Then include your component headers
#include "knx_interface.h"
#include "config_manager.h"

ThermostatState::ThermostatState() :
  currentTemperature(20.0),
  currentHumidity(50.0),
  currentPressure(1013.0),
  targetTemperature(21.0),
  valvePosition(0.0),
  operatingMode(MODE_COMFORT),
  heatingActive(false),
  onTemperatureChanged(nullptr),
  onHumidityChanged(nullptr),
  onPressureChanged(nullptr),
  onSetpointChanged(nullptr),
  onValvePositionChanged(nullptr),
  onModeChanged(nullptr)
{}

void ThermostatState::setTemperature(float value) {
  if (value != currentTemperature) {
    currentTemperature = value;
    if (onTemperatureChanged) {
      onTemperatureChanged(value);
    }
  }
}

void ThermostatState::setHumidity(float value) {
  if (value != currentHumidity) {
    currentHumidity = value;
    if (onHumidityChanged) {
      onHumidityChanged(value);
    }
  }
}

void ThermostatState::setPressure(float value) {
  currentPressure = value;
  if (onPressureChanged) {
    onPressureChanged(value);
  }
}

void ThermostatState::setTargetTemperature(float value) {
  if (value != targetTemperature) {
    targetTemperature = value;
    if (onSetpointChanged) {
      onSetpointChanged(value);
    }
  }
}

void ThermostatState::setValvePosition(float value) {
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

void ThermostatState::setMode(ThermostatMode mode) {
  if (mode != operatingMode) {
    operatingMode = mode;
    if (onModeChanged) {
      onModeChanged(mode);
    }
  }
}