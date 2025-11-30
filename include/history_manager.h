/**
 * @file history_manager.h
 * @brief Circular buffer manager for historical sensor data storage
 *
 * Provides efficient storage and retrieval of time-series sensor data using
 * a fixed-size circular buffer. Designed for embedded systems with limited
 * memory where dynamic allocation should be avoided.
 *
 * @par Storage Capacity
 * The buffer stores 2880 data points, supporting:
 * - 24 hours at 30-second intervals
 * - 48 hours at 1-minute intervals
 * - 10 days at 5-minute intervals
 *
 * @par Memory Usage
 * Each HistoryDataPoint is approximately 18 bytes:
 * - time_t timestamp: 4 bytes
 * - float temperature: 4 bytes
 * - float humidity: 4 bytes
 * - float pressure: 4 bytes
 * - uint8_t valvePosition: 1 byte + padding
 *
 * Total buffer: ~52KB statically allocated
 *
 * @par JSON Serialization
 * The getHistoryJson() methods support writing directly to AsyncJsonResponse
 * buffers to avoid double-buffering and heap fragmentation issues.
 *
 * @see WebServerManager for the /api/history endpoint implementation
 */

#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>

/**
 * @struct HistoryDataPoint
 * @brief Single data point containing timestamped sensor readings
 *
 * Stores a snapshot of all sensor values at a given point in time.
 * Used as the element type for the circular buffer.
 */
struct HistoryDataPoint {
    time_t timestamp;       ///< Unix timestamp (seconds since epoch) or millis/1000 as fallback
    float temperature;      ///< Temperature reading in degrees Celsius
    float humidity;         ///< Relative humidity as percentage (0-100)
    float pressure;         ///< Atmospheric pressure in hectopascals (hPa)
    uint8_t valvePosition;  ///< Valve opening position as percentage (0-100)
};

/**
 * @class HistoryManager
 * @brief Singleton manager for storing and retrieving historical sensor data
 *
 * Implements a circular buffer for efficient storage of time-series data
 * without dynamic memory allocation. When the buffer is full, oldest entries
 * are automatically overwritten.
 *
 * @par Thread Safety
 * Currently not thread-safe. All access should be from the main loop context.
 * If called from async web handlers, ensure proper synchronization.
 *
 * @par Usage Example
 * @code
 * // Add a new reading
 * HistoryManager::getInstance()->addDataPoint(21.5, 55.0, 1013.25, 45);
 *
 * // Get data as JSON (for AsyncJsonResponse)
 * JsonObject obj = response->getRoot().as<JsonObject>();
 * HistoryManager::getInstance()->getHistoryJson(obj, 200);
 * @endcode
 */
class HistoryManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Pointer to the HistoryManager instance (created on first call)
     */
    static HistoryManager* getInstance();

    /**
     * @brief Add a new data point to the history buffer
     *
     * Stores sensor readings with current timestamp. Uses NTP time if available,
     * otherwise falls back to millis()/1000. Oldest data is overwritten when
     * buffer is full.
     *
     * @param temperature Temperature reading in degrees Celsius
     * @param humidity Relative humidity as percentage (0-100)
     * @param pressure Atmospheric pressure in hectopascals (hPa)
     * @param valvePosition Valve opening position as percentage (0-100)
     *
     * @note Called from main loop at configured intervals (typically 30 seconds)
     */
    void addDataPoint(float temperature, float humidity, float pressure, uint8_t valvePosition);

    /**
     * @brief Get historical data as JSON document
     *
     * Populates a JsonDocument with arrays of historical data. Useful when
     * the caller manages their own JsonDocument.
     *
     * @param doc JSON document to populate (will be converted to object)
     * @param maxPoints Maximum number of points to return (0 = all available)
     *
     * @deprecated Prefer the JsonObject overload to avoid double-buffering
     * @see getHistoryJson(JsonObject&, int) for memory-efficient version
     */
    void getHistoryJson(JsonDocument& doc, int maxPoints = 0);

    /**
     * @brief Get historical data as JSON (memory-efficient overload)
     *
     * Writes directly to an existing JsonObject, enabling zero-copy serialization
     * when used with AsyncJsonResponse. This is the preferred method for the
     * web API to avoid heap fragmentation.
     *
     * @param obj JSON object to populate with data arrays
     * @param maxPoints Maximum number of points to return (0 = all available)
     *
     * @par Output Format
     * @code
     * {
     *   "timestamps": [1234567890, ...],
     *   "temperatures": [21.5, ...],
     *   "humidities": [55.0, ...],
     *   "pressures": [1013.25, ...],
     *   "valvePositions": [45, ...],
     *   "count": 200,
     *   "maxSize": 2880
     * }
     * @endcode
     *
     * @note Points are returned in chronological order (oldest first)
     * @note If maxPoints < available data, points are evenly sampled
     */
    void getHistoryJson(JsonObject& obj, int maxPoints = 0);

    /**
     * @brief Get the number of stored data points
     * @return Number of valid data points in buffer (0 to BUFFER_SIZE)
     */
    int getDataPointCount();

    /**
     * @brief Clear all historical data
     *
     * Resets the buffer to empty state. Use after configuration changes
     * or when starting a new monitoring session.
     */
    void clear();

private:
    /** @brief Private constructor for singleton pattern */
    HistoryManager();

    static HistoryManager* _instance;  ///< Singleton instance pointer

    /** @brief Maximum number of data points stored (24h at 30s intervals) */
    static const int BUFFER_SIZE = 2880;

    HistoryDataPoint _buffer[BUFFER_SIZE];  ///< Circular buffer storage
    int _head;   ///< Next write position (0 to BUFFER_SIZE-1)
    int _count;  ///< Number of valid entries (0 to BUFFER_SIZE)
};

#endif // HISTORY_MANAGER_H
