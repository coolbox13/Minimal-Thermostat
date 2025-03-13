#include <Arduino.h>
#include <Wire.h>
#include "bme280_sensor.h"

BME280Sensor::BME280Sensor() : initialized(false) {
}

bool BME280Sensor::begin() {
    Serial.println("Initializing BME280 sensor...");
    
    if (!bme.begin(0x76)) {  // Default I2C address for BME280
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        return false;
    }
    
    initialized = true;
    Serial.println("BME280 sensor initialized successfully");
    return true;
}

float BME280Sensor::readTemperature() {
    if (!initialized) return 0.0f;
    return bme.readTemperature();
}

float BME280Sensor::readHumidity() {
    if (!initialized) return 0.0f;
    return bme.readHumidity();
}

float BME280Sensor::readPressure() {
    if (!initialized) return 0.0f;
    return bme.readPressure() / 100.0F; // Convert to hPa
}