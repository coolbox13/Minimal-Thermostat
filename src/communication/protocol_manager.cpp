#include "protocol_manager.h"
#include "knx_interface.h"
#include "mqtt_interface.h"

ProtocolManager::ProtocolManager() : 
  thermostatState(nullptr),
  knxInterface(nullptr),
  mqttInterface(nullptr),
  lastCommandSource(SOURCE_LOCAL),
  lastCommandTime(0) {
}

void ProtocolManager::begin(ThermostatState* state) {
  thermostatState = state;
}

void ProtocolManager::registerProtocols(KNXInterface* knx, MQTTInterface* mqtt) {
  knxInterface = knx;
  mqttInterface = mqtt;
}

void ProtocolManager::handleIncomingCommand(CommandSource source, CommandType cmd, float value) {
  // Skip processing if thermostat state not available
  if (!thermostatState) {
    Serial.println("Cannot process command: thermostat state not available");
    return;
  }

  // Prevent command loops by implementing a cooldown
  unsigned long currentTime = millis();
  if (currentTime - lastCommandTime < COMMAND_COOLDOWN) {
    // Only allow if new command has higher priority
    if (!hasPriority(source, lastCommandSource)) {
      Serial.printf("Command rejected: %d cooldown active, source %d doesn't have priority over %d\n", 
                  COMMAND_COOLDOWN, source, lastCommandSource);
      return;
    }
  }
  
  // Update last command details
  lastCommandSource = source;
  lastCommandTime = currentTime;
  
  // Process command based on type
  switch (cmd) {
    case CMD_SET_TEMPERATURE:
      Serial.printf("Setting target temperature to %.2f°C from source %d\n", value, source);
      thermostatState->setTargetTemperature(value);
      break;
      
    case CMD_SET_MODE:
      if (value >= MODE_OFF && value <= MODE_ANTIFREEZE) {
        Serial.printf("Setting mode to %d from source %d\n", (int)value, source);
        thermostatState->setMode(static_cast<ThermostatMode>(static_cast<int>(value)));
      } else {
        Serial.printf("Invalid mode value: %d\n", (int)value);
      }
      break;
      
    case CMD_SET_VALVE_POSITION:
      // Only certain sources should be able to directly set valve position
      if (source == SOURCE_LOCAL || source == SOURCE_WEB_API) {
        Serial.printf("Setting valve position to %.2f%% from source %d\n", value, source);
        thermostatState->setValvePosition(value);
      } else {
        Serial.printf("Valve position command from source %d rejected (unauthorized source)\n", source);
      }
      break;
      
    default:
      Serial.printf("Unknown command type: %d\n", cmd);
      break;
  }
  
  // Broadcast state to all interfaces after change
  broadcastState(thermostatState);
}

void ProtocolManager::broadcastState(ThermostatState* state) {
  // Skip if state not available
  if (!state) return;
  
  // Broadcast current state to all protocols
  if (knxInterface && state) {
    knxInterface->sendTemperature(state->currentTemperature);
    knxInterface->sendSetpoint(state->targetTemperature);
    knxInterface->sendValvePosition(state->valvePosition);
    knxInterface->sendMode(state->operatingMode);
  }
  
  if (mqttInterface && state) {
    mqttInterface->publishTemperature(state->currentTemperature);
    mqttInterface->publishHumidity(state->currentHumidity);
    mqttInterface->publishPressure(state->currentPressure);
    mqttInterface->publishSetpoint(state->targetTemperature);
    mqttInterface->publishValvePosition(state->valvePosition);
    mqttInterface->publishMode(state->operatingMode);
    mqttInterface->publishHeatingStatus(state->heatingActive);
  }
}

void ProtocolManager::loop() {
  // Process protocol loops
  if (knxInterface) {
    knxInterface->loop();
  }
  
  if (mqttInterface) {
    mqttInterface->loop();
  }
  
  // Periodically broadcast state (heartbeat)
  static unsigned long lastBroadcastTime = 0;
  unsigned long currentTime = millis();
  
  // Every 60 seconds, broadcast state as heartbeat
  if (currentTime - lastBroadcastTime > 60000) {
    lastBroadcastTime = currentTime;
    
    if (thermostatState) {
      broadcastState(thermostatState);
    }
  }
}

bool ProtocolManager::hasPriority(CommandSource a, CommandSource b) {
  // Priority order: LOCAL > KNX > MQTT > WEB_API > SCHEDULE
  return a < b;
}