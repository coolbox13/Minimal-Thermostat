/**
 * @file history_manager.cpp
 * @brief Implementation of circular buffer history storage
 *
 * @see history_manager.h for class documentation
 */

#include "history_manager.h"
#include "ntp_manager.h"
#include "logger.h"
#include <ArduinoJson.h>

/// @brief Log tag for history manager messages
static const char* TAG = "HISTORY";

const int HistoryManager::BUFFER_SIZE;
HistoryManager* HistoryManager::_instance = nullptr;

HistoryManager* HistoryManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new HistoryManager();
    }
    return _instance;
}

HistoryManager::HistoryManager() : _head(0), _count(0) {
    LOG_I(TAG, "History manager initialized (buffer size: %d)", BUFFER_SIZE);
}

void HistoryManager::addDataPoint(float temperature, float humidity, float pressure, uint8_t valvePosition) {
    // Use actual time if available, otherwise fall back to millis
    NTPManager& ntpManager = NTPManager::getInstance();
    time_t currentTime = ntpManager.getCurrentTime();
    _buffer[_head].timestamp = (currentTime > 0) ? currentTime : (millis() / 1000);

    _buffer[_head].temperature = temperature;
    _buffer[_head].humidity = humidity;
    _buffer[_head].pressure = pressure;
    _buffer[_head].valvePosition = valvePosition;

    _head = (_head + 1) % BUFFER_SIZE;

    if (_count < BUFFER_SIZE) {
        _count++;
    }

    LOG_D(TAG, "Data point added: T=%.1f°C H=%.1f%% P=%.1fhPa V=%d%% (count=%d)",
          temperature, humidity, pressure, valvePosition, _count);
}

void HistoryManager::getHistoryJson(JsonDocument& doc, int maxPoints) {
    // Delegate to JsonObject overload
    JsonObject obj = doc.to<JsonObject>();
    getHistoryJson(obj, maxPoints);
}

void HistoryManager::getHistoryJson(JsonObject& obj, int maxPoints) {
    JsonArray timestamps = obj.createNestedArray("timestamps");
    JsonArray temperatures = obj.createNestedArray("temperatures");
    JsonArray humidities = obj.createNestedArray("humidities");
    JsonArray pressures = obj.createNestedArray("pressures");
    JsonArray valvePositions = obj.createNestedArray("valvePositions");

    // Calculate start index (oldest point in buffer)
    int startIdx;
    if (_count < BUFFER_SIZE) {
        // Buffer not full yet, start from beginning
        startIdx = 0;
    } else {
        // Buffer is full, start from head (oldest)
        startIdx = _head;
    }

    // Determine how many points to return
    int numPoints = (maxPoints > 0 && maxPoints < _count) ? maxPoints : _count;

    // Calculate skip factor using floating point for even distribution
    // This ensures we cover the full range from oldest to newest
    float step = (_count > numPoints) ? (float)(_count - 1) / (numPoints - 1) : 1.0f;

    int pointsAdded = 0;
    for (int i = 0; i < numPoints; i++) {
        // Calculate index using floating point step to evenly distribute
        int dataIdx = (int)(i * step + 0.5f);  // Round to nearest
        if (dataIdx >= _count) dataIdx = _count - 1;  // Clamp to valid range

        int idx = (startIdx + dataIdx) % BUFFER_SIZE;
        timestamps.add(_buffer[idx].timestamp);
        temperatures.add(_buffer[idx].temperature);
        humidities.add(_buffer[idx].humidity);
        pressures.add(_buffer[idx].pressure);
        valvePositions.add(_buffer[idx].valvePosition);
        pointsAdded++;
    }

    obj["count"] = pointsAdded;
    obj["maxSize"] = BUFFER_SIZE;
    obj["totalStored"] = _count;  // Add total stored for transparency

    LOG_D(TAG, "Returning %d of %d data points (step=%.2f)", pointsAdded, _count, step);
}

int HistoryManager::getDataPointCount() {
    return _count;
}

void HistoryManager::clear() {
    _head = 0;
    _count = 0;
    LOG_I(TAG, "History cleared");
}
