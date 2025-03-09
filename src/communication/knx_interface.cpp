#include <Arduino.h>
#include <memory>

// Include our interfaces and types first
#include "interfaces/protocol_interface.h"
#include "thermostat_types.h"
#include "protocol_types.h"

// Then include our implementation headers
#include "knx_interface.h"
#include "thermostat_state.h"
#include "protocol_manager.h"

// Finally include the KNX library
#include "esp-knx-ip.h"

// Private implementation class
class KNXInterface::Impl {
public:
    Impl() : knx(std::make_unique<EspKnxIp::EspKnxIp>()) {
        knx->setBufferSize(512); // Increase buffer size for larger messages
    }

    // KNX state
    std::unique_ptr<EspKnxIp::EspKnxIp> knx;
    bool connected = false;
    ThermostatStatus lastError = ThermostatStatus::OK;
    const char* lastErrorMsg = nullptr;
    
    // KNX addresses
    struct {
        uint8_t area;
        uint8_t line;
        uint8_t member;
    } physicalAddress = {1, 1, 1};  // Default physical address 1.1.1

    // Group addresses for each datapoint
    KNXInterface::GroupAddress temperatureGA = {3, 1, 0};  // Default: 3/1/0
    KNXInterface::GroupAddress humidityGA = {3, 1, 1};     // Default: 3/1/1
    KNXInterface::GroupAddress pressureGA = {3, 1, 2};     // Default: 3/1/2
    KNXInterface::GroupAddress setpointGA = {3, 2, 0};     // Default: 3/2/0
    KNXInterface::GroupAddress valveGA = {3, 3, 0};        // Default: 3/3/0
    KNXInterface::GroupAddress modeGA = {3, 4, 0};         // Default: 3/4/0
    KNXInterface::GroupAddress heatingGA = {3, 5, 0};      // Default: 3/5/0

    // Component references
    ThermostatState* thermostatState = nullptr;
    ProtocolManager* protocolManager = nullptr;

    // Callback IDs
    uint32_t setpointCallbackId = 0;
    uint32_t modeCallbackId = 0;

    // Helper methods
    EspKnxIp::address_t gaToAddress(const KNXInterface::GroupAddress& ga) const {
        return EspKnxIp::GA_to_address(ga.area, ga.line, ga.member);
    }

    void setError(ThermostatStatus status, const char* message) {
        lastError = status;
        lastErrorMsg = message;
    }

    void clearError() {
        lastError = ThermostatStatus::OK;
        lastErrorMsg = nullptr;
    }
};

// Static instance pointer initialization
static KNXInterface* instance = nullptr;

KNXInterface::KNXInterface() : pimpl(std::make_unique<Impl>()) {
    instance = this;
}

KNXInterface::~KNXInterface() = default;

bool KNXInterface::begin() {
    if (!pimpl || !pimpl->knx) {
        pimpl->setError(ThermostatStatus::ERROR_INITIALIZATION, "KNX implementation not initialized");
        return false;
    }

    // Initialize KNX with physical address
    if (!pimpl->knx->begin(pimpl->physicalAddress.area, 
                          pimpl->physicalAddress.line, 
                          pimpl->physicalAddress.member)) {
        pimpl->setError(ThermostatStatus::ERROR_COMMUNICATION, "Failed to initialize KNX interface");
        return false;
    }
    
    pimpl->connected = true;
    pimpl->clearError();
    return true;
}

void KNXInterface::loop() {
    if (pimpl->connected && pimpl->knx) {
        pimpl->knx->loop();
    }
}

bool KNXInterface::isConnected() const {
    return pimpl && pimpl->connected;
}

void KNXInterface::disconnect() {
    if (pimpl && pimpl->knx) {
        cleanupCallbacks();
        pimpl->connected = false;
    }
}

bool KNXInterface::reconnect() {
    disconnect();
    return begin();
}

