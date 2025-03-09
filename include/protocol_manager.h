#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <Arduino.h>
#include "thermostat_types.h"
#include "protocol_types.h"

// Forward declarations
class ThermostatState;
class KNXInterface;
class MQTTInterface;

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

class ProtocolManager {
public:
    ProtocolManager(ThermostatState* state);

    // Protocol registration
    void registerProtocols(KNXInterface* knx = nullptr, MQTTInterface* mqtt = nullptr);

    // Command handling
    bool handleIncomingCommand(CommandSource source, CommandType cmd, float value);

    // Data transmission to all registered protocols
    void sendTemperature(float temperature);
    void sendSetpoint(float setpoint);
    void sendValvePosition(float position);
    void sendMode(ThermostatMode mode);
    void sendHeatingState(bool isHeating);

private:
    ThermostatState* thermostatState;
    KNXInterface* knxInterface;
    MQTTInterface* mqttInterface;

    // Command priority handling
    bool hasHigherPriority(CommandSource newSource, CommandSource currentSource);
    CommandSource lastCommandSource;
    unsigned long lastCommandTime;
    static const unsigned long PRIORITY_TIMEOUT = 5000; // 5 seconds
};

#endif // PROTOCOL_MANAGER_H