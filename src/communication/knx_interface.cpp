
#include "knx_interface.h"
#include "protocol_manager.h"

KNXInterface::KNXInterface() : 
  thermostatState(nullptr),
  protocolManager(nullptr),
  setpointCallbackId(0),
  modeCallbackId(0) {
  
  // Initialize group addresses to default values
  temperatureGA = convertToKnxGA(3, 1, 0);
  setpointGA = convertToKnxGA(3, 2, 0);
  valvePositionGA = convertToKnxGA(3, 3, 0);
  modeGA = convertToKnxGA(3, 4, 0);
}

bool KNXInterface::begin(int area, int line, int member) {
  // Set physical address
  address_t pa = knx.PA_to_address(area, line, member);
  knx.physical_address_set(pa);
  
  // Start KNX without the built-in web interface
  knx.start(nullptr);
  
  Serial.printf("KNX physical address set to %d.%d.%d\n", area, line, member);
  
  // Register callback for setpoint
  setpointCallbackId = knx.callback_register("SetpointReceived", handleSetpointCallback, this);
  knx.callback_assign(setpointCallbackId, setpointGA);
  
  // Register callback for mode
  modeCallbackId = knx.callback_register("ModeReceived", handleModeCallback, this);
  knx.callback_assign(modeCallbackId, modeGA);
  
  return true;
}

void KNXInterface::registerCallbacks(ThermostatState* state, ProtocolManager* protocolManager) {
  thermostatState = state;
  this->protocolManager = protocolManager;
}

void KNXInterface::loop() {
  // Process KNX communications using the library loop
  knx.loop();
}

void KNXInterface::setTemperatureGA(int area, int line, int member) {
  temperatureGA = convertToKnxGA(area, line, member);
  Serial.printf("KNX Temperature GA set to %d/%d/%d\n", area, line, member);
}

void KNXInterface::setSetpointGA(int area, int line, int member) {
  setpointGA = convertToKnxGA(area, line, member);
  Serial.printf("KNX Setpoint GA set to %d/%d/%d\n", area, line, member);
  
  // Update the callback with the new group address
  if (setpointCallbackId > 0) {
    knx.callback_assign(setpointCallbackId, setpointGA);
  }
}

void KNXInterface::setValvePositionGA(int area, int line, int member) {
  valvePositionGA = convertToKnxGA(area, line, member);
  Serial.printf("KNX Valve Position GA set to %d/%d/%d\n", area, line, member);
}

void KNXInterface::setModeGA(int area, int line, int member) {
  modeGA = convertToKnxGA(area, line, member);
  Serial.printf("KNX Mode GA set to %d/%d/%d\n", area, line, member);
  
  // Update the callback with the new group address
  if (modeCallbackId > 0) {
    knx.callback_assign(modeCallbackId, modeGA);
  }
}

bool KNXInterface::sendTemperature(float temperature) {
  // Send the temperature value using the KNX library
  knx.write_2byte_float(temperatureGA, temperature);
  Serial.printf("Temperature sent to KNX: %.2f°C\n", temperature);
  return true;
}

bool KNXInterface::sendSetpoint(float setpoint) {
  // Send the setpoint value using the KNX library
  knx.write_2byte_float(setpointGA, setpoint);
  Serial.printf("Setpoint sent to KNX: %.2f°C\n", setpoint);
  return true;
}

bool KNXInterface::sendValvePosition(float position) {
  // Ensure position is within 0-100 range
  uint8_t scaledPosition = constrain(position, 0, 100);
  
  // Send the valve position using the KNX library
  knx.write_1byte_uint(valvePositionGA, scaledPosition);
  Serial.printf("Valve position sent to KNX: %.1f%%\n", position);
  return true;
}

bool KNXInterface::sendMode(ThermostatMode mode) {
  // Send the operating mode using the KNX library
  uint8_t modeValue = static_cast<uint8_t>(mode);
  
  knx.write_1byte_uint(modeGA, modeValue);
  Serial.printf("Mode sent to KNX: %d\n", modeValue);
  return true;
}

address_t KNXInterface::convertToKnxGA(int area, int line, int member) {
  return knx.GA_to_address(area, line, member);
}

void KNXInterface::handleSetpointCallback(message_t const &msg, void *arg) {
  KNXInterface* instance = static_cast<KNXInterface*>(arg);
  
  if (instance && instance->thermostatState) {
    // Process a setpoint message from the KNX bus
    if (msg.ct == KNX_CT_WRITE) {
      float newSetpoint = knx.data_to_2byte_float(msg.data);
      Serial.printf("Received setpoint from KNX: %.2f°C\n", newSetpoint);
      
      // Use protocol manager to handle the command with proper priority
      if (instance->protocolManager) {
        instance->protocolManager->handleIncomingCommand(
          SOURCE_KNX, 
          CMD_SET_TEMPERATURE, 
          newSetpoint
        );
      } else {
        // Direct update if protocol manager isn't available
        instance->thermostatState->setTargetTemperature(newSetpoint);
      }
    }
  }
}

void KNXInterface::handleModeCallback(message_t const &msg, void *arg) {
  KNXInterface* instance = static_cast<KNXInterface*>(arg);
  
  if (instance && instance->thermostatState) {
    // Process a mode change message from the KNX bus
    if (msg.ct == KNX_CT_WRITE) {
      uint8_t modeValue = knx.data_to_1byte_uint(msg.data);
      ThermostatMode newMode = static_cast<ThermostatMode>(modeValue);
      
      Serial.printf("Received mode from KNX: %d\n", modeValue);
      
      // Use protocol manager to handle the command with proper priority
      if (instance->protocolManager) {
        instance->protocolManager->handleIncomingCommand(
          SOURCE_KNX, 
          CMD_SET_MODE, 
          static_cast<float>(modeValue)
        );
      } else {
        // Direct update if protocol manager isn't available
        instance->thermostatState->setMode(newMode);
      }
    }
  }
}
