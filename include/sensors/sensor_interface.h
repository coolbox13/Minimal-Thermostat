#pragma once

#include <Arduino.h>
#include "thermostat_types.h"

class SensorInterface {
public:
    virtual ~SensorInterface() = default;

    virtual bool begin() = 0;
    virtual void loop() = 0;
    virtual float getTemperature() const = 0;
    virtual float getHumidity() const = 0;
    virtual float getPressure() const = 0;
    virtual void setValvePosition(float position) = 0;
    virtual ThermostatStatus getLastError() const = 0;
    virtual const char* getLastErrorMessage() const = 0;
    virtual void clearError() = 0;
}; 