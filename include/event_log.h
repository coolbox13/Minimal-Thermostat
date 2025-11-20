#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>
#include <ArduinoJson.h>
#include "logger.h"

/**
 * @brief Structure representing a single log entry
 */
struct LogEntry {
    unsigned long timestamp;  // Milliseconds since boot
    LogLevel level;
    String tag;
    String message;

    LogEntry() : timestamp(0), level(LOG_INFO) {}

    LogEntry(unsigned long ts, LogLevel lvl, const char* t, const char* msg)
        : timestamp(ts), level(lvl), tag(t), message(msg) {}
};

/**
 * @brief EventLog class for persistent logging
 *
 * Stores important events (errors, warnings, info) in LittleFS for troubleshooting.
 * Implements a circular buffer approach with configurable maximum entries.
 * Also publishes logs to MQTT if enabled.
 */
class EventLog {
public:
    /**
     * @brief Get singleton instance
     */
    static EventLog& getInstance();

    /**
     * @brief Initialize the event log system
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Add a log entry
     * @param level Log level
     * @param tag Log tag/category
     * @param message Log message
     */
    void addEntry(LogLevel level, const char* tag, const char* message);

    /**
     * @brief Get all log entries as JSON string
     * @return JSON string containing all log entries
     */
    String getEntriesJSON();

    /**
     * @brief Get filtered log entries
     * @param minLevel Minimum log level to include
     * @param tag Optional tag filter (empty = all tags)
     * @return JSON string containing filtered log entries
     */
    String getFilteredEntriesJSON(LogLevel minLevel = LOG_INFO, const char* tag = nullptr);

    /**
     * @brief Clear all log entries
     */
    void clear();

    /**
     * @brief Get number of log entries
     */
    size_t getCount() const;

    /**
     * @brief Enable/disable MQTT logging
     * @param enabled True to enable MQTT logging
     */
    void setMQTTLoggingEnabled(bool enabled);

    /**
     * @brief Check if MQTT logging is enabled
     */
    bool isMQTTLoggingEnabled() const;

    /**
     * @brief Set MQTT publish callback
     * @param callback Function to call for MQTT publishing
     */
    void setMQTTCallback(std::function<void(LogLevel, const char*, const char*)> callback);

    /**
     * @brief Convert LogLevel to string
     */
    static const char* logLevelToString(LogLevel level);

private:
    EventLog();
    ~EventLog();

    // Prevent copying
    EventLog(const EventLog&) = delete;
    EventLog& operator=(const EventLog&) = delete;

    /**
     * @brief Load log entries from LittleFS
     */
    bool loadFromLittleFS();

    /**
     * @brief Save log entries to LittleFS
     */
    bool saveToLittleFS();

    /**
     * @brief Publish log entry to MQTT
     */
    void publishToMQTT(LogLevel level, const char* tag, const char* message);

    static const int MAX_ENTRIES = 100;           // Maximum number of log entries to store
    static constexpr const char* LOG_FILE = "/event_log.json";  // LittleFS file path

    std::vector<LogEntry> _entries;               // In-memory log entries
    bool _mqttLoggingEnabled;                     // MQTT logging enabled flag
    std::function<void(LogLevel, const char*, const char*)> _mqttCallback;  // MQTT callback
};

#endif // EVENT_LOG_H
