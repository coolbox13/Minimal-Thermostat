#include "sensors/bme280_sensor_interface.h"
#include <Arduino.h>

BME280SensorInterface::BME280SensorInterface(uint8_t i2cAddress) 
    : address(i2cAddress), 
      temperature(0.0f), 
      humidity(0.0f), 
      pressure(0.0f), 
      lastError(ThermostatStatus::OK), 
      lastReadTime(0) {
    memset(errorMessage, 0, sizeof(errorMessage));
}

bool BME280SensorInterface::begin() {
    if (!bme.begin(address)) {
        lastError = ThermostatStatus::ERROR_SENSOR;
        snprintf(errorMessage, sizeof(errorMessage), "Could not find BME280 sensor at address 0x%02X", address);
        return false;
    }
    
    // Set up the BME280 sensor
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X1,  // temperature
                    Adafruit_BME280::SAMPLING_X1,  // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_OFF,
                    Adafruit_BME280::STANDBY_MS_1000);
    
    lastReadTime = millis();
    updateReadings();
    return true;
}

void BME280SensorInterface::loop() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= 1000) { // Update every second
        updateReadings();
        lastReadTime = currentTime;
    }
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

ThermostatStatus BME280SensorInterface::getLastError() const {
    return lastError;
}

const char* BME280SensorInterface::getLastErrorMessage() const {
    return errorMessage;
}

void BME280SensorInterface::clearError() {
    lastError = ThermostatStatus::OK;
    memset(errorMessage, 0, sizeof(errorMessage));
}

void BME280SensorInterface::updateReadings() {
    if (lastError != ThermostatStatus::OK && lastError != ThermostatStatus::WARNING) {
        // Try to reconnect if there was an error
        if (!bme.begin(address)) {
            lastError = ThermostatStatus::ERROR_SENSOR;
            snprintf(errorMessage, sizeof(errorMessage), "Failed to reconnect to BME280 sensor");
            return;
        }
        clearError();
    }
    
    // Read sensor data
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
    
    // Check for invalid readings
    if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
        lastError = ThermostatStatus::WARNING;
        snprintf(errorMessage, sizeof(errorMessage), "Invalid sensor readings detected");
    }
} 