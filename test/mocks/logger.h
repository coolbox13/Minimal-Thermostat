#ifndef MOCK_LOGGER_H
#define MOCK_LOGGER_H

#include "Arduino.h"
#include <stdarg.h>

/**
 * Mock logger for testing
 * Provides minimal logging functionality without actual output
 */

enum LogLevel {
    LOG_NONE = 0,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_VERBOSE
};

class Logger {
private:
    LogLevel _logLevel;
    int _messageCount;

    Logger() : _logLevel(LOG_INFO), _messageCount(0) {}

public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        _logLevel = level;
    }

    LogLevel getLogLevel() const {
        return _logLevel;
    }

    void log(LogLevel level, const char* tag, const char* format, ...) {
        if (level > _logLevel) return;
        _messageCount++;
        // Mock: do nothing, just count messages
    }

    // Test utility
    int getMockMessageCount() const {
        return _messageCount;
    }

    void resetMockMessageCount() {
        _messageCount = 0;
    }
};

// Convenience macros matching the real logger
#define LOG_E(tag, format, ...) Logger::getInstance().log(LOG_ERROR, tag, format, ##__VA_ARGS__)
#define LOG_W(tag, format, ...) Logger::getInstance().log(LOG_WARNING, tag, format, ##__VA_ARGS__)
#define LOG_I(tag, format, ...) Logger::getInstance().log(LOG_INFO, tag, format, ##__VA_ARGS__)
#define LOG_D(tag, format, ...) Logger::getInstance().log(LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define LOG_V(tag, format, ...) Logger::getInstance().log(LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#endif // MOCK_LOGGER_H
