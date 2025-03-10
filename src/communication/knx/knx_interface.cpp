#include <Arduino.h>
#include <ArduinoJson.h>
#include "esp-knx-ip.h"
#include "communication/knx/knx_interface.h"
#include "protocol_manager.h"
#include "thermostat_state.h"
#include <esp_log.h>
#include <map>
#include <string>

static const char* TAG = "KNXInterface";

// Implementation class definition
class KNXInterface::Impl {
public:
    Impl(ThermostatState* state) : state(state), enabled(false), lastError(ThermostatStatus::OK) {
        memset(lastErrorMessage, 0, sizeof(lastErrorMessage));
    }
    
    bool begin() {
        if (!enabled) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "KNX interface not enabled");
            lastError = ThermostatStatus::ERROR_CONFIGURATION;
            return false;
        }
        
        if (!knx.start(nullptr)) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Failed to start KNX interface");
            lastError = ThermostatStatus::ERROR_COMMUNICATION;
            return false;
        }
        
        return true;
    }
    
    void loop() {
        if (enabled) {
            knx.loop();
        }
    }
    
    bool configure(const JsonDocument& config) {
        // Configure KNX interface
        if (config.containsKey("physical")) {
            JsonObject physical = config["physical"].as<JsonObject>();
            uint8_t area = physical["area"] | 1;
            uint8_t line = physical["line"] | 1;
            uint8_t device = physical["device"] | 1;
            
            address_t addr;
            addr.bytes.high = (area << 4) | line;
            addr.bytes.low = device;
            knx.physical_address_set(addr);
        }
        
        enabled = true;
        return true;
    }
    
    bool isConnected() const {
        return enabled;
    }
    
    ThermostatStatus getLastError() const {
        return lastError;
    }
    
    const char* getLastErrorMessage() const {
        return lastErrorMessage;
    }
    
    void clearError() {
        lastError = ThermostatStatus::OK;
        memset(lastErrorMessage, 0, sizeof(lastErrorMessage));
    }
    
    bool setPhysicalAddress(uint8_t area, uint8_t line, uint8_t device) {
        address_t addr;
        addr.bytes.high = (area << 4) | line;
        addr.bytes.low = device;
        knx.physical_address_set(addr);
        return true;
    }
    
    bool setGroupAddress(uint8_t main, uint8_t middle, uint8_t sub, const char* name) {
        groupAddresses[name] = {main, middle, sub};
        return true;
    }
    
    bool sendValue(const char* gaName, float value) {
        if (!enabled) return false;
        
        auto it = groupAddresses.find(gaName);
        if (it == groupAddresses.end()) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Group address '%s' not found", gaName);
            lastError = ThermostatStatus::ERROR_CONFIGURATION;
            return false;
        }
        
        const auto& ga = it->second;
        address_t addr;
        addr.bytes.high = (ga.main << 3) | ga.middle;
        addr.bytes.low = ga.sub;
        knx.write_2byte_float(addr, value);
        return true;
    }
    
    bool sendStatus(const char* gaName, bool status) {
        if (!enabled) return false;
        
        auto it = groupAddresses.find(gaName);
        if (it == groupAddresses.end()) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Group address '%s' not found", gaName);
            lastError = ThermostatStatus::ERROR_CONFIGURATION;
            return false;
        }
        
        const auto& ga = it->second;
        address_t addr;
        addr.bytes.high = (ga.main << 3) | ga.middle;
        addr.bytes.low = ga.sub;
        knx.write_1bit(addr, status);
        return true;
    }
    
    // Make these public for access from KNXInterface
    ESPKNXIP knx;
    bool enabled;
    ThermostatStatus lastError;
    char lastErrorMessage[128];
    
    struct GroupAddress {
        uint8_t main;
        uint8_t middle;
        uint8_t sub;
    };
    
    std::map<std::string, GroupAddress> groupAddresses;
    ThermostatState* state;
};

// KNXInterface implementation
KNXInterface::KNXInterface(ThermostatState* state) : state(state) {
    pimpl = std::unique_ptr<Impl>(new Impl(state));
    Serial2.println("KNX interface created");
}

KNXInterface::~KNXInterface() {
    // Destructor implementation
}

bool KNXInterface::begin() {
    return pimpl ? pimpl->begin() : false;
}

void KNXInterface::loop() {
    if (pimpl) pimpl->loop();
}

bool KNXInterface::configure(const JsonDocument& config) {
    return pimpl ? pimpl->configure(config) : false;
}

bool KNXInterface::isConnected() const {
    return pimpl ? pimpl->isConnected() : false;
}

ThermostatStatus KNXInterface::getLastError() const {
    return pimpl ? pimpl->getLastError() : ThermostatStatus::ERROR_CONFIGURATION;
}

bool KNXInterface::setPhysicalAddress(uint8_t area, uint8_t line, uint8_t device) {
    return pimpl ? pimpl->setPhysicalAddress(area, line, device) : false;
}

