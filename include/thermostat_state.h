#ifndef THERMOSTAT_STATE_H
#define THERMOSTAT_STATE_H

#include <Arduino.h>
#include <functional>
#include "thermostat_types.h"

class ThermostatState {
public:
  // Constructor with default values
  ThermostatState();
  
  // Getters
  float getCurrentTemperature() const { return currentTemperature; }
  float getCurrentHumidity() const { return currentHumidity; }
  float getCurrentPressure() const { return currentPressure; }
  float getTargetTemperature() const { return targetTemperature; }
  float getValvePosition() const { return valvePosition; }
  ThermostatMode getMode() const { return operatingMode; }
  bool isHeating() const { return heatingActive; }
  ThermostatStatus getStatus() const { return status; }
  
  // Setters
  void setTemperature(float value);
  void setHumidity(float value);
  void setPressure(float value);
  void setTargetTemperature(float value);
  void setValvePosition(float value);
  void setMode(ThermostatMode mode);
  void setHeating(bool active) { heatingActive = active; }
  void setStatus(ThermostatStatus newStatus) { status = newStatus; }
  
  // Callback function types
  using TemperatureCallback = std::function<void(float)>;
  using HumidityCallback = std::function<void(float)>;
  using PressureCallback = std::function<void(float)>;
  using TargetTemperatureCallback = std::function<void(float)>;
  using ValvePositionCallback = std::function<void(float)>;
  using ModeCallback = std::function<void(ThermostatMode)>;
  using HeatingCallback = std::function<void(bool)>;
  using StatusCallback = std::function<void(ThermostatStatus)>;
  
  // Register callbacks
  void onTemperatureChange(TemperatureCallback cb) { temperatureCallback = cb; }
  void onHumidityChange(HumidityCallback cb) { humidityCallback = cb; }
  void onPressureChange(PressureCallback cb) { pressureCallback = cb; }
  void onTargetTemperatureChange(TargetTemperatureCallback cb) { targetTemperatureCallback = cb; }
  void onValvePositionChange(ValvePositionCallback cb) { valvePositionCallback = cb; }
  void onModeChange(ModeCallback cb) { modeCallback = cb; }
  void onHeatingChange(HeatingCallback cb) { heatingCallback = cb; }
  void onStatusChange(StatusCallback cb) { statusCallback = cb; }

private:
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
  ThermostatStatus status;
  
  // Callbacks
  TemperatureCallback temperatureCallback;
  HumidityCallback humidityCallback;
  PressureCallback pressureCallback;
  TargetTemperatureCallback targetTemperatureCallback;
  ValvePositionCallback valvePositionCallback;
  ModeCallback modeCallback;
  HeatingCallback heatingCallback;
  StatusCallback statusCallback;
  
  // Helper to validate temperature range
  bool isValidTemperature(float temp) const {
    return temp >= ThermostatLimits::MIN_TEMPERATURE && 
           temp <= ThermostatLimits::MAX_TEMPERATURE;
  }
  
  // Helper to validate valve position range
  bool isValidValvePosition(float pos) const {
    return pos >= ThermostatLimits::MIN_VALVE_POSITION && 
           pos <= ThermostatLimits::MAX_VALVE_POSITION;
  }
};

#endif // THERMOSTAT_STATE_H