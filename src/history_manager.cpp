#include "history_manager.h"
#include "ntp_manager.h"
#include "logger.h"
#include <ArduinoJson.h>

static const char* TAG = "HISTORY";

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
    int numPoints = _count;
    if (maxPoints > 0 && maxPoints < numPoints) {
        numPoints = maxPoints;
    }

    JsonArray timestamps = doc.createNestedArray("timestamps");
    JsonArray temperatures = doc.createNestedArray("temperatures");
    JsonArray humidities = doc.createNestedArray("humidities");
    JsonArray pressures = doc.createNestedArray("pressures");
    JsonArray valvePositions = doc.createNestedArray("valvePositions");

    // Calculate start index (oldest point to include)
    int startIdx;
    if (_count < BUFFER_SIZE) {
        // Buffer not full yet, start from beginning
        startIdx = 0;
    } else {
        // Buffer is full, start from head (oldest)
        startIdx = _head;
    }

    // Calculate how many points to skip if maxPoints is set
    int skip = 1;
    if (maxPoints > 0 && numPoints > maxPoints) {
        skip = numPoints / maxPoints;
    }

    int pointsAdded = 0;
    for (int i = 0; i < numPoints && pointsAdded < numPoints; i += skip) {
        int idx = (startIdx + i) % BUFFER_SIZE;
        if (idx < _count || _count == BUFFER_SIZE) {
            timestamps.add(_buffer[idx].timestamp);
            temperatures.add(_buffer[idx].temperature);
            humidities.add(_buffer[idx].humidity);
            pressures.add(_buffer[idx].pressure);
            valvePositions.add(_buffer[idx].valvePosition);
            pointsAdded++;
        }
    }

    doc["count"] = pointsAdded;
    doc["maxSize"] = BUFFER_SIZE;

    LOG_D(TAG, "Returning %d data points", pointsAdded);
}

int HistoryManager::getDataPointCount() {
    return _count;
}

void HistoryManager::clear() {
    _head = 0;
    _count = 0;
    LOG_I(TAG, "History cleared");
}
