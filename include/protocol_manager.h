#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <Arduino.h>
#include "thermostat_types.h"
#include "protocol_types.h"

// Forward declarations
class ThermostatState;
class KNXInterface;
class MQTTInterface;

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

    // Command tracking
    CommandSource lastCommandSource;
    CommandType lastCommandType;
    float lastCommandValue;
    
    // Helper to propagate commands to other interfaces
    void propagateCommand(CommandSource source, CommandType cmd, float value);
    
    // Helper to determine command priority
    bool hasHigherPriority(CommandSource newSource, CommandSource currentSource);
};

#endif // PROTOCOL_MANAGER_H