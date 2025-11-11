#include "valve_health_monitor.h"
#include "logger.h"
#include <math.h>

static const char* TAG = "ValveHealth";

// Initialize static instance pointer
ValveHealthMonitor* ValveHealthMonitor::_instance = nullptr;

ValveHealthMonitor::ValveHealthMonitor()
    : _historyIndex(0),
      _historyCount(0),
      _stuckCount(0),
      _consecutiveStuckCount(0),
      _lastCommandedPosition(0.0f),
      _lastActualPosition(0.0f),
      _lastError(0.0f),
      _wasStuck(false) {
    // Initialize error history
    for (int i = 0; i < HISTORY_SIZE; i++) {
        _errorHistory[i] = 0.0f;
    }
}

ValveHealthMonitor* ValveHealthMonitor::getInstance() {
    if (_instance == nullptr) {
        _instance = new ValveHealthMonitor();
    }
    return _instance;
}

void ValveHealthMonitor::begin() {
    LOG_I(TAG, "Valve health monitor initialized");
}

void ValveHealthMonitor::recordCommand(float commanded, float actual) {
    _lastCommandedPosition = commanded;
    _lastActualPosition = actual;

    // Calculate absolute position error
    _lastError = fabs(commanded - actual);

    // Store in history
    _errorHistory[_historyIndex] = _lastError;
    _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;
    if (_historyCount < HISTORY_SIZE) {
        _historyCount++;
    }

    // Check if valve is stuck (error exceeds critical threshold)
    if (_lastError > CRITICAL_THRESHOLD) {
        _consecutiveStuckCount++;
        _stuckCount++;

        LOG_W(TAG, "Valve position error: commanded=%.1f%%, actual=%.1f%%, error=%.1f%% (consecutive=%lu)",
              commanded, actual, _lastError, _consecutiveStuckCount);

        if (_consecutiveStuckCount == STUCK_CONSECUTIVE_LIMIT) {
            LOG_E(TAG, "CRITICAL: Valve appears stuck or non-responsive (%lu consecutive errors > %.1f%%)",
                  _consecutiveStuckCount, CRITICAL_THRESHOLD);
        }
    } else if (_lastError > WARNING_THRESHOLD) {
        // Reset consecutive but log warning
        _consecutiveStuckCount = 0;
        LOG_W(TAG, "Valve position warning: commanded=%.1f%%, actual=%.1f%%, error=%.1f%%",
              commanded, actual, _lastError);
    } else {
        // Valve responding correctly
        _consecutiveStuckCount = 0;
    }
}

bool ValveHealthMonitor::isValveHealthy() const {
    // Consider unhealthy if:
    // 1. Consecutive stuck count >= limit
    // 2. Average error > warning threshold
    if (_consecutiveStuckCount >= STUCK_CONSECUTIVE_LIMIT) {
        return false;
    }

    float avgError = getAverageError();
    if (avgError > WARNING_THRESHOLD) {
        return false;
    }

    return true;
}

float ValveHealthMonitor::getAverageError() const {
    if (_historyCount == 0) {
        return 0.0f;
    }

    float sum = 0.0f;
    for (int i = 0; i < _historyCount; i++) {
        sum += _errorHistory[i];
    }

    return sum / _historyCount;
}

float ValveHealthMonitor::getMaxError() const {
    if (_historyCount == 0) {
        return 0.0f;
    }

    float maxError = 0.0f;
    for (int i = 0; i < _historyCount; i++) {
        if (_errorHistory[i] > maxError) {
            maxError = _errorHistory[i];
        }
    }

    return maxError;
}

uint32_t ValveHealthMonitor::getStuckCount() const {
    return _stuckCount;
}

uint32_t ValveHealthMonitor::getConsecutiveStuckCount() const {
    return _consecutiveStuckCount;
}

float ValveHealthMonitor::getLastCommandedPosition() const {
    return _lastCommandedPosition;
}

float ValveHealthMonitor::getLastActualPosition() const {
    return _lastActualPosition;
}

float ValveHealthMonitor::getLastError() const {
    return _lastError;
}

bool ValveHealthMonitor::hasRecovered() {
    bool isHealthyNow = isValveHealthy();

    // Check if we've recovered (was stuck, now healthy)
    if (_wasStuck && isHealthyNow) {
        LOG_I(TAG, "Valve has recovered from stuck condition");
        _wasStuck = false;
        return true;
    }

    // Update state for next check
    if (!isHealthyNow) {
        _wasStuck = true;
    }

    return false;
}
