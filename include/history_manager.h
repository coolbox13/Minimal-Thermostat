#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>

/**
 * @brief Data point for storing sensor readings and valve position
 */
struct HistoryDataPoint {
    time_t timestamp;  // Unix timestamp (seconds since epoch) or millis/1000 as fallback
    float temperature;
    float humidity;
    float pressure;
    uint8_t valvePosition;
};

/**
 * @brief Manager for storing and retrieving historical sensor data
 *
 * Uses a circular buffer to store the last N readings (24 hours worth).
 * Stores one reading every 5 minutes = 12 per hour = 288 per 24 hours.
 */
class HistoryManager {
public:
    static HistoryManager* getInstance();

    /**
     * @brief Add a new data point to the history
     * @param temperature Temperature in °C
     * @param humidity Humidity in %
     * @param pressure Pressure in hPa
     * @param valvePosition Valve position 0-100%
     */
    void addDataPoint(float temperature, float humidity, float pressure, uint8_t valvePosition);

    /**
     * @brief Get historical data as JSON
     * @param doc JSON document to populate
     * @param maxPoints Maximum number of points to return (0 = all)
     */
    void getHistoryJson(JsonDocument& doc, int maxPoints = 0);

    /**
     * @brief Get the number of stored data points
     * @return Number of data points in buffer
     */
    int getDataPointCount();

    /**
     * @brief Clear all historical data
     */
    void clear();

private:
    HistoryManager();
    static HistoryManager* _instance;

    static const int BUFFER_SIZE = 288;  // 24 hours * 12 readings/hour (5min intervals)
    HistoryDataPoint _buffer[BUFFER_SIZE];
    int _head;       // Next write position
    int _count;      // Number of valid entries
};

#endif // HISTORY_MANAGER_H
