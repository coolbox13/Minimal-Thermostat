#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

// Command sources
enum CommandSource {
    SOURCE_KNX = 0,
    SOURCE_MQTT = 1,
    SOURCE_WEB_API = 2,
    SOURCE_INTERNAL = 3
};

// Command types
enum CommandType {
    CMD_SET_TEMPERATURE = 0,
    CMD_SET_MODE = 1,
    CMD_SET_VALVE = 2
};

#endif // PROTOCOL_TYPES_H 