#pragma once

#include <Arduino.h>  // For float type
#include <ArduinoJson.h>

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
    OK = 0,
    ERROR_CONFIGURATION,
    ERROR_COMMUNICATION,
    ERROR_SENSOR_READ,
    ERROR_CONTROL,
    ERROR_STORAGE
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
        case ThermostatStatus::OK:
            return "OK";
        case ThermostatStatus::ERROR_CONFIGURATION:
            return "Configuration Error";
        case ThermostatStatus::ERROR_COMMUNICATION:
            return "Communication Error";
        case ThermostatStatus::ERROR_SENSOR_READ:
            return "Sensor Read Error";
        case ThermostatStatus::ERROR_CONTROL:
            return "Control Error";
        case ThermostatStatus::ERROR_STORAGE:
            return "Storage Error";
        default:
            return "Unknown Error";
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

// Add JSON serialization support
namespace ArduinoJson {
    namespace V731PB22 {
        template<>
        struct Converter<ThermostatMode> {
            static void toJson(const ThermostatMode& src, JsonVariant dst) {
                switch (src) {
                    case ThermostatMode::OFF: dst.set("OFF"); break;
                    case ThermostatMode::HEAT: dst.set("HEAT"); break;
                    case ThermostatMode::COOL: dst.set("COOL"); break;
                    case ThermostatMode::AUTO: dst.set("AUTO"); break;
                }
            }
        };

        template<>
        struct Converter<ThermostatStatus> {
            static void toJson(const ThermostatStatus& src, JsonVariant dst) {
                switch (src) {
                    case ThermostatStatus::OK: dst.set("OK"); break;
                    case ThermostatStatus::ERROR_CONFIGURATION: dst.set("Configuration Error"); break;
                    case ThermostatStatus::ERROR_COMMUNICATION: dst.set("Communication Error"); break;
                    case ThermostatStatus::ERROR_SENSOR_READ: dst.set("Sensor Read Error"); break;
                    case ThermostatStatus::ERROR_CONTROL: dst.set("Control Error"); break;
                    case ThermostatStatus::ERROR_STORAGE: dst.set("Storage Error"); break;
                }
            }
        };
    }
} 