#include "sensors/bme280_sensor_interface.h"
#include <Wire.h>
#include <esp_log.h>

static const char* TAG = "BME280";

BME280SensorInterface::BME280SensorInterface()
    : temperature(0)
    , humidity(0)
    , pressure(0)
    , temperatureOffset(0)
    , humidityOffset(0)
    , pressureOffset(0)
    , updateInterval(30000)  // Default 30 seconds
    , lastUpdateTime(0)
    , lastError(ThermostatStatus::OK) {
    memset(lastErrorMessage, 0, sizeof(lastErrorMessage));
}

bool BME280SensorInterface::begin() {
    if (!bme.begin(BME280_ADDRESS_ALTERNATE)) {
        snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Could not find BME280 sensor");
        lastError = ThermostatStatus::ERROR_SENSOR;
        return false;
    }
    
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X16,  // temperature
                    Adafruit_BME280::SAMPLING_X16,  // pressure
                    Adafruit_BME280::SAMPLING_X16,  // humidity
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5);
    
    return true;
}

void BME280SensorInterface::loop() {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= updateInterval) {
        updateReadings();
        lastUpdateTime = currentTime;
    }
}

void BME280SensorInterface::updateReadings() {
    if (lastError != ThermostatStatus::OK && lastError != ThermostatStatus::WARNING) {
        if (!begin()) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Failed to reinitialize BME280");
            lastError = ThermostatStatus::ERROR_SENSOR;
            return;
        }
    }
    
    float newTemp = bme.readTemperature() + temperatureOffset;
    float newHumidity = bme.readHumidity() + humidityOffset;
    float newPressure = (bme.readPressure() / 100.0F) + pressureOffset;
    
    if (isnan(newTemp) || isnan(newHumidity) || isnan(newPressure)) {
        snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Failed to read from BME280");
        lastError = ThermostatStatus::WARNING;
        return;
    }
    
    temperature = newTemp;
    humidity = newHumidity;
    pressure = newPressure;
    
    clearError();
}

void BME280SensorInterface::setUpdateInterval(unsigned long interval) {
    updateInterval = interval;
}

bool BME280SensorInterface::isAvailable() const {
    return lastError != ThermostatStatus::ERROR_SENSOR;
}

float BME280SensorInterface::getTemperature() const {
    return temperature;
}

float BME280SensorInterface::getHumidity() const {
    return humidity;
}

float BME280SensorInterface::getPressure() const {
    return pressure;
}

void BME280SensorInterface::setTemperatureOffset(float offset) {
    temperatureOffset = offset;
}

void BME280SensorInterface::setHumidityOffset(float offset) {
    humidityOffset = offset;
}

void BME280SensorInterface::setPressureOffset(float offset) {
    pressureOffset = offset;
}

ThermostatStatus BME280SensorInterface::getLastError() const {
    return lastError;
}

const char* BME280SensorInterface::getLastErrorMessage() const {
    return lastErrorMessage;
}

void BME280SensorInterface::clearError() {
    lastError = ThermostatStatus::OK;
    memset(lastErrorMessage, 0, sizeof(lastErrorMessage));
} 