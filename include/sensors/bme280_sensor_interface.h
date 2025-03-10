#pragma once

#include <Adafruit_BME280.h>
#include "sensors/sensor_interface.h"

class BME280SensorInterface : public SensorInterface {
public:
    BME280SensorInterface(uint8_t i2cAddress = 0x76);
    virtual ~BME280SensorInterface() = default;

    bool begin() override;
    void loop() override;
    float getTemperature() const override;
    float getHumidity() const override;
    float getPressure() const override;
    void setValvePosition(float position) override;
    ThermostatStatus getLastError() const override;
    const char* getLastErrorMessage() const override;
    void clearError() override;

private:
    void updateReadings();

    Adafruit_BME280 bme;
    uint8_t address;
    float temperature;
    float humidity;
    float pressure;
    float valvePosition;
    ThermostatStatus lastError;
    char errorMessage[64];
    unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 1000; // 1 second
}; 