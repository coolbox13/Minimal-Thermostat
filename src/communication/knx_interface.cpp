#include "knx_interface.h"

// Constructor
KNXInterface::KNXInterface() : thermostatState(nullptr) {
  // Initialize group addresses to default values (3/1/0, 3/2/0, etc.)
  temperatureGA = calculateGA(3, 1, 0);
  setpointGA = calculateGA(3, 2, 0);
  valvePositionGA = calculateGA(3, 3, 0);
  modeGA = calculateGA(3, 4, 0);
}

// Initialize KNX communication
bool KNXInterface::begin(int area, int line, int member) {
  // Start KNX communication
  if (!knx.start(nullptr)) {
    Serial.println("Failed to start KNX communication");
    return false;
  }
  
  // Set physical address
  address_t physical;
  physical.pa.area = area;
  physical.pa.line = line;
  physical.pa.member = member;
  
  if (!knx.physical_address_set(physical)) {
    Serial.println("Failed to set KNX physical address");
    return false;
  }
  
  Serial.printf("KNX physical address set to %d.%d.%d\n", area, line, member);
  
  // Register callbacks
  registerKNXCallbacks();
  
  return true;
}

// Register with thermostat state
void KNXInterface::registerCallbacks(ThermostatState* state) {
  thermostatState = state;
  
  // Register callbacks to send updates to KNX when thermostat state changes
  if (thermostatState) {
    thermostatState->onTemperatureChanged = [this](float temp) {
      this->sendTemperature(temp);
    };
    
    thermostatState->onSetpointChanged = [this](float setpoint) {
      this->sendSetpoint(setpoint);
    };
    
    thermostatState->onValvePositionChanged = [this](float position) {
      this->sendValvePosition(position);
    };
    
    thermostatState->onModeChanged = [this](ThermostatMode mode) {
      this->sendMode(mode);
    };
  }
}

// Set group address for temperature
void KNXInterface::setTemperatureGA(int area, int line, int member) {
  temperatureGA = calculateGA(area, line, member);
}

// Set group address for setpoint
void KNXInterface::setSetpointGA(int area, int line, int member) {
  setpointGA = calculateGA(area, line, member);
  
  // Re-register callback for the new group address
  registerKNXCallbacks();
}

// Set group address for valve position
void KNXInterface::setValvePositionGA(int area, int line, int member) {
  valvePositionGA = calculateGA(area, line, member);
}

// Set group address for mode
void KNXInterface::setModeGA(int area, int line, int member) {
  modeGA = calculateGA(area, line, member);
  
  // Re-register callback for the new group address
  registerKNXCallbacks();
}

// Process KNX loop
void KNXInterface::loop() {
  knx.loop();
}

// Send temperature to KNX (DPT 9.001 - Temperature)
bool KNXInterface::sendTemperature(float temperature) {
  return knx.write_2byte_float(temperatureGA, temperature);
}

// Send setpoint to KNX (DPT 9.001 - Temperature)
bool KNXInterface::sendSetpoint(float setpoint) {
  return knx.write_2byte_float(setpointGA, setpoint);
}

// Send valve position to KNX (DPT 5.001 - Percentage)
bool KNXInterface::sendValvePosition(float position) {
  return knx.write_1byte_uint(valvePositionGA, (uint8_t)position);
}

// Send mode to KNX (DPT 5.xxx - Unsigned Value)
bool KNXInterface::sendMode(ThermostatMode mode) {
  return knx.write_1byte_uint(modeGA, (uint8_t)mode);
}

// Helper to calculate group address
address_t KNXInterface::calculateGA(int area, int line, int member) {
  address_t tmp;
  tmp.ga.area = area;
  tmp.ga.line = line;
  tmp.ga.member = member;
  return tmp;
}

// KNX callback for setpoint
void KNXInterface::handleSetpointCallback(message_t const &msg, void *arg) {
  KNXInterface* instance = static_cast<KNXInterface*>(arg);
  
  if (instance && instance->thermostatState && msg.data_len >= 2) {
    // KNX DPT 9.001 is 2-byte float
    float newSetpoint = knx.dpt2_to_float(msg.data[0], msg.data[1]);
    Serial.print("Received setpoint from KNX: ");
    Serial.println(newSetpoint);
    
    instance->thermostatState->setTargetTemperature(newSetpoint);
  }
}

// KNX callback for mode
void KNXInterface::handleModeCallback(message_t const &msg, void *arg) {
  KNXInterface* instance = static_cast<KNXInterface*>(arg);
  
  if (instance && instance->thermostatState && msg.data_len >= 1) {
    uint8_t modeValue = msg.data[0];
    ThermostatMode newMode = static_cast<ThermostatMode>(modeValue);
    
    Serial.print("Received mode from KNX: ");
    Serial.println(modeValue);
    
    instance->thermostatState->setMode(newMode);
  }
}

// Helper for registering with KNX callbacks
void KNXInterface::registerKNXCallbacks() {
  // Register for setpoint changes
  knx.callback_register("SetpointReceived", setpointGA, handleSetpointCallback, this);
  
  // Register for mode changes
  knx.callback_register("ModeReceived", modeGA, handleModeCallback, this);
}