bool KNXInterface::setGroupAddress(uint8_t main, uint8_t middle, uint8_t sub, const char* name) {
    return pimpl ? pimpl->setGroupAddress(main, middle, sub, name) : false;
}

bool KNXInterface::sendValue(const char* gaName, float value) {
    return pimpl ? pimpl->sendValue(gaName, value) : false;
}

bool KNXInterface::sendStatus(const char* gaName, bool status) {
    return pimpl ? pimpl->sendStatus(gaName, status) : false;
}

// Core functionality
bool KNXInterface::reconnect() {
    return begin();
}

void KNXInterface::disconnect() {
    if (pimpl) {
        pimpl->enabled = false;
    }
}

bool KNXInterface::validateConfig() const {
    // Simplified implementation
    return true;
}

void KNXInterface::getConfig(JsonDocument& config) const {
    // Simplified implementation
    JsonObject knx = config.createNestedObject("knx");
    
    // Physical address
    JsonObject physical = knx.createNestedObject("physical");
    physical["area"] = pimpl->knx.physical_address_get().area;
    physical["line"] = pimpl->knx.physical_address_get().line;
    physical["device"] = pimpl->knx.physical_address_get().device;
    
    // Group addresses
    JsonObject ga = knx.createNestedObject("ga");
    
    // Helper function to add group addresses
    auto addGA = [](JsonObject& obj, const KnxGroupAddress& ga) {
        obj["area"] = ga.area;
        obj["line"] = ga.line;
        obj["device"] = ga.device;
    };
    
    // Add all group addresses
    for (const auto& pair : pimpl->groupAddresses) {
        JsonObject obj = ga.createNestedObject(pair.first.c_str());
        addGA(obj, pair.second);
    }
}

// Data transmission
bool KNXInterface::sendTemperature(float value) {
    // Simplified implementation
    return true;
}

bool KNXInterface::sendHumidity(float value) {
    // Simplified implementation
    return true;
}

bool KNXInterface::sendPressure(float value) {
    // Simplified implementation
    return true;
}

bool KNXInterface::sendSetpoint(float value) {
    // Simplified implementation
    return true;
}

bool KNXInterface::sendValvePosition(float value) {
    // Simplified implementation
    return true;
}

bool KNXInterface::sendMode(ThermostatMode mode) {
    // Simplified implementation
    return true;
}

bool KNXInterface::sendHeatingState(bool isHeating) {
    // Simplified implementation
    return true;
}

// Protocol registration
void KNXInterface::registerCallbacks(ThermostatState* thermostatState, ProtocolManager* manager) {
    if (pimpl) {
        this->state = thermostatState;
        this->registerProtocolManager(manager);
    }
}

void KNXInterface::unregisterCallbacks() {
    if (pimpl) {
        this->state = nullptr;
        this->registerProtocolManager(nullptr);
    }
}

void KNXInterface::registerProtocolManager(ProtocolManager* manager) {
    if (pimpl) {
        this->protocolManager = manager;
    }
}

// KNX specific methods
void KNXInterface::setTemperatureGA(uint8_t area, uint8_t line, uint8_t device) {
    pimpl->knx.groupWrite2ByteFloat(area, line, device, 0.0f);
}

void KNXInterface::setHumidityGA(const KnxGroupAddress& ga) {
    pimpl->knx.groupWrite2ByteFloat(ga.area, ga.line, ga.device, 0.0f);
}

void KNXInterface::setPressureGA(const KnxGroupAddress& ga) {
    pimpl->knx.groupWrite2ByteFloat(ga.area, ga.line, ga.device, 0.0f);
}

void KNXInterface::setSetpointGA(uint8_t area, uint8_t line, uint8_t device) {
    pimpl->knx.groupWrite2ByteFloat(area, line, device, 0.0f);
}

void KNXInterface::setValvePositionGA(const KnxGroupAddress& ga) {
    pimpl->knx.groupWrite2ByteFloat(ga.area, ga.line, ga.device, 0.0f);
}

void KNXInterface::setModeGA(uint8_t area, uint8_t line, uint8_t device) {
    pimpl->knx.groupWriteBool(area, line, device, false);
}

void KNXInterface::setHeatingStateGA(const KnxGroupAddress& ga) {
    pimpl->knx.groupWriteBool(ga.area, ga.line, ga.device, false);
}

// Helper methods
bool KNXInterface::validateGroupAddress(const KnxGroupAddress& ga) const {
    // Simplified implementation
    return true;
}

void KNXInterface::setupCallbacks() {
    // Simplified implementation
}

void KNXInterface::cleanupCallbacks() {
    // Simplified implementation
}

uint8_t KNXInterface::modeToKnx(ThermostatMode mode) const {
    // Simplified implementation
    return static_cast<uint8_t>(mode);
}

ThermostatMode KNXInterface::knxToMode(uint8_t value) const {
    // Simplified implementation
    return static_cast<ThermostatMode>(value);
} 