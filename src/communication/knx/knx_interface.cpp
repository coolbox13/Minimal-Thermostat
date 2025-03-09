#include <Arduino.h>
#include <memory>
#include <esp-knx-ip.h>
#include "communication/knx/knx_interface.h"
#include "thermostat_state.h"
#include "protocol_manager.h"
#include "thermostat_types.h"

// KNX DPT (Data Point Type) definitions as constexpr for type safety
namespace KnxDPT {
    constexpr float TEMPERATURE = 9.001f;  // 2-byte float
    constexpr float HUMIDITY = 9.007f;     // 2-byte float
    constexpr float PRESSURE = 9.006f;     // 2-byte float
    constexpr float SETPOINT = 9.001f;     // 2-byte float
    constexpr float VALVE = 5.001f;        // 8-bit unsigned (0-100%)
    constexpr float MODE = 20.102f;        // 8-bit unsigned (HVAC mode)
    constexpr float BOOL = 1.001f;         // 1-bit boolean
}

class KNXInterface::Impl {
public:
    Impl() : knx(std::make_unique<EspKnxIp>()) {
        knx->setBufferSize(512); // Increase buffer size for larger messages
    }
    
    // KNX library instance
    std::unique_ptr<EspKnxIp> knx;
    
    // Physical address
    struct {
        uint8_t area = 1;
        uint8_t line = 1;
        uint8_t member = 1;
    } physicalAddress;
    
    // Group addresses
    KnxGroupAddress temperatureGA{3, 0, 1};  // Default: 3/0/1
    KnxGroupAddress humidityGA{3, 1, 1};     // Default: 3/1/1
    KnxGroupAddress pressureGA{3, 2, 1};     // Default: 3/2/1
    KnxGroupAddress setpointGA{3, 3, 1};     // Default: 3/3/1
    KnxGroupAddress valveGA{3, 4, 1};        // Default: 3/4/1
    KnxGroupAddress modeGA{3, 5, 1};         // Default: 3/5/1
    KnxGroupAddress heatingGA{3, 6, 1};      // Default: 3/6/1
    
    // Callback IDs
    int setpointCallbackId = -1;
    int modeCallbackId = -1;
    
    // State
    ThermostatState* thermostatState = nullptr;
    ProtocolManager* protocolManager = nullptr;
    ThermostatStatus lastError = ThermostatStatus::OK;
    char lastErrorMessage[128] = {0};
    
    // Helper methods
    address_t gaToAddress(const KnxGroupAddress& ga) const {
        address_t addr;
        addr.ga.area = ga.area;
        addr.ga.line = ga.line;
        addr.ga.member = ga.member;
        return addr;
    }
    
    void setError(ThermostatStatus status, const char* message) {
        lastError = status;
        if (message) {
            strncpy(lastErrorMessage, message, sizeof(lastErrorMessage) - 1);
            lastErrorMessage[sizeof(lastErrorMessage) - 1] = '\0';
        } else {
            lastErrorMessage[0] = '\0';
        }
    }
};

KNXInterface::KNXInterface() : pimpl(std::make_unique<Impl>()) {
}

KNXInterface::~KNXInterface() = default;

bool KNXInterface::begin() {
    if (!pimpl || !pimpl->knx) {
        pimpl->setError(ThermostatStatus::ERROR_INITIALIZATION, "KNX implementation not initialized");
        return false;
    }
    
    if (!pimpl->knx->begin(pimpl->physicalAddress.area,
                          pimpl->physicalAddress.line,
                          pimpl->physicalAddress.member)) {
        pimpl->setError(ThermostatStatus::ERROR_INITIALIZATION, "Failed to initialize KNX interface");
        return false;
    }
    
    setupCallbacks();
    return true;
}

void KNXInterface::loop() {
    if (pimpl && pimpl->knx) {
        pimpl->knx->loop();
    }
}

bool KNXInterface::isConnected() const {
    return pimpl && pimpl->knx && pimpl->knx->connected();
}

void KNXInterface::disconnect() {
    if (pimpl && pimpl->knx) {
        pimpl->knx->stop();
    }
}

