#pragma once

#include <Adafruit_BME280.h>
#include "thermostat_types.h"
#include "interfaces/sensor_interface.h"

class BME280SensorInterface : public SensorInterface {
public:
    BME280SensorInterface();
    
    bool begin() override;
    void loop() override;
    void updateReadings() override;
    void setUpdateInterval(unsigned long interval) override;
    bool isAvailable() const override;
    
    float getTemperature() const override;
    float getHumidity() const override;
    float getPressure() const override;
    
    void setTemperatureOffset(float offset) override;
    void setHumidityOffset(float offset) override;
    void setPressureOffset(float offset) override;
    
    ThermostatStatus getLastError() const override;
    const char* getLastErrorMessage() const override;
    void clearError() override;
    
private:
    Adafruit_BME280 bme;
    float temperature;
    float humidity;
    float pressure;
    float temperatureOffset;
    float humidityOffset;
    float pressureOffset;
    unsigned long updateInterval;
    unsigned long lastUpdateTime;
    ThermostatStatus lastError;
    char lastErrorMessage[128];
}; 