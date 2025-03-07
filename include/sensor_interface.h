#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "thermostat_state.h"

class SensorInterface {
public:
  // Constructor
  SensorInterface();
  
  // Initialize sensor
  bool begin(ThermostatState* state);
  
  // Update sensor readings (call periodically)
  void update();
  
  // Force an immediate reading
  void forceUpdate();
  
  // Get last readings directly
  float getTemperature() const;
  float getHumidity() const;
  float getPressure() const;
  
  // Set update interval
  void setUpdateInterval(unsigned long interval);
  
  // Check if sensor is available
  bool isAvailable() const;

private:
  // BME280 sensor instance
  Adafruit_BME280 bme;
  
  // Reference to thermostat state
  ThermostatState* thermostatState;
  
  // Timing
  unsigned long lastUpdateTime;
  unsigned long updateInterval;
  
  // Last readings
  float temperature;
  float humidity;
  float pressure;
  
  // Sensor status
  bool sensorAvailable;
  
  // Read sensor values and update thermostat state
  void readSensor();
};

#endif // SENSOR_INTERFACE_H