bool KNXInterface::configure(const JsonDocument& config) {
    if (!pimpl) return false;

    // Physical address
    if (config.containsKey("physical_address")) {
        const auto& pa = config["physical_address"];
        if (pa.containsKey("area") && pa.containsKey("line") && pa.containsKey("member")) {
            pimpl->physicalAddress = {
                static_cast<uint8_t>(pa["area"].as<int>()),
                static_cast<uint8_t>(pa["line"].as<int>()),
                static_cast<uint8_t>(pa["member"].as<int>())
            };
        }
    }

    // Group addresses
    if (config.containsKey("group_addresses")) {
        const auto& ga = config["group_addresses"];
        auto setGA = [](const JsonObject& obj, GroupAddress& ga) {
            if (obj.containsKey("area") && obj.containsKey("line") && obj.containsKey("member")) {
                ga = {
                    static_cast<uint8_t>(obj["area"].as<int>()),
                    static_cast<uint8_t>(obj["line"].as<int>()),
                    static_cast<uint8_t>(obj["member"].as<int>())
                };
            }
        };

        if (ga.containsKey("temperature")) setGA(ga["temperature"], pimpl->temperatureGA);
        if (ga.containsKey("humidity")) setGA(ga["humidity"], pimpl->humidityGA);
        if (ga.containsKey("pressure")) setGA(ga["pressure"], pimpl->pressureGA);
        if (ga.containsKey("setpoint")) setGA(ga["setpoint"], pimpl->setpointGA);
        if (ga.containsKey("valve")) setGA(ga["valve"], pimpl->valveGA);
        if (ga.containsKey("mode")) setGA(ga["mode"], pimpl->modeGA);
        if (ga.containsKey("heating")) setGA(ga["heating"], pimpl->heatingGA);
    }

    return validateConfig();
}

bool KNXInterface::validateConfig() const {
    if (!pimpl) return false;

    // Validate physical address
    if (pimpl->physicalAddress.area > 15 || 
        pimpl->physicalAddress.line > 15 || 
        pimpl->physicalAddress.member > 255) {
        return false;
    }

    // Validate all group addresses
    return validateGroupAddress(pimpl->temperatureGA) &&
           validateGroupAddress(pimpl->humidityGA) &&
           validateGroupAddress(pimpl->pressureGA) &&
           validateGroupAddress(pimpl->setpointGA) &&
           validateGroupAddress(pimpl->valveGA) &&
           validateGroupAddress(pimpl->modeGA) &&
           validateGroupAddress(pimpl->heatingGA);
}

void KNXInterface::getConfig(JsonDocument& config) const {
    if (!pimpl) return;

    // Physical address
    auto& pa = config.createNestedObject("physical_address");
    pa["area"] = pimpl->physicalAddress.area;
    pa["line"] = pimpl->physicalAddress.line;
    pa["member"] = pimpl->physicalAddress.member;

    // Group addresses
    auto& ga = config.createNestedObject("group_addresses");
    auto addGA = [](JsonObject& obj, const GroupAddress& ga) {
        obj["area"] = ga.area;
        obj["line"] = ga.line;
        obj["member"] = ga.member;
    };

    addGA(ga.createNestedObject("temperature"), pimpl->temperatureGA);
    addGA(ga.createNestedObject("humidity"), pimpl->humidityGA);
    addGA(ga.createNestedObject("pressure"), pimpl->pressureGA);
    addGA(ga.createNestedObject("setpoint"), pimpl->setpointGA);
    addGA(ga.createNestedObject("valve"), pimpl->valveGA);
    addGA(ga.createNestedObject("mode"), pimpl->modeGA);
    addGA(ga.createNestedObject("heating"), pimpl->heatingGA);
}

bool KNXInterface::sendTemperature(float value) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->temperatureGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendHumidity(float value) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->humidityGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendPressure(float value) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->pressureGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendSetpoint(float value) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->setpointGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendValvePosition(float value) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->valveGA);
    // Convert 0-100 float to 0-255 uint8
    uint8_t scaledValue = static_cast<uint8_t>(value * 2.55f);
    pimpl->knx->write_1byte_uint(addr, scaledValue);
    return true;
}

bool KNXInterface::sendMode(ThermostatMode mode) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->modeGA);
    uint8_t knxMode = modeToKnx(mode);
    pimpl->knx->write_1byte_uint(addr, knxMode);
    return true;
}

bool KNXInterface::sendHeatingState(bool isHeating) {
    if (!isConnected()) return false;
    EspKnxIp::address_t addr = pimpl->gaToAddress(pimpl->heatingGA);
    pimpl->knx->write_1bit(addr, isHeating);
    return true;
}

