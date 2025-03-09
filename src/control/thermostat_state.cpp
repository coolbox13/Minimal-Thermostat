#include "thermostat_state.h"

ThermostatState::ThermostatState() :
  currentTemperature(0.0f),
  currentHumidity(0.0f),
  currentPressure(0.0f),
  targetTemperature(ThermostatLimits::DEFAULT_TEMPERATURE),
  valvePosition(0.0f),
  operatingMode(ThermostatMode::OFF),
  heatingActive(false),
  status(ThermostatStatus::OK),
  temperatureCallback(nullptr),
  humidityCallback(nullptr),
  pressureCallback(nullptr),
  targetTemperatureCallback(nullptr),
  valvePositionCallback(nullptr),
  modeCallback(nullptr),
  heatingCallback(nullptr),
  statusCallback(nullptr) {
}

void ThermostatState::setTemperature(float value) {
  if (value != currentTemperature) {
    currentTemperature = value;
    if (temperatureCallback) {
      temperatureCallback(value);
    }
  }
}

void ThermostatState::setHumidity(float value) {
  if (value != currentHumidity) {
    currentHumidity = value;
    if (humidityCallback) {
      humidityCallback(value);
    }
  }
}

void ThermostatState::setPressure(float value) {
  if (value != currentPressure) {
    currentPressure = value;
    if (pressureCallback) {
      pressureCallback(value);
    }
  }
}

void ThermostatState::setTargetTemperature(float value) {
  if (!isValidTemperature(value)) {
    return;
  }
  
  if (value != targetTemperature) {
    targetTemperature = value;
    if (targetTemperatureCallback) {
      targetTemperatureCallback(value);
    }
  }
}

void ThermostatState::setValvePosition(float value) {
  if (!isValidValvePosition(value)) {
    return;
  }
  
  if (value != valvePosition) {
    valvePosition = value;
    if (valvePositionCallback) {
      valvePositionCallback(value);
    }
  }
}

void ThermostatState::setMode(ThermostatMode mode) {
  if (mode != operatingMode) {
    operatingMode = mode;
    if (modeCallback) {
      modeCallback(mode);
    }
  }
} 