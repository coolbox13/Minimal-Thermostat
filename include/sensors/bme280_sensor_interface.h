#pragma once

#include <Adafruit_BME280.h>
#include "thermostat_types.h"

class BME280SensorInterface {
public:
    BME280SensorInterface();
    
    bool begin();
    void updateReadings();
    
    float getTemperature() const;
    float getHumidity() const;
    float getPressure() const;
    
    ThermostatStatus getLastError() const;
    const char* getLastErrorMessage() const;
    void clearError();
    
private:
    Adafruit_BME280 bme;
    float temperature;
    float humidity;
    float pressure;
    ThermostatStatus lastError;
    char lastErrorMessage[128];
}; 