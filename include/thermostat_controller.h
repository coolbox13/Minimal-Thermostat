#pragma once

#include <Arduino.h>
#include "pid_controller.h"
#include "sensor_interface.h"
#include "thermostat_types.h"

class ThermostatController {
private:
    float currentTemp;
    float targetTemp;
    float output;
    bool isHeating;
    ThermostatMode mode;
    PIDController* pidController;
    SensorInterface* sensorInterface;
    float hysteresis;

public:
    ThermostatController(PIDController* pid, SensorInterface* sensor);
    ~ThermostatController();

    void begin();
    void update();

    // Getters
    float getCurrentTemperature() const { return currentTemp; }
    float getTargetTemperature() const { return targetTemp; }
    float getOutput() const { return output; }
    bool isActive() const { return isHeating; }
    ThermostatMode getMode() const { return mode; }

    // Setters
    void setTargetTemperature(float temp) { targetTemp = temp; }
    void setMode(ThermostatMode newMode) { mode = newMode; }
    void setHysteresis(float hyst) { hysteresis = hyst; }
}; 