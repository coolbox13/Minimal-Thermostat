#include "sensor_interface.h"
#include "thermostat_state.h"
#include <Wire.h>
#include <Adafruit_BME280.h>

class SensorInterfaceImpl {
public:
    Adafruit_BME280 bme;
    ThermostatState* thermostatState;
    unsigned long lastUpdateTime;
    unsigned long updateInterval;
    float temperature;
    float humidity;
    float pressure;
    bool sensorAvailable;
    float tempOffset;
    float humOffset;
    float pressOffset;
    ThermostatStatus lastError;
};

SensorInterface::SensorInterface() : pimpl(new SensorInterfaceImpl()) {
    pimpl->thermostatState = nullptr;
    pimpl->lastUpdateTime = 0;
    pimpl->updateInterval = 10000;
    pimpl->temperature = 0.0;
    pimpl->humidity = 0.0;
    pimpl->pressure = 0.0;
    pimpl->sensorAvailable = false;
    pimpl->tempOffset = 0.0;
    pimpl->humOffset = 0.0;
    pimpl->pressOffset = 0.0;
    pimpl->lastError = ThermostatStatus::OK;
}

bool SensorInterface::begin() {
    Serial.println("Initializing BME280 sensor...");
    
    // Try to initialize BME280 sensor
    if (!pimpl->bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        pimpl->sensorAvailable = false;
        pimpl->lastError = ThermostatStatus::ERROR_SENSOR;
        return false;
    }
    
    pimpl->bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                         Adafruit_BME280::SAMPLING_X1, // temperature
                         Adafruit_BME280::SAMPLING_X1, // pressure
                         Adafruit_BME280::SAMPLING_X1, // humidity
                         Adafruit_BME280::FILTER_OFF   );
    
    Serial.println("BME280 sensor initialized successfully");
    pimpl->sensorAvailable = true;
    
    // Take initial reading
    loop();
    
    return true;
}

void SensorInterface::loop() {
    unsigned long currentTime = millis();
    
    // Check if it's time to update readings
    if (currentTime - pimpl->lastUpdateTime >= pimpl->updateInterval) {
        readSensor();
        pimpl->lastUpdateTime = currentTime;
    }
}

void SensorInterface::setUpdateInterval(unsigned long interval) {
    pimpl->updateInterval = interval;
}

float SensorInterface::getTemperature() const {
    return pimpl->temperature + pimpl->tempOffset;
}

float SensorInterface::getHumidity() const {
    return pimpl->humidity + pimpl->humOffset;
}

float SensorInterface::getPressure() const {
    return pimpl->pressure + pimpl->pressOffset;
}

bool SensorInterface::isAvailable() const {
    return pimpl->sensorAvailable;
}

ThermostatStatus SensorInterface::getLastError() const {
    return pimpl->lastError;
}

void SensorInterface::setTemperatureOffset(float offset) {
    pimpl->tempOffset = offset;
}

void SensorInterface::setHumidityOffset(float offset) {
    pimpl->humOffset = offset;
}

void SensorInterface::setPressureOffset(float offset) {
    pimpl->pressOffset = offset;
}

void SensorInterface::readSensor() {
    if (!pimpl->sensorAvailable) {
        Serial.println("Sensor not available");
        return;
    }
    
    // Read sensor values
    pimpl->temperature = pimpl->bme.readTemperature();
    pimpl->humidity = pimpl->bme.readHumidity();
    pimpl->pressure = pimpl->bme.readPressure() / 100.0F; // Convert to hPa
    
    Serial.println("Sensor readings:");
    Serial.print("Temperature: ");
    Serial.print(pimpl->temperature);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(pimpl->humidity);
    Serial.println(" %");
    
    Serial.print("Pressure: ");
    Serial.print(pimpl->pressure);
    Serial.println(" hPa");
}