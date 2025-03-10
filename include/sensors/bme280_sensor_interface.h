#pragma once

#include <Adafruit_BME280.h>
#include "thermostat_types.h"
#include "interfaces/sensor_interface.h"

class BME280SensorInterface : public SensorInterface {
public:
    BME280SensorInterface();
    
    bool begin() override;
    void updateReadings() override;
    
    float getTemperature() const override;
    float getHumidity() const override;
    float getPressure() const override;
    
    ThermostatStatus getLastError() const override;
    const char* getLastErrorMessage() const override;
    void clearError() override;
    
private:
    Adafruit_BME280 bme;
    float temperature;
    float humidity;
    float pressure;
    ThermostatStatus lastError;
    char lastErrorMessage[128];
}; 