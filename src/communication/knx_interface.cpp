#include "knx_interface.h"
#include "thermostat_state.h"
#include "protocol_manager.h"
#include "esp-knx-ip/esp-knx-ip.h"

// Basic includes
#include <Arduino.h>

// Then include your component headers
#include "config_manager.h"

// Private implementation class
class KNXInterface::Impl {
public:
    Impl() : knx(new EspKnxIp()) {}
    ~Impl() { if (knx) delete knx; }

    EspKnxIp* knx;
};

KNXInterface::KNXInterface() 
    : pimpl(new Impl())
    , connected(false)
    , lastError(ThermostatStatus::OK)
    , thermostatState(nullptr)
    , protocolManager(nullptr)
    , setpointCallbackId(0)
    , modeCallbackId(0) {
    // Initialize addresses with defaults
    physicalAddress = {1, 1, 1};  // Default physical address 1.1.1
    groupAddresses = {
        3, 1, 0,  // Temperature: 3/1/0
        3, 2, 0,  // Setpoint: 3/2/0
        3, 3, 0,  // Valve: 3/3/0
        3, 4, 0   // Mode: 3/4/0
    };
}

KNXInterface::~KNXInterface() {
    if (pimpl) {
        delete pimpl;
        pimpl = nullptr;
    }
}

bool KNXInterface::begin() {
    // Initialize KNX with physical address
    if (!pimpl->knx->begin(physicalAddress.area, physicalAddress.line, physicalAddress.member)) {
        lastError = ThermostatStatus::ERROR_COMMUNICATION;
        return false;
    }
    
    connected = true;
    lastError = ThermostatStatus::OK;
    return true;
}

void KNXInterface::loop() {
    if (connected && pimpl && pimpl->knx) {
        pimpl->knx->loop();
    }
}

bool KNXInterface::isConnected() const {
    return connected;
}

void KNXInterface::setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
    physicalAddress = {area, line, member};
    if (connected && pimpl && pimpl->knx) {
        pimpl->knx->begin(area, line, member);
    }
}

void KNXInterface::setGroupAddress(uint8_t area, uint8_t line, uint8_t member) {
    groupAddresses.tempArea = area;
    groupAddresses.tempLine = line;
    groupAddresses.tempMember = member;
}

bool KNXInterface::sendTemperature(float value) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.tempArea, groupAddresses.tempLine, groupAddresses.tempMember);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendHumidity(float value) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.tempArea, groupAddresses.tempLine + 1, groupAddresses.tempMember);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendPressure(float value) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.tempArea, groupAddresses.tempLine + 2, groupAddresses.tempMember);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendSetpoint(float value) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.setpointArea, groupAddresses.setpointLine, groupAddresses.setpointMember);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendValvePosition(float value) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.valveArea, groupAddresses.valveLine, groupAddresses.valveMember);
    // Convert 0-100 float to 0-255 uint8
    uint8_t scaledValue = static_cast<uint8_t>(value * 2.55f);
    pimpl->knx->write_1byte_uint(addr, scaledValue);
    return true;
}

bool KNXInterface::sendMode(ThermostatMode mode) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.modeArea, groupAddresses.modeLine, groupAddresses.modeMember);
    uint8_t knxMode = modeToKnx(mode);
    pimpl->knx->write_1byte_uint(addr, knxMode);
    return true;
}

bool KNXInterface::sendHeatingState(bool isHeating) {
    if (!connected || !pimpl || !pimpl->knx) return false;
    address_t addr = EspKnxIp::GA_to_address(groupAddresses.valveArea, groupAddresses.valveLine + 1, groupAddresses.valveMember);
    pimpl->knx->write_1bit(addr, isHeating);
    return true;
}

ThermostatStatus KNXInterface::getLastError() const {
    return lastError;
}

void KNXInterface::registerCallbacks(ThermostatState* state, ProtocolManager* manager) {
    thermostatState = state;
    protocolManager = manager;

    if (connected && pimpl && pimpl->knx) {
        // Register for setpoint updates
        address_t setpointAddr = EspKnxIp::GA_to_address(groupAddresses.setpointArea, groupAddresses.setpointLine, groupAddresses.setpointMember);
        setpointCallbackId = pimpl->knx->callback_register("setpoint", 
            [this](message_t const &msg, void *) {
                if (msg.ct == KNX_CT_WRITE && thermostatState) {
                    float setpoint = pimpl->knx->data_to_2byte_float(msg.data);
                    if (protocolManager) {
                        protocolManager->handleIncomingCommand(
                            CommandSource::SOURCE_KNX,
                            CommandType::CMD_SET_TEMPERATURE,
                            setpoint
                        );
                    } else {
                        thermostatState->setTargetTemperature(setpoint);
                    }
                }
            }
        );
        pimpl->knx->callback_assign(setpointCallbackId, setpointAddr);

        // Register for mode updates
        address_t modeAddr = EspKnxIp::GA_to_address(groupAddresses.modeArea, groupAddresses.modeLine, groupAddresses.modeMember);
        modeCallbackId = pimpl->knx->callback_register("mode",
            [this](message_t const &msg, void *) {
                if (msg.ct == KNX_CT_WRITE && thermostatState) {
                    uint8_t modeValue = pimpl->knx->data_to_1byte_uint(msg.data);
                    ThermostatMode mode = knxToMode(modeValue);
                    if (protocolManager) {
                        protocolManager->handleIncomingCommand(
                            CommandSource::SOURCE_KNX,
                            CommandType::CMD_SET_MODE,
                            static_cast<float>(static_cast<int>(mode))
                        );
                    } else {
                        thermostatState->setMode(mode);
                    }
                }
            }
        );
        pimpl->knx->callback_assign(modeCallbackId, modeAddr);
    }
}

uint8_t KNXInterface::modeToKnx(ThermostatMode mode) {
    // Convert our mode to KNX HVAC mode (DPT 20.102)
    switch (mode) {
        case ThermostatMode::OFF:
            return 0;  // Auto
        case ThermostatMode::COMFORT:
            return 1;  // Comfort
        case ThermostatMode::STANDBY:
            return 2;  // Standby
        case ThermostatMode::ECONOMY:
            return 3;  // Economy
        case ThermostatMode::PROTECTION:
            return 4;  // Building Protection
        default:
            return 0;
    }
}

ThermostatMode KNXInterface::knxToMode(uint8_t value) {
    // Convert KNX HVAC mode (DPT 20.102) to our mode
    switch (value) {
        case 1:
            return ThermostatMode::COMFORT;
        case 2:
            return ThermostatMode::STANDBY;
        case 3:
            return ThermostatMode::ECONOMY;
        case 4:
            return ThermostatMode::PROTECTION;
        case 0:
        default:
            return ThermostatMode::OFF;
    }
}
