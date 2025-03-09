#include "knx_interface.h"
#include <esp_knx_ip.h>  // Include the KNX library

KNXInterface::KNXInterface()
    : connected(false)
    , lastError(ThermostatStatus::OK)
    , thermostatState(nullptr)
    , protocolManager(nullptr)
    , setpointCallbackId(0)
    , modeCallbackId(0) {
    // Initialize addresses to 0
    physicalAddress = {0, 0, 0};
    groupAddresses = {0};
}

bool KNXInterface::begin() {
    // Set physical address
    address_t pa = knx.PA_to_address(physicalAddress.area, physicalAddress.line, physicalAddress.member);
    knx.physical_address_set(pa);
    
    // Start KNX without the built-in web interface
    connected = knx.start(nullptr);
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    Serial.printf("KNX physical address set to %d.%d.%d\n", 
                 physicalAddress.area, physicalAddress.line, physicalAddress.member);
    
    // Register callback for setpoint
    setpointCallbackId = knx.callback_register("SetpointReceived", 
        [this](message_t const &msg) {
            if (msg.ct == KNX_CT_WRITE) {
                float newSetpoint = knx.data_to_2byte_float(msg.data);
                if (protocolManager) {
                    protocolManager->handleIncomingCommand(SOURCE_KNX, CMD_SET_TEMPERATURE, newSetpoint);
                }
            }
        }
    );
    
    // Register callback for mode
    modeCallbackId = knx.callback_register("ModeReceived", 
        [this](message_t const &msg) {
            if (msg.ct == KNX_CT_WRITE) {
                uint8_t modeValue = knx.data_to_1byte_uint(msg.data);
                if (protocolManager) {
                    protocolManager->handleIncomingCommand(SOURCE_KNX, CMD_SET_MODE, static_cast<float>(modeValue));
                }
            }
        }
    );
    
    lastError = ThermostatStatus::OK;
    return true;
}

void KNXInterface::loop() {
    knx.loop();
}

bool KNXInterface::isConnected() const {
    return connected;
}

bool KNXInterface::sendTemperature(float value) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    address_t ga = knx.GA_to_address(groupAddresses.tempArea, 
                                    groupAddresses.tempLine, 
                                    groupAddresses.tempMember);
    knx.write_2byte_float(ga, value);
    return true;
}

bool KNXInterface::sendHumidity(float value) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    uint16_t humidityScaled = static_cast<uint16_t>(value * 100);
    address_t ga = knx.GA_to_address(groupAddresses.tempArea, 
                                    groupAddresses.tempLine, 
                                    groupAddresses.tempMember);
    knx.write_2byte_uint(ga, humidityScaled);
    return true;
}

bool KNXInterface::sendPressure(float value) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    address_t ga = knx.GA_to_address(groupAddresses.tempArea, 
                                    groupAddresses.tempLine, 
                                    groupAddresses.tempMember);
    knx.write_2byte_float(ga, value);
    return true;
}

bool KNXInterface::sendSetpoint(float value) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    address_t ga = knx.GA_to_address(groupAddresses.setpointArea, 
                                    groupAddresses.setpointLine, 
                                    groupAddresses.setpointMember);
    knx.write_2byte_float(ga, value);
    return true;
}

bool KNXInterface::sendValvePosition(float value) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    uint8_t scaled = static_cast<uint8_t>(value * 255 / 100);
    address_t ga = knx.GA_to_address(groupAddresses.valveArea, 
                                    groupAddresses.valveLine, 
                                    groupAddresses.valveMember);
    knx.write_1byte_uint(ga, scaled);
    return true;
}

bool KNXInterface::sendMode(ThermostatMode mode) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    uint8_t modeValue = static_cast<uint8_t>(mode);
    address_t ga = knx.GA_to_address(groupAddresses.modeArea, 
                                    groupAddresses.modeLine, 
                                    groupAddresses.modeMember);
    knx.write_1byte_uint(ga, modeValue);
    return true;
}

bool KNXInterface::sendHeatingState(bool isHeating) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }
    
    uint8_t state = isHeating ? 1 : 0;
    address_t ga = knx.GA_to_address(groupAddresses.valveArea, 
                                    groupAddresses.valveLine, 
                                    groupAddresses.valveMember);
    knx.write_1byte_uint(ga, state);
    return true;
}

ThermostatStatus KNXInterface::getLastError() const {
    return lastError;
}

void KNXInterface::setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
    physicalAddress.area = area;
    physicalAddress.line = line;
    physicalAddress.member = member;
    
    if (connected) {
        address_t pa = knx.PA_to_address(area, line, member);
        knx.physical_address_set(pa);
    }
}

void KNXInterface::setGroupAddress(uint8_t area, uint8_t line, uint8_t member) {
    groupAddresses.tempArea = area;
    groupAddresses.tempLine = line;
    groupAddresses.tempMember = member;
}

void KNXInterface::registerCallbacks(ThermostatState* state, ProtocolManager* manager) {
    thermostatState = state;
    protocolManager = manager;
    
    // Update callback assignments if connected
    if (connected) {
        if (setpointCallbackId > 0) {
            address_t ga = knx.GA_to_address(groupAddresses.setpointArea, 
                                           groupAddresses.setpointLine, 
                                           groupAddresses.setpointMember);
            knx.callback_assign(setpointCallbackId, ga);
        }
        
        if (modeCallbackId > 0) {
            address_t ga = knx.GA_to_address(groupAddresses.modeArea, 
                                           groupAddresses.modeLine, 
                                           groupAddresses.modeMember);
            knx.callback_assign(modeCallbackId, ga);
        }
    }
}

void KNXInterface::handleKNXEvent(uint16_t address, uint8_t* data, uint8_t length) {
    if (!thermostatState) {
        return;
    }

    // TODO: Implement KNX event handling
    // This would typically involve:
    // 1. Decode the group address
    // 2. Parse the data according to its DPT (Data Point Type)
    // 3. Update the thermostat state accordingly
}

bool KNXInterface::sendValue(uint8_t area, uint8_t line, uint8_t member, const void* data, uint8_t length) {
    if (!connected) {
        lastError = ThermostatStatus::COMMUNICATION_ERROR;
        return false;
    }

    // TODO: Implement KNX telegram sending
    // This would typically involve:
    // 1. Create a KNX telegram
    // 2. Set the group address
    // 3. Set the data and length
    // 4. Send the telegram
    
    return true;
} 