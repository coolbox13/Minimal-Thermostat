#include "interfaces/sensor_interface.h"
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <esp_log.h>

static const char* TAG = "SensorInterface";

class BME280SensorInterface : public SensorInterface {
public:
    BME280SensorInterface() :
        lastUpdateTime(0),
        updateInterval(30000), // 30 seconds default
        tempOffset(0.0f),
        humidityOffset(0.0f),
        pressureOffset(0.0f),
        lastError(ThermostatStatus::OK) {}

    bool begin() override {
        Wire.begin();
        if (!bme.begin(BME280_ADDRESS_ALTERNATE)) {
            ESP_LOGE(TAG, "Could not find BME280 sensor!");
            lastError = ThermostatStatus::ERROR_SENSOR_READ;
            return false;
        }
        
        // Configure the sensor
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating Mode
                       Adafruit_BME280::SAMPLING_X2,       // Temp. oversampling
                       Adafruit_BME280::SAMPLING_X16,      // Pressure oversampling
                       Adafruit_BME280::SAMPLING_X1,       // Humidity oversampling
                       Adafruit_BME280::FILTER_X16,        // Filtering
                       Adafruit_BME280::STANDBY_MS_500);   // Standby time

        ESP_LOGI(TAG, "BME280 sensor initialized successfully");
        return true;
    }

    void loop() override {
        unsigned long now = millis();
        if (now - lastUpdateTime >= updateInterval) {
            updateReadings();
            lastUpdateTime = now;
        }
    }

    void setUpdateInterval(unsigned long interval) override {
        updateInterval = interval;
    }

    float getTemperature() const override {
        return temperature + tempOffset;
    }

    float getHumidity() const override {
        return humidity + humidityOffset;
    }

    float getPressure() const override {
        return pressure + pressureOffset;
    }

    bool isAvailable() const override {
        return lastError == ThermostatStatus::OK;
    }

    ThermostatStatus getLastError() const override {
        return lastError;
    }

    void setTemperatureOffset(float offset) override {
        tempOffset = offset;
    }

    void setHumidityOffset(float offset) override {
        humidityOffset = offset;
    }

    void setPressureOffset(float offset) override {
        pressureOffset = offset;
    }

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

    void updateReadings() {
        float temp = bme.readTemperature();
        float hum = bme.readHumidity();
        float pres = bme.readPressure() / 100.0F; // Convert Pa to hPa

        if (isnan(temp) || isnan(hum) || isnan(pres)) {
            ESP_LOGE(TAG, "Failed to read from BME280 sensor!");
            lastError = ThermostatStatus::ERROR_SENSOR_READ;
            return;
        }

        temperature = temp;
        humidity = hum;
        pressure = pres;
        lastError = ThermostatStatus::OK;

        ESP_LOGD(TAG, "Sensor readings - Temp: %.2f°C, Humidity: %.2f%%, Pressure: %.2fhPa",
                 temperature, humidity, pressure);
    }
};