ThermostatStatus KNXInterface::getLastError() const {
    return pimpl ? pimpl->lastError : ThermostatStatus::ERROR_INITIALIZATION;
}

const char* KNXInterface::getLastErrorMessage() const {
    return pimpl ? pimpl->lastErrorMsg : "KNX interface not initialized";
}

void KNXInterface::clearError() {
    if (pimpl) {
        pimpl->clearError();
    }
}

void KNXInterface::registerCallbacks(ThermostatState* state, ProtocolManager* manager) {
    if (!pimpl) return;
    
    pimpl->thermostatState = state;
    pimpl->protocolManager = manager;

    if (isConnected()) {
        setupCallbacks();
    }
}

void KNXInterface::unregisterCallbacks() {
    if (!pimpl) return;
    
    cleanupCallbacks();
    pimpl->thermostatState = nullptr;
    pimpl->protocolManager = nullptr;
}

void KNXInterface::setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
    if (!pimpl) return;
    pimpl->physicalAddress = {area, line, member};
    if (isConnected()) {
        pimpl->knx->begin(area, line, member);
    }
}

void KNXInterface::setTemperatureGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->temperatureGA = ga;
}

void KNXInterface::setHumidityGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->humidityGA = ga;
}

void KNXInterface::setPressureGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->pressureGA = ga;
}

void KNXInterface::setSetpointGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->setpointGA = ga;
}

void KNXInterface::setValvePositionGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->valveGA = ga;
}

void KNXInterface::setModeGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->modeGA = ga;
}

void KNXInterface::setHeatingStateGA(const GroupAddress& ga) {
    if (!pimpl || !validateGroupAddress(ga)) return;
    pimpl->heatingGA = ga;
}

bool KNXInterface::validateGroupAddress(const GroupAddress& ga) const {
    return ga.area <= 31 && ga.line <= 7 && ga.member <= 255;
}

void KNXInterface::setupCallbacks() {
    if (!pimpl || !pimpl->knx || !pimpl->thermostatState || !pimpl->protocolManager) return;

    cleanupCallbacks();

    // Register for setpoint updates
    EspKnxIp::address_t setpointAddr = pimpl->gaToAddress(pimpl->setpointGA);
    pimpl->setpointCallbackId = pimpl->knx->callback_register("setpoint", 
        [this](EspKnxIp::message_t const &msg, void *) {
            if (msg.ct == EspKnxIp::KNX_CT_WRITE && pimpl->thermostatState && pimpl->protocolManager) {
                float setpoint = pimpl->knx->data_to_2byte_float(msg.data);
                pimpl->protocolManager->handleIncomingCommand(
                    CommandType::CMD_SET_TEMPERATURE,
                    setpoint,
                    getCommandSource()
                );
            }
        }
    );
    pimpl->knx->callback_assign(pimpl->setpointCallbackId, setpointAddr);

    // Register for mode updates
    EspKnxIp::address_t modeAddr = pimpl->gaToAddress(pimpl->modeGA);
    pimpl->modeCallbackId = pimpl->knx->callback_register("mode",
        [this](EspKnxIp::message_t const &msg, void *) {
            if (msg.ct == EspKnxIp::KNX_CT_WRITE && pimpl->thermostatState && pimpl->protocolManager) {
                uint8_t modeValue = pimpl->knx->data_to_1byte_uint(msg.data);
                ThermostatMode mode = knxToMode(modeValue);
                pimpl->protocolManager->handleIncomingCommand(
                    CommandType::CMD_SET_MODE,
                    static_cast<float>(static_cast<int>(mode)),
                    getCommandSource()
                );
            }
        }
    );
    pimpl->knx->callback_assign(pimpl->modeCallbackId, modeAddr);
}

void KNXInterface::cleanupCallbacks() {
    if (!pimpl || !pimpl->knx) return;

    if (pimpl->setpointCallbackId) {
        pimpl->knx->callback_deregister(pimpl->setpointCallbackId);
        pimpl->setpointCallbackId = 0;
    }

    if (pimpl->modeCallbackId) {
        pimpl->knx->callback_deregister(pimpl->modeCallbackId);
        pimpl->modeCallbackId = 0;
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
