#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum LogLevel {
    LOG_NONE = 0,   // No logging
    LOG_ERROR,      // Critical errors
    LOG_WARNING,    // Warnings
    LOG_INFO,       // Informational messages
    LOG_DEBUG,      // Debug-level messages
    LOG_VERBOSE     // Verbose debug messages
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance; // Guaranteed to be destroyed, instantiated on first use
        return instance;
    }

    // Set global log level
    void setLogLevel(LogLevel level) {
        _logLevel = level;
    }

    // Get current log level
    LogLevel getLogLevel() const {
        return _logLevel;
    }

    // Log a message with specified level
    void log(LogLevel level, const char* tag, const char* format, ...) {
        if (level > _logLevel) return; // Skip if level is higher than current log level

        // Get current timestamp
        unsigned long timestamp = millis();
        
        // Print timestamp and log level
        Serial.print(timestamp);
        Serial.print(" | ");
        
        // Print log level
        switch (level) {
            case LOG_ERROR:
                Serial.print("ERROR");
                break;
            case LOG_WARNING:
                Serial.print("WARN ");
                break;
            case LOG_INFO:
                Serial.print("INFO ");
                break;
            case LOG_DEBUG:
                Serial.print("DEBUG");
                break;
            case LOG_VERBOSE:
                Serial.print("VERB ");
                break;
            default:
                Serial.print("?????");
                break;
        }
        
        // Print tag
        Serial.print(" | ");
        Serial.print(tag);
        Serial.print(" | ");
        
        // Print formatted message
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        Serial.println(buffer);
        
        // If we have a log callback registered, call it
        if (_logCallback) {
            _logCallback(level, tag, buffer, timestamp);
        }
    }

    // Simplified log methods for common use
    void error(const char* tag, const char* format, ...) {
        if (LOG_ERROR > _logLevel) return;
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(LOG_ERROR, tag, buffer);
    }
    
    void warning(const char* tag, const char* format, ...) {
        if (LOG_WARNING > _logLevel) return;
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(LOG_WARNING, tag, buffer);
    }
    
    void info(const char* tag, const char* format, ...) {
        if (LOG_INFO > _logLevel) return;
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(LOG_INFO, tag, buffer);
    }
    
    void debug(const char* tag, const char* format, ...) {
        if (LOG_DEBUG > _logLevel) return;
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(LOG_DEBUG, tag, buffer);
    }
    
    void verbose(const char* tag, const char* format, ...) {
        if (LOG_VERBOSE > _logLevel) return;
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(LOG_VERBOSE, tag, buffer);
    }

    // Register a callback for logs (useful for storing logs or sending to a server)
    typedef void (*LogCallback)(LogLevel level, const char* tag, const char* message, unsigned long timestamp);
    void setLogCallback(LogCallback callback) {
        _logCallback = callback;
    }
    
private:
    // Private constructor for singleton
    Logger() : _logLevel(LOG_INFO), _logCallback(nullptr) {}
    
    // Prevent copy/move
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    LogLevel _logLevel;
    LogCallback _logCallback;
};

// Convenience macros for logging
#define LOG_E(tag, ...) Logger::getInstance().error(tag, __VA_ARGS__)
#define LOG_W(tag, ...) Logger::getInstance().warning(tag, __VA_ARGS__)
#define LOG_I(tag, ...) Logger::getInstance().info(tag, __VA_ARGS__)
#define LOG_D(tag, ...) Logger::getInstance().debug(tag, __VA_ARGS__)
#define LOG_V(tag, ...) Logger::getInstance().verbose(tag, __VA_ARGS__)

#define TAG_WIFI "WIFI"
#define TAG_WATCHDOG "WDOG"
#define TAG_CONNECTIVITY "CONN"

#endif // LOGGER_H