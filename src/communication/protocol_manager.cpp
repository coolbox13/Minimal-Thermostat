#include "thermostat_state.h"
#include "protocol_manager.h"
#include "knx_interface.h"
#include "mqtt_interface.h"

ProtocolManager::ProtocolManager() :
  knxInterface(nullptr),
  mqttInterface(nullptr),
  thermostatState(nullptr),
  lastSendTime(0),
  sendInterval(10000) { // Default 10 seconds
}

void ProtocolManager::begin(ThermostatState* state) {
  thermostatState = state;
}

void ProtocolManager::registerProtocols(KNXInterface* knx) {
  knxInterface = knx;
  
  // Register callbacks if state is available
  if (thermostatState && knxInterface) {
    knxInterface->registerCallbacks(thermostatState);
  }
}

void ProtocolManager::registerMQTT(MQTTInterface* mqtt) {
  mqttInterface = mqtt;
  
  // No special setup needed for MQTT, it registers its own callbacks
}

void ProtocolManager::loop() {
  unsigned long currentTime = millis();
  
  // Check if it's time to send updates
  if (currentTime - lastSendTime >= sendInterval) {
    // Send current values to protocols
    if (thermostatState) {
      sendTemperature(thermostatState->currentTemperature);
      sendSetpoint(thermostatState->targetTemperature);
      sendValvePosition(thermostatState->valvePosition);
      sendMode(thermostatState->operatingMode);
    }
    
    lastSendTime = currentTime;
  }
}

bool ProtocolManager::handleIncomingCommand(int source, int command, float value) {
  if (!thermostatState) {
    return false;
  }
  
  bool success = false;
  
  switch (command) {
    case CMD_SET_TEMPERATURE:
      thermostatState->setTargetTemperature(value);
      // Propagate to other protocols (except the source)
      if (source != SOURCE_KNX && knxInterface) {
        knxInterface->sendSetpoint(value);
      }
      if (source != SOURCE_MQTT && mqttInterface) {
        mqttInterface->publishSetpoint(value);
      }
      success = true;
      break;
      
    case CMD_SET_MODE:
      thermostatState->setMode(static_cast<ThermostatMode>(value));
      // Propagate to other protocols
      if (source != SOURCE_KNX && knxInterface) {
        knxInterface->sendMode(static_cast<ThermostatMode>(value));
      }
      if (source != SOURCE_MQTT && mqttInterface) {
        mqttInterface->publishMode(static_cast<ThermostatMode>(value));
      }
      success = true;
      break;
      
    case CMD_SET_VALVE:
      thermostatState->setValvePosition(value);
      // Propagate to other protocols
      if (source != SOURCE_KNX && knxInterface) {
        knxInterface->sendValvePosition(value);
      }
      if (source != SOURCE_MQTT && mqttInterface) {
        mqttInterface->publishValvePosition(value);
      }
      success = true;
      break;
  }
  
  return success;
}

void ProtocolManager::sendTemperature(float temperature) {
  // Send to KNX
  if (knxInterface) {
    knxInterface->sendTemperature(temperature);
  }
  
  // Send to MQTT
  if (mqttInterface) {
    mqttInterface->publishTemperature(temperature);
  }
}

void ProtocolManager::sendSetpoint(float setpoint) {
  // Send to KNX
  if (knxInterface) {
    knxInterface->sendSetpoint(setpoint);
  }
  
  // Send to MQTT
  if (mqttInterface) {
    mqttInterface->publishSetpoint(setpoint);
  }
}

void ProtocolManager::sendValvePosition(float position) {
  // Send to KNX
  if (knxInterface) {
    knxInterface->sendValvePosition(position);
  }
  
  // Send to MQTT
  if (mqttInterface) {
    mqttInterface->publishValvePosition(position);
  }
}

void ProtocolManager::sendMode(ThermostatMode mode) {
  // Send to KNX
  if (knxInterface) {
    knxInterface->sendMode(mode);
  }
  
  // Send to MQTT
  if (mqttInterface) {
    mqttInterface->publishMode(mode);
  }
}