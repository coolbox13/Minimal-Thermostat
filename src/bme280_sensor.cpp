#include <Arduino.h>
#include <Wire.h>
#include "bme280_sensor.h"
#include "serial_monitor.h"
#include "serial_redirect.h"

// Redirect Serial to CapturedSerial for web monitor
#define Serial CapturedSerial

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

// CRITICAL FIX: Return NaN on failure instead of 0.0 (Audit Fix #3)
// This allows callers to distinguish between actual 0°C and sensor failure
float BME280Sensor::readTemperature() {
    if (!initialized) {
        return NAN; // Return NaN instead of 0.0f
    }
    float temp = bme.readTemperature();
    // Validate the reading from the library
    if (isnan(temp)) {
        Serial.println("BME280: readTemperature returned NaN");
    }
    return temp;
}

float BME280Sensor::readHumidity() {
    if (!initialized) {
        return NAN; // Return NaN instead of 0.0f
    }
    float humidity = bme.readHumidity();
    if (isnan(humidity)) {
        Serial.println("BME280: readHumidity returned NaN");
    }
    return humidity;
}

float BME280Sensor::readPressure() {
    if (!initialized) {
        return NAN; // Return NaN instead of 0.0f
    }
    float pressure = bme.readPressure();
    if (isnan(pressure)) {
        Serial.println("BME280: readPressure returned NaN");
        return NAN;
    }
    return pressure / 100.0F; // Convert to hPa
}

// CRITICAL FIX: Add health check method (Audit Fix #3)
bool BME280Sensor::isHealthy() {
    if (!initialized) {
        return false;
    }
    // Try to read temperature and check if it's valid
    float temp = bme.readTemperature();
    return !isnan(temp);
}