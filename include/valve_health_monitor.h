#ifndef VALVE_HEALTH_MONITOR_H
#define VALVE_HEALTH_MONITOR_H

#include <Arduino.h>

/**
 * @brief Monitors valve actuator health by comparing commanded vs actual position
 *
 * Tracks valve response to detect:
 * - Stuck valves (commanded position != actual position)
 * - Position deviation trends
 * - Actuator failures
 */
class ValveHealthMonitor {
public:
    /**
     * @brief Get singleton instance
     */
    static ValveHealthMonitor* getInstance();

    /**
     * @brief Initialize the valve health monitor
     */
    void begin();

    /**
     * @brief Record a valve command and actual feedback
     * @param commanded The commanded valve position (0-100%)
     * @param actual The actual valve position from feedback (0-100%)
     */
    void recordCommand(float commanded, float actual);

    /**
     * @brief Check if valve is responding correctly
     * @return true if valve is healthy
     */
    bool isValveHealthy() const;

    /**
     * @brief Get average position error over history
     * @return Average error in percentage points
     */
    float getAverageError() const;

    /**
     * @brief Get maximum position error over history
     * @return Maximum error in percentage points
     */
    float getMaxError() const;

    /**
     * @brief Get number of times valve appeared stuck
     * @return Count of stuck events
     */
    uint32_t getStuckCount() const;

    /**
     * @brief Get number of consecutive stuck events
     * @return Consecutive stuck count
     */
    uint32_t getConsecutiveStuckCount() const;

    /**
     * @brief Get the last commanded position
     * @return Last commanded position (0-100%)
     */
    float getLastCommandedPosition() const;

    /**
     * @brief Get the last actual position
     * @return Last actual position from feedback (0-100%)
     */
    float getLastActualPosition() const;

    /**
     * @brief Get the last position error
     * @return Last error in percentage points
     */
    float getLastError() const;

    /**
     * @brief Check if valve has recovered from stuck condition
     * @return true if valve was stuck but is now responding
     */
    bool hasRecovered();

private:
    ValveHealthMonitor();
    static ValveHealthMonitor* _instance;

    // Error history (last 100 commands)
    static const int VALVE_HISTORY_SIZE = 100;
    float _errorHistory[VALVE_HISTORY_SIZE];
    int _historyIndex;
    int _historyCount;

    // Statistics
    uint32_t _stuckCount;
    uint32_t _consecutiveStuckCount;
    float _lastCommandedPosition;
    float _lastActualPosition;
    float _lastError;

    // Recovery tracking
    bool _wasStuck;

    // Thresholds
    static constexpr float WARNING_THRESHOLD = 10.0f;  // 10% deviation = warning
    static constexpr float CRITICAL_THRESHOLD = 20.0f; // 20% deviation = critical
    static const uint32_t STUCK_CONSECUTIVE_LIMIT = 5; // 5 consecutive = stuck
};

#endif // VALVE_HEALTH_MONITOR_H