bool KNXInterface::reconnect() {
    return begin();
}

bool KNXInterface::configure(const JsonDocument& config) {
    if (!pimpl) return false;

    if (config.containsKey("physical_address")) {
        const JsonObject& pa = config["physical_address"].as<JsonObject>();
        if (pa.containsKey("area")) pimpl->physicalAddress.area = pa["area"];
        if (pa.containsKey("line")) pimpl->physicalAddress.line = pa["line"];
        if (pa.containsKey("member")) pimpl->physicalAddress.member = pa["member"];
    }

    if (config.containsKey("group_addresses")) {
        const JsonObject& ga = config["group_addresses"].as<JsonObject>();
        auto setGA = [](KnxGroupAddress& target, const JsonObject& obj) {
            if (obj.containsKey("area")) target.area = obj["area"];
            if (obj.containsKey("line")) target.line = obj["line"];
            if (obj.containsKey("member")) target.member = obj["member"];
        };

        if (ga.containsKey("temperature")) setGA(pimpl->temperatureGA, ga["temperature"].as<JsonObject>());
        if (ga.containsKey("humidity")) setGA(pimpl->humidityGA, ga["humidity"].as<JsonObject>());
        if (ga.containsKey("pressure")) setGA(pimpl->pressureGA, ga["pressure"].as<JsonObject>());
        if (ga.containsKey("setpoint")) setGA(pimpl->setpointGA, ga["setpoint"].as<JsonObject>());
        if (ga.containsKey("valve")) setGA(pimpl->valveGA, ga["valve"].as<JsonObject>());
        if (ga.containsKey("mode")) setGA(pimpl->modeGA, ga["mode"].as<JsonObject>());
        if (ga.containsKey("heating")) setGA(pimpl->heatingGA, ga["heating"].as<JsonObject>());
    }

    return true;
}

bool KNXInterface::validateConfig() const {
    if (!pimpl) return false;
    
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

    JsonObject pa = config.createNestedObject("physical_address");
    pa["area"] = pimpl->physicalAddress.area;
    pa["line"] = pimpl->physicalAddress.line;
    pa["member"] = pimpl->physicalAddress.member;

    JsonObject ga = config.createNestedObject("group_addresses");
    auto addGA = [](JsonObject& obj, const KnxGroupAddress& ga) {
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
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->temperatureGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendHumidity(float value) {
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->humidityGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendPressure(float value) {
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->pressureGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendSetpoint(float value) {
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->setpointGA);
    pimpl->knx->write_2byte_float(addr, value);
    return true;
}

bool KNXInterface::sendValvePosition(float value) {
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->valveGA);
    uint8_t scaledValue = static_cast<uint8_t>(value * 255.0f / 100.0f);
    pimpl->knx->write_1byte_uint(addr, scaledValue);
    return true;
}

bool KNXInterface::sendMode(ThermostatMode mode) {
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->modeGA);
    uint8_t knxMode = modeToKnx(mode);
    pimpl->knx->write_1byte_uint(addr, knxMode);
    return true;
}

bool KNXInterface::sendHeatingState(bool isHeating) {
    if (!pimpl || !pimpl->knx) return false;
    address_t addr = pimpl->gaToAddress(pimpl->heatingGA);
    pimpl->knx->write_1bit(addr, isHeating);
    return true;
}

ThermostatStatus KNXInterface::getLastError() const {
    return pimpl ? pimpl->lastError : ThermostatStatus::ERROR_INITIALIZATION;
}

const char* KNXInterface::getLastErrorMessage() const {
    return pimpl ? pimpl->lastErrorMessage : "KNX interface not initialized";
}

void KNXInterface::clearError() {
    if (pimpl) {
        pimpl->lastError = ThermostatStatus::OK;
        pimpl->lastErrorMessage[0] = '\0';
    }
}

void KNXInterface::registerCallbacks(ThermostatState* state, ProtocolManager* manager) {
    if (!pimpl) return;
    
    pimpl->thermostatState = state;
    pimpl->protocolManager = manager;
    setupCallbacks();
}

void KNXInterface::unregisterCallbacks() {
    if (!pimpl) return;
    
    cleanupCallbacks();
    pimpl->thermostatState = nullptr;
    pimpl->protocolManager = nullptr;
}

void KNXInterface::setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
    if (pimpl) {
        pimpl->physicalAddress.area = area;
        pimpl->physicalAddress.line = line;
        pimpl->physicalAddress.member = member;
        
        if (pimpl->knx) {
            pimpl->knx->begin(area, line, member);
        }
    }
}

