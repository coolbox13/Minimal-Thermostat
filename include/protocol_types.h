#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

#include <cstdint>
#include "thermostat_types.h"

// Command source for incoming commands
enum class CommandSource : uint8_t {
    SOURCE_NONE = 0,
    SOURCE_KNX,
    SOURCE_MQTT,
    SOURCE_WEB_API,
    SOURCE_INTERNAL
};

// Command type for incoming commands
enum class CommandType : uint8_t {
    CMD_NONE = 0,
    CMD_SET_TEMPERATURE,
    CMD_SET_MODE,
    CMD_SET_VALVE_POSITION,
    CMD_SET_HEATING_STATE
};

// Forward declare ThermostatStatus from thermostat_types.h
#include "thermostat_types.h"

// Protocol status codes
enum class ProtocolStatus : int8_t {
    OK = 0,                     // Operation successful
    ERROR_NOT_CONNECTED = -1,   // Protocol not connected
    ERROR_TIMEOUT = -2,         // Operation timed out
    ERROR_INVALID_DATA = -3,    // Invalid data received
    ERROR_AUTHENTICATION = -4   // Authentication failed
};

// Helper functions
inline const char* getCommandSourceName(CommandSource source) {
    switch (source) {
        case CommandSource::SOURCE_KNX: return "KNX";
        case CommandSource::SOURCE_MQTT: return "MQTT";
        case CommandSource::SOURCE_WEB_API: return "Web API";
        case CommandSource::SOURCE_INTERNAL: return "Internal";
        case CommandSource::SOURCE_NONE: return "None";
        default: return "Unknown";
    }
}

inline const char* getCommandTypeName(CommandType cmd) {
    switch (cmd) {
        case CommandType::CMD_NONE: return "None";
        case CommandType::CMD_SETPOINT: return "Set Setpoint";
        case CommandType::CMD_MODE: return "Set Mode";
        case CommandType::CMD_VALVE: return "Set Valve Position";
        case CommandType::CMD_HEATING: return "Set Heating State";
        default: return "Unknown";
    }
}

inline const char* getProtocolStatusString(ProtocolStatus status) {
    switch (status) {
        case ProtocolStatus::OK: return "OK";
        case ProtocolStatus::ERROR_NOT_CONNECTED: return "Not Connected";
        case ProtocolStatus::ERROR_TIMEOUT: return "Timeout";
        case ProtocolStatus::ERROR_INVALID_DATA: return "Invalid Data";
        case ProtocolStatus::ERROR_AUTHENTICATION: return "Authentication Failed";
        default: return "Unknown Error";
    }
}

#endif // PROTOCOL_TYPES_H 