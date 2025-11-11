#ifndef MOCK_ADAFRUIT_BME280_H
#define MOCK_ADAFRUIT_BME280_H

#include <stdint.h>
#include <math.h>

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

/**
 * Mock implementation of Adafruit BME280 sensor library for testing
 * Allows tests to control sensor behavior and readings
 */
class Adafruit_BME280 {
private:
    bool _initialized;
    float _temperature;
    float _humidity;
    float _pressure;
    bool _shouldFail;

public:
    Adafruit_BME280()
        : _initialized(false)
        , _temperature(22.0f)
        , _humidity(50.0f)
        , _pressure(101325.0f)
        , _shouldFail(false) {}

    /**
     * Initialize the sensor
     * @param addr I2C address (ignored in mock)
     * @return true if successful, false if mock is set to fail
     */
    bool begin(uint8_t addr = 0x76) {
        if (_shouldFail) {
            return false;
        }
        _initialized = true;
        return true;
    }

    /**
     * Read temperature in Celsius
     * @return Temperature value or NaN if not initialized/failed
     */
    float readTemperature() {
        if (!_initialized || _shouldFail) {
            return NAN;
        }
        return _temperature;
    }

    /**
     * Read humidity as percentage (0-100)
     * @return Humidity value or NaN if not initialized/failed
     */
    float readHumidity() {
        if (!_initialized || _shouldFail) {
            return NAN;
        }
        return _humidity;
    }

    /**
     * Read atmospheric pressure in Pascals
     * @return Pressure value or NaN if not initialized/failed
     */
    float readPressure() {
        if (!_initialized || _shouldFail) {
            return NAN;
        }
        return _pressure;
    }

    // ===== Test Control Methods =====

    /**
     * Set the mock temperature reading
     * @param temp Temperature in Celsius
     */
    void setMockTemperature(float temp) {
        _temperature = temp;
    }

    /**
     * Set the mock humidity reading
     * @param humidity Humidity percentage (0-100)
     */
    void setMockHumidity(float humidity) {
        _humidity = humidity;
    }

    /**
     * Set the mock pressure reading
     * @param pressure Pressure in Pascals
     */
    void setMockPressure(float pressure) {
        _pressure = pressure;
    }

    /**
     * Control whether the sensor should fail
     * @param shouldFail true to make sensor fail, false for normal operation
     */
    void setMockShouldFail(bool shouldFail) {
        _shouldFail = shouldFail;
        if (shouldFail) {
            _initialized = false;
        }
    }

    /**
     * Reset the mock to default values
     */
    void resetMock() {
        _initialized = false;
        _temperature = 22.0f;
        _humidity = 50.0f;
        _pressure = 101325.0f;
        _shouldFail = false;
    }
};

#endif // MOCK_ADAFRUIT_BME280_H
