#include "thermostat_state.h"
#include "protocol_manager.h"
#include "knx_interface.h"
#include "mqtt_interface.h"

ProtocolManager::ProtocolManager(ThermostatState* state)
    : thermostatState(state)
    , knxInterface(nullptr)
    , mqttInterface(nullptr)
    , lastCommandSource(CommandSource::SOURCE_INTERNAL)
    , lastCommandTime(0) {
}

void ProtocolManager::registerProtocols(KNXInterface* knx, MQTTInterface* mqtt) {
    knxInterface = knx;
    mqttInterface = mqtt;
    
    // Register callbacks if state is available
    if (thermostatState && knxInterface) {
        knxInterface->registerCallbacks(thermostatState, this);
    }
    
    if (thermostatState && mqttInterface) {
        mqttInterface->registerProtocolManager(this);
    }
}

bool ProtocolManager::handleIncomingCommand(CommandSource source, CommandType cmd, float value) {
    // Check if this source has priority
    if (!hasHigherPriority(source, lastCommandSource)) {
        return false;
    }

    // Update command source and time
    lastCommandSource = source;
    lastCommandTime = millis();

    // Process command
    bool success = false;
    switch (cmd) {
        case CommandType::CMD_SET_TEMPERATURE:
            thermostatState->setTargetTemperature(value);
            // Forward to other protocols
            if (source != CommandSource::SOURCE_KNX && knxInterface) {
                knxInterface->sendSetpoint(value);
            }
            if (source != CommandSource::SOURCE_MQTT && mqttInterface) {
                mqttInterface->sendSetpoint(value);
            }
            success = true;
            break;
            
        case CommandType::CMD_SET_MODE:
            thermostatState->setMode(static_cast<ThermostatMode>(static_cast<int>(value)));
            // Forward to other protocols
            if (source != CommandSource::SOURCE_KNX && knxInterface) {
                knxInterface->sendMode(static_cast<ThermostatMode>(static_cast<int>(value)));
            }
            if (source != CommandSource::SOURCE_MQTT && mqttInterface) {
                mqttInterface->sendMode(static_cast<ThermostatMode>(static_cast<int>(value)));
            }
            success = true;
            break;
            
        case CommandType::CMD_SET_VALVE:
            thermostatState->setValvePosition(value);
            // Forward to other protocols
            if (source != CommandSource::SOURCE_KNX && knxInterface) {
                knxInterface->sendValvePosition(value);
            }
            if (source != CommandSource::SOURCE_MQTT && mqttInterface) {
                mqttInterface->sendValvePosition(value);
            }
            success = true;
            break;
    }
    
    return success;
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
    // If enough time has passed, any source can take control
    if (millis() - lastCommandTime > PRIORITY_TIMEOUT) {
        return true;
    }
    
    // Priority order: KNX > MQTT > WEB_API > INTERNAL
    return static_cast<uint8_t>(newSource) <= static_cast<uint8_t>(currentSource);
}