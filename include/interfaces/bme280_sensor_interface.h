#pragma once

#include "sensor_interface.h"
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <esp_log.h>

class BME280SensorInterface : public SensorInterface {
public:
    BME280SensorInterface();
    virtual ~BME280SensorInterface() = default;

    bool begin() override;
    void loop() override;
    void setUpdateInterval(unsigned long interval) override;

    float getTemperature() const override;
    float getHumidity() const override;
    float getPressure() const override;
    bool isAvailable() const override;
    ThermostatStatus getLastError() const override;

    void setTemperatureOffset(float offset) override;
    void setHumidityOffset(float offset) override;
    void setPressureOffset(float offset) override;

private:
    Adafruit_BME280 bme;
    unsigned long lastUpdateTime;
    unsigned long updateInterval;
    float temperature;
    float humidity;
    float pressure;
    float tempOffset;
    float humidityOffset;
    float pressureOffset;
    ThermostatStatus lastError;

    void updateReadings();
}; 