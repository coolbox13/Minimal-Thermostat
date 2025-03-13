#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <Adafruit_BME280.h>

class BME280Sensor {
public:
    BME280Sensor();
    
    bool begin();
    float readTemperature();
    float readHumidity();
    float readPressure();
    
private:
    Adafruit_BME280 bme;
    bool initialized;
};

#endif // BME280_SENSOR_H