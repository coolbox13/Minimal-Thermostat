#pragma once

#include "interfaces/sensor_interface.h"  // Use the abstract sensor interface from interfaces folder
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <esp_log.h>
#include "thermostat_types.h"

class BME280SensorInterface : public SensorInterface {
public:
    BME280SensorInterface();
    virtual ~BME280SensorInterface() = default;

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
    void setSensorMode(); // Internal helper to configure the sensor mode
    
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