void KNXInterface::setTemperatureGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->temperatureGA = ga;
}

void KNXInterface::setHumidityGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->humidityGA = ga;
}

void KNXInterface::setPressureGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->pressureGA = ga;
}

void KNXInterface::setSetpointGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->setpointGA = ga;
}

void KNXInterface::setValvePositionGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->valveGA = ga;
}

void KNXInterface::setModeGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->modeGA = ga;
}

void KNXInterface::setHeatingStateGA(const KnxGroupAddress& ga) {
    if (pimpl) pimpl->heatingGA = ga;
}

bool KNXInterface::validateGroupAddress(const KnxGroupAddress& ga) const {
    return ga.area <= 31 && ga.line <= 7 && ga.member <= 255;
}

void KNXInterface::setupCallbacks() {
    if (!pimpl || !pimpl->knx) return;

    // Register setpoint callback
    address_t setpointAddr = pimpl->gaToAddress(pimpl->setpointGA);
    pimpl->setpointCallbackId = pimpl->knx->callback_register("setpoint",
        [this](message_t const &msg, void *) {
            if (msg.ct == KNX_CT_WRITE && pimpl->thermostatState && pimpl->protocolManager) {
                float setpoint = pimpl->knx->data_to_2byte_float(msg.data);
                pimpl->protocolManager->handleIncomingCommand(
                    CommandSource::SOURCE_KNX,
                    CommandType::CMD_SETPOINT,
                    setpoint
                );
            }
        }
    );
    pimpl->knx->callback_assign(pimpl->setpointCallbackId, setpointAddr);

    // Register mode callback
    address_t modeAddr = pimpl->gaToAddress(pimpl->modeGA);
    pimpl->modeCallbackId = pimpl->knx->callback_register("mode",
        [this](message_t const &msg, void *) {
            if (msg.ct == KNX_CT_WRITE && pimpl->thermostatState && pimpl->protocolManager) {
                uint8_t modeValue = pimpl->knx->data_to_1byte_uint(msg.data);
                ThermostatMode mode = knxToMode(modeValue);
                pimpl->protocolManager->handleIncomingCommand(
                    CommandSource::SOURCE_KNX,
                    CommandType::CMD_MODE,
                    static_cast<float>(mode)
                );
            }
        }
    );
    pimpl->knx->callback_assign(pimpl->modeCallbackId, modeAddr);
}

void KNXInterface::cleanupCallbacks() {
    if (!pimpl || !pimpl->knx) return;

    if (pimpl->setpointCallbackId >= 0) {
        pimpl->knx->callback_deregister(pimpl->setpointCallbackId);
        pimpl->setpointCallbackId = -1;
    }

    if (pimpl->modeCallbackId >= 0) {
        pimpl->knx->callback_deregister(pimpl->modeCallbackId);
        pimpl->modeCallbackId = -1;
    }
}

uint8_t KNXInterface::modeToKnx(ThermostatMode mode) const {
    switch (mode) {
        case ThermostatMode::MODE_OFF:
            return 0;
        case ThermostatMode::MODE_HEAT:
            return 1;
        case ThermostatMode::MODE_COOL:
            return 2;
        case ThermostatMode::MODE_AUTO:
            return 3;
        default:
            return 0;
    }
}

ThermostatMode KNXInterface::knxToMode(uint8_t value) const {
    switch (value) {
        case 0:
            return ThermostatMode::MODE_OFF;
        case 1:
            return ThermostatMode::MODE_HEAT;
        case 2:
            return ThermostatMode::MODE_COOL;
        case 3:
            return ThermostatMode::MODE_AUTO;
        default:
            return ThermostatMode::MODE_OFF;
    }
} 