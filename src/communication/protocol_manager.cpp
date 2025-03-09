#include "thermostat_state.h"
#include "protocol_manager.h"
#include "communication/knx/knx_interface.h"
#include "mqtt_interface.h"

ProtocolManager::ProtocolManager(ThermostatState* state)
    : thermostatState(state)
    , knxInterface(nullptr)
    , mqttInterface(nullptr)
    , lastCommandSource(CommandSource::SOURCE_INTERNAL)
    , lastCommandType(CommandType::CMD_NONE)
    , lastCommandValue(0.0f)
{
}

void ProtocolManager::registerProtocols(KNXInterface* knx, MQTTInterface* mqtt) {
    knxInterface = knx;
    mqttInterface = mqtt;
    
    // Register this protocol manager with the interfaces
    if (knxInterface) {
        knxInterface->registerProtocolManager(this);
    }
    
    if (mqttInterface) {
        mqttInterface->registerProtocolManager(this);
    }
}

bool ProtocolManager::handleIncomingCommand(CommandSource source, CommandType cmd, float value) {
    // Store last command
    lastCommandSource = source;
    lastCommandType = cmd;
    lastCommandValue = value;
    
    // Process command
    switch (cmd) {
        case CommandType::CMD_SETPOINT:
            if (thermostatState) {
                thermostatState->setTargetTemperature(value);
                
                // Propagate to other interfaces
                propagateCommand(source, cmd, value);
                return true;
            }
            break;
            
        case CommandType::CMD_MODE:
            if (thermostatState) {
                ThermostatMode mode = static_cast<ThermostatMode>(static_cast<int>(value));
                thermostatState->setMode(mode);
                
                // Propagate to other interfaces
                propagateCommand(source, cmd, value);
                return true;
            }
            break;
            
        case CommandType::CMD_VALVE:
            if (thermostatState) {
                thermostatState->setValvePosition(value);
                
                // Propagate to other interfaces
                propagateCommand(source, cmd, value);
                return true;
            }
            break;
            
        case CommandType::CMD_HEATING:
            if (thermostatState) {
                thermostatState->setHeating(value > 0.5f);
                
                // Propagate to other interfaces
                propagateCommand(source, cmd, value);
                return true;
            }
            break;
            
        default:
            // Unknown command
            return false;
    }
    
    return false;
}

void ProtocolManager::propagateCommand(CommandSource source, CommandType cmd, float value) {
    // Send command to all interfaces except the source
    if (knxInterface && source != CommandSource::SOURCE_KNX) {
        switch (cmd) {
            case CommandType::CMD_SETPOINT:
                knxInterface->sendSetpoint(value);
                break;
                
            case CommandType::CMD_MODE:
                knxInterface->sendMode(static_cast<ThermostatMode>(static_cast<int>(value)));
                break;
                
            case CommandType::CMD_VALVE:
                knxInterface->sendValvePosition(value);
                break;
                
            case CommandType::CMD_HEATING:
                knxInterface->sendHeatingState(value > 0.5f);
                break;
                
            default:
                break;
        }
    }
    
    if (mqttInterface && source != CommandSource::SOURCE_MQTT) {
        switch (cmd) {
            case CommandType::CMD_SETPOINT:
                mqttInterface->sendSetpoint(value);
                break;
                
            case CommandType::CMD_MODE:
                mqttInterface->sendMode(static_cast<ThermostatMode>(static_cast<int>(value)));
                break;
                
            case CommandType::CMD_VALVE:
                mqttInterface->sendValvePosition(value);
                break;
                
            case CommandType::CMD_HEATING:
                mqttInterface->sendHeatingState(value > 0.5f);
                break;
                
            default:
                break;
        }
    }
}

void ProtocolManager::sendTemperature(float temperature) {
    if (knxInterface) knxInterface->sendTemperature(temperature);
    if (mqttInterface) mqttInterface->sendTemperature(temperature);
}

void ProtocolManager::sendSetpoint(float setpoint) {
    if (knxInterface) knxInterface->sendSetpoint(setpoint);
    if (mqttInterface) mqttInterface->sendSetpoint(setpoint);
}

void ProtocolManager::sendValvePosition(float position) {
    if (knxInterface) knxInterface->sendValvePosition(position);
    if (mqttInterface) mqttInterface->sendValvePosition(position);
}

void ProtocolManager::sendMode(ThermostatMode mode) {
    if (knxInterface) knxInterface->sendMode(mode);
    if (mqttInterface) mqttInterface->sendMode(mode);
}

void ProtocolManager::sendHeatingState(bool isHeating) {
    if (knxInterface) knxInterface->sendHeatingState(isHeating);
    if (mqttInterface) mqttInterface->sendHeatingState(isHeating);
}

bool ProtocolManager::hasHigherPriority(CommandSource newSource, CommandSource currentSource) {
    // Simple priority check without timeout
    // Priority order: KNX > MQTT > WEB_API > WEB > INTERNAL
    return static_cast<uint8_t>(newSource) <= static_cast<uint8_t>(currentSource);
}