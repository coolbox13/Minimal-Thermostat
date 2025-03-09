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
  
  // Getters
  float getCurrentTemperature() const { return currentTemperature; }
  float getCurrentHumidity() const { return currentHumidity; }
  float getCurrentPressure() const { return currentPressure; }
  float getTargetTemperature() const { return targetTemperature; }
  float getValvePosition() const { return valvePosition; }
  ThermostatMode getMode() const { return operatingMode; }
  bool isHeating() const { return heatingActive; }
  
  // Setters
  void setTemperature(float value);
  void setHumidity(float value);
  void setPressure(float value);
  void setTargetTemperature(float value);
  void setValvePosition(float value);
  void setMode(ThermostatMode mode);
  void setHeating(bool active) { heatingActive = active; }
  
  // Callback function types
  using TemperatureCallback = std::function<void(float)>;
  using HumidityCallback = std::function<void(float)>;
  using PressureCallback = std::function<void(float)>;
  using TargetTemperatureCallback = std::function<void(float)>;
  using ValvePositionCallback = std::function<void(float)>;
  using ModeCallback = std::function<void(ThermostatMode)>;
  using HeatingCallback = std::function<void(bool)>;
  
  // Register callbacks
  void onTemperatureChange(TemperatureCallback cb);
  void onHumidityChange(HumidityCallback cb);
  void onPressureChange(PressureCallback cb);
  void onTargetTemperatureChange(TargetTemperatureCallback cb);
  void onValvePositionChange(ValvePositionCallback cb);
  void onModeChange(ModeCallback cb);
  void onHeatingChange(HeatingCallback cb);

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
  
  // Callbacks
  TemperatureCallback temperatureCallback;
  HumidityCallback humidityCallback;
  PressureCallback pressureCallback;
  TargetTemperatureCallback targetTemperatureCallback;
  ValvePositionCallback valvePositionCallback;
  ModeCallback modeCallback;
  HeatingCallback heatingCallback;
};

#endif // THERMOSTAT_STATE_H