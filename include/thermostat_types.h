#pragma once

#include <Arduino.h>  // For float type

// Operating modes for the thermostat
enum class ThermostatMode : uint8_t {
    OFF = 0,
    HEAT = 1,
    COOL = 2,
    AUTO = 3
};

// Temperature ranges and limits
struct ThermostatLimits {
    static constexpr float MIN_TEMPERATURE = 5.0f;      // Minimum allowed temperature
    static constexpr float MAX_TEMPERATURE = 30.0f;     // Maximum allowed temperature
    static constexpr float DEFAULT_TEMPERATURE = 21.0f; // Default target temperature
    static constexpr float MIN_VALVE_POSITION = 0.0f;   // Valve fully closed
    static constexpr float MAX_VALVE_POSITION = 100.0f; // Valve fully open
    
    // Prevent instantiation
    ThermostatLimits() = delete;
};

// Status codes for operations
enum class ThermostatStatus : int8_t {
    OK = 0,                       // Operation successful
    ERROR_SENSOR = -1,           // Sensor reading error
    ERROR_COMMUNICATION = -2,    // Communication protocol error
    ERROR_CONFIGURATION = -3,    // Configuration error
    ERROR_FILESYSTEM = -4,       // File system error
    ERROR_INITIALIZATION = -5    // Initialization error
};

// Command sources for protocol interfaces
enum class CommandSource : uint8_t {
    SOURCE_NONE = 0,
    SOURCE_KNX,
    SOURCE_MQTT,
    SOURCE_WEB,
    SOURCE_WEB_API,  // Added for web API commands
    SOURCE_INTERNAL
};

// Command types for protocol interfaces
enum class CommandType : uint8_t {
    CMD_NONE = 0,
    CMD_SETPOINT,
    CMD_MODE,
    CMD_VALVE,
    CMD_HEATING,
    CMD_SET_TEMPERATURE  // Added for temperature setting commands
};

// Helper functions
inline const char* getThermostatModeName(ThermostatMode mode) {
    switch (mode) {
        case ThermostatMode::OFF: return "OFF";
        case ThermostatMode::HEAT: return "HEAT";
        case ThermostatMode::COOL: return "COOL";
        case ThermostatMode::AUTO: return "AUTO";
        default: return "UNKNOWN";
    }
}

inline const char* getThermostatStatusString(ThermostatStatus status) {
    switch (status) {
        case ThermostatStatus::OK: return "OK";
        case ThermostatStatus::ERROR_SENSOR: return "Sensor Error";
        case ThermostatStatus::ERROR_COMMUNICATION: return "Communication Error";
        case ThermostatStatus::ERROR_CONFIGURATION: return "Configuration Error";
        case ThermostatStatus::ERROR_FILESYSTEM: return "Filesystem Error";
        case ThermostatStatus::ERROR_INITIALIZATION: return "Initialization Error";
        default: return "Unknown Error";
    }
}

// Simple helper functions for JSON conversion
inline int thermostatModeToInt(ThermostatMode mode) {
    return static_cast<int>(mode);
}

inline ThermostatMode intToThermostatMode(int value) {
    return static_cast<ThermostatMode>(value);
}

inline int commandSourceToInt(CommandSource source) {
    return static_cast<int>(source);
}

inline CommandSource intToCommandSource(int value) {
    return static_cast<CommandSource>(value);
}

inline int commandTypeToInt(CommandType type) {
    return static_cast<int>(type);
}

inline CommandType intToCommandType(int value) {
    return static_cast<CommandType>(value);
}

inline int thermostatStatusToInt(ThermostatStatus status) {
    return static_cast<int>(status);
}

inline ThermostatStatus intToThermostatStatus(int value) {
    return static_cast<ThermostatStatus>(value);
} 