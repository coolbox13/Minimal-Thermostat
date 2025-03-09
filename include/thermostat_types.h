#ifndef THERMOSTAT_TYPES_H
#define THERMOSTAT_TYPES_H

#include <Arduino.h>  // For float type

// Operating modes for the thermostat
enum class ThermostatMode : uint8_t {
    OFF = 0,        // System is off
    COMFORT = 1,    // Normal comfort mode
    STANDBY = 2,    // Reduced temperature mode
    ECONOMY = 3,    // Energy saving mode
    PROTECTION = 4  // Building protection mode (frost/heat)
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
    OK = 0,                    // Operation successful
    ERROR_SENSOR = -1,         // Sensor reading error
    ERROR_COMMUNICATION = -2,  // Communication protocol error
    ERROR_CONFIGURATION = -3,  // Configuration error
    ERROR_FILESYSTEM = -4      // File system error
};

// Helper functions
inline const char* getThermostatModeName(ThermostatMode mode) {
    switch (mode) {
        case ThermostatMode::OFF: return "Off";
        case ThermostatMode::COMFORT: return "Comfort";
        case ThermostatMode::STANDBY: return "Standby";
        case ThermostatMode::ECONOMY: return "Economy";
        case ThermostatMode::PROTECTION: return "Protection";
        default: return "Unknown";
    }
}

inline const char* getThermostatStatusString(ThermostatStatus status) {
    switch (status) {
        case ThermostatStatus::OK: return "OK";
        case ThermostatStatus::ERROR_SENSOR: return "Sensor Error";
        case ThermostatStatus::ERROR_COMMUNICATION: return "Communication Error";
        case ThermostatStatus::ERROR_CONFIGURATION: return "Configuration Error";
        case ThermostatStatus::ERROR_FILESYSTEM: return "Filesystem Error";
        default: return "Unknown Error";
    }
}

#endif // THERMOSTAT_TYPES_H 