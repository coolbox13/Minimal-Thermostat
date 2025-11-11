#include "sensor_health_monitor.h"
#include "logger.h"

static const char* TAG = "SensorHealth";

// Initialize static instance pointer
SensorHealthMonitor* SensorHealthMonitor::_instance = nullptr;

SensorHealthMonitor::SensorHealthMonitor()
    : _consecutiveFailures(0),
      _totalReadings(0),
      _failedReadings(0),
      _lastGoodReadingTime(0),
      _lastGoodValue(NAN),
      _historyIndex(0),
      _historyCount(0),
      _wasUnhealthy(false) {
    // Initialize history buffer
    for (int i = 0; i < HISTORY_SIZE; i++) {
        _readingHistory[i] = true; // Start optimistic
    }
}

SensorHealthMonitor* SensorHealthMonitor::getInstance() {
    if (_instance == nullptr) {
        _instance = new SensorHealthMonitor();
    }
    return _instance;
}

void SensorHealthMonitor::begin() {
    LOG_I(TAG, "Sensor health monitor initialized");
}

void SensorHealthMonitor::recordReading(bool isValid, float value) {
    _totalReadings++;

    // Update circular buffer for failure rate
    _readingHistory[_historyIndex] = isValid;
    _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;
    if (_historyCount < HISTORY_SIZE) {
        _historyCount++;
    }

    if (isValid) {
        // Successful reading
        _consecutiveFailures = 0;
        _lastGoodReadingTime = millis();
        _lastGoodValue = value;
    } else {
        // Failed reading
        _consecutiveFailures++;
        _failedReadings++;

        LOG_W(TAG, "Sensor reading failed (consecutive: %lu, total failures: %lu/%lu)",
              _consecutiveFailures, _failedReadings, _totalReadings);
    }
}

bool SensorHealthMonitor::isSensorHealthy() const {
    // Consider unhealthy if:
    // 1. More than 3 consecutive failures
    // 2. Failure rate > 50% over recent history
    if (_consecutiveFailures >= 3) {
        return false;
    }

    float failureRate = getFailureRate();
    if (failureRate > 50.0f) {
        return false;
    }

    return true;
}

uint32_t SensorHealthMonitor::getConsecutiveFailures() const {
    return _consecutiveFailures;
}

unsigned long SensorHealthMonitor::getLastGoodReadingTime() const {
    return _lastGoodReadingTime;
}

float SensorHealthMonitor::getLastGoodValue() const {
    return _lastGoodValue;
}

float SensorHealthMonitor::getFailureRate() const {
    if (_historyCount == 0) {
        return 0.0f;
    }

    int failureCount = 0;
    for (int i = 0; i < _historyCount; i++) {
        if (!_readingHistory[i]) {
            failureCount++;
        }
    }

    return (failureCount * 100.0f) / _historyCount;
}

uint32_t SensorHealthMonitor::getTotalReadings() const {
    return _totalReadings;
}

uint32_t SensorHealthMonitor::getFailedReadings() const {
    return _failedReadings;
}

bool SensorHealthMonitor::hasRecovered() {
    bool isHealthyNow = isSensorHealthy();

    // Check if we've recovered (was unhealthy, now healthy)
    if (_wasUnhealthy && isHealthyNow) {
        _wasUnhealthy = false;
        return true;
    }

    // Update state for next check
    if (!isHealthyNow) {
        _wasUnhealthy = true;
    }

    return false;
}
