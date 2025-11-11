#ifndef SENSOR_HEALTH_MONITOR_H
#define SENSOR_HEALTH_MONITOR_H

#include <Arduino.h>

/**
 * @brief Monitors sensor health and provides failure detection
 *
 * Tracks sensor reading history to detect:
 * - Consecutive failures (immediate alerts)
 * - Failure rate over time (degradation detection)
 * - Last known good values (fallback data)
 */
class SensorHealthMonitor {
public:
    /**
     * @brief Get singleton instance
     */
    static SensorHealthMonitor* getInstance();

    /**
     * @brief Initialize the health monitor
     */
    void begin();

    /**
     * @brief Record a sensor reading attempt
     * @param isValid true if reading was successful, false if failed
     * @param value The sensor value (only stored if valid)
     */
    void recordReading(bool isValid, float value);

    /**
     * @brief Check if sensor is currently healthy
     * @return true if sensor is responding correctly
     */
    bool isSensorHealthy() const;

    /**
     * @brief Get number of consecutive failures
     * @return Count of consecutive failed readings
     */
    uint32_t getConsecutiveFailures() const;

    /**
     * @brief Get timestamp of last successful reading
     * @return millis() timestamp of last good reading
     */
    unsigned long getLastGoodReadingTime() const;

    /**
     * @brief Get the last known good sensor value
     * @return Last valid temperature reading
     */
    float getLastGoodValue() const;

    /**
     * @brief Get failure rate over recent history
     * @return Percentage of failed readings (0-100)
     */
    float getFailureRate() const;

    /**
     * @brief Get total number of readings recorded
     * @return Total reading count
     */
    uint32_t getTotalReadings() const;

    /**
     * @brief Get total number of failed readings
     * @return Failed reading count
     */
    uint32_t getFailedReadings() const;

    /**
     * @brief Check if sensor has recovered from failure
     * @return true if sensor was failing but is now healthy
     */
    bool hasRecovered();

private:
    SensorHealthMonitor();
    static SensorHealthMonitor* _instance;

    uint32_t _consecutiveFailures;
    uint32_t _totalReadings;
    uint32_t _failedReadings;
    unsigned long _lastGoodReadingTime;
    float _lastGoodValue;

    // Circular buffer for failure rate calculation (5 minutes at 1 reading/sec)
    static const int HISTORY_SIZE = 300;
    bool _readingHistory[HISTORY_SIZE];
    int _historyIndex;
    int _historyCount; // Number of valid entries in history

    // Recovery tracking
    bool _wasUnhealthy;
};

#endif // SENSOR_HEALTH_MONITOR_H
