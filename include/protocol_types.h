#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

#include <Arduino.h>  // For uint8_t type

// Command sources - identifies where a command originated from
enum class CommandSource : uint8_t {
    SOURCE_KNX = 0,      // Command from KNX bus
    SOURCE_MQTT = 1,     // Command from MQTT broker
    SOURCE_WEB_API = 2,  // Command from web interface
    SOURCE_INTERNAL = 3  // Command from internal logic
};

// Command types - identifies what type of command was received
enum class CommandType : uint8_t {
    CMD_SET_TEMPERATURE = 0,  // Set target temperature
    CMD_SET_MODE = 1,         // Set thermostat mode
    CMD_SET_VALVE = 2         // Set valve position directly
};

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
        default: return "Unknown";
    }
}

inline const char* getCommandTypeName(CommandType cmd) {
    switch (cmd) {
        case CommandType::CMD_SET_TEMPERATURE: return "Set Temperature";
        case CommandType::CMD_SET_MODE: return "Set Mode";
        case CommandType::CMD_SET_VALVE: return "Set Valve";
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