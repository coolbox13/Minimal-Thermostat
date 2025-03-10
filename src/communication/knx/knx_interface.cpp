#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp-knx-ip.h>
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
        
        knx.start(nullptr);  // We don't need web server integration
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
            // Direct access without conversion
            uint8_t area = config["physical"]["area"] | 1;
            uint8_t line = config["physical"]["line"] | 1;
            uint8_t member = config["physical"]["member"] | 1;
            
            knx.physical_address_set(knx.PA_to_address(area, line, member));
        }
        
        // Configure group addresses
        if (config.containsKey("ga")) {
            for (JsonPair kv : config["ga"].template as<JsonObject>()) {
                const char* name = kv.key().c_str();
                JsonVariant val = kv.value();
                
                // Direct access to avoid JsonObject conversion
                uint8_t main = val["main"] | 0;
                uint8_t middle = val["middle"] | 0;
                uint8_t sub = val["sub"] | 0;
                
                address_t addr = knx.GA_to_address(main, middle, sub);
                groupAddresses[name] = addr;
            }
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
    
    bool setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member) {
        knx.physical_address_set(knx.PA_to_address(area, line, member));
        return true;
    }
    
    bool setGroupAddress(const char* name, uint8_t main, uint8_t middle, uint8_t sub) {
        address_t addr = knx.GA_to_address(main, middle, sub);
        groupAddresses[name] = addr;
        return true;
    }
    
    bool sendValue(const char* gaName, float value) {
        auto it = groupAddresses.find(gaName);
        if (it == groupAddresses.end()) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Group address %s not found", gaName);
            lastError = ThermostatStatus::ERROR_CONFIGURATION;
            return false;
        }
        
        knx.write_2byte_float(it->second, value);
        return true;
    }
    
    bool sendStatus(const char* gaName, bool status) {
        auto it = groupAddresses.find(gaName);
        if (it == groupAddresses.end()) {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Group address %s not found", gaName);
            lastError = ThermostatStatus::ERROR_CONFIGURATION;
            return false;
        }
        
        knx.write_1bit(it->second, status);
        return true;
    }
    
    // Make KNXInterface a friend class so it can access private members
    friend class KNXInterface;
    
private:
    ESPKNXIP knx;
    bool enabled;
    ThermostatStatus lastError;
    char lastErrorMessage[128];
    ThermostatState* state;
    std::map<std::string, address_t> groupAddresses;
};

// Use raw pointer initialization
KNXInterface::KNXInterface(ThermostatState* state) : pimpl(new Impl(state)) {}
KNXInterface::~KNXInterface() = default;

bool KNXInterface::begin() {
    return pimpl->begin();
}

void KNXInterface::loop() {
    pimpl->loop();
}

bool KNXInterface::configure(const JsonDocument& config) {
    return pimpl->configure(config);
}

bool KNXInterface::isConnected() const {
    return pimpl->isConnected();
}

ThermostatStatus KNXInterface::getLastError() const {
    return pimpl->getLastError();
}

const char* KNXInterface::getLastErrorMessage() const {
    return pimpl->getLastErrorMessage();
}

void KNXInterface::clearError() {
    pimpl->clearError();
}

void KNXInterface::getConfig(JsonDocument& config) const {
    JsonObject knx = config.createNestedObject("knx");
    
    JsonObject physical = knx.createNestedObject("physical");
    address_t addr = pimpl->knx.physical_address_get();
    physical["area"] = addr.pa.area;
    physical["line"] = addr.pa.line;
    physical["member"] = addr.pa.member;
    
    JsonObject ga = knx.createNestedObject("ga");
    for (const auto& pair : pimpl->groupAddresses) {
        JsonObject obj = ga.createNestedObject(pair.first.c_str());
        // Extract KNX group address components
        obj["main"] = pair.second.ga.area;
        obj["middle"] = pair.second.ga.line;
        obj["sub"] = pair.second.ga.member;
    }
}

void KNXInterface::setTemperatureGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("temperature", ga.main, ga.middle, ga.sub);
}

void KNXInterface::setHumidityGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("humidity", ga.main, ga.middle, ga.sub);
}

void KNXInterface::setPressureGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("pressure", ga.main, ga.middle, ga.sub);
}

void KNXInterface::setSetpointGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("setpoint", ga.main, ga.middle, ga.sub);
}

void KNXInterface::setValvePositionGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("valve", ga.main, ga.middle, ga.sub);
}

void KNXInterface::setModeGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("mode", ga.main, ga.middle, ga.sub);
}

void KNXInterface::setHeatingStateGA(const KnxGroupAddress& ga) {
    pimpl->setGroupAddress("heating", ga.main, ga.middle, ga.sub);
}

bool KNXInterface::sendTemperature(float value) {
    return pimpl->sendValue("temperature", value);
}

bool KNXInterface::sendHumidity(float value) {
    return pimpl->sendValue("humidity", value);
}

bool KNXInterface::sendPressure(float value) {
    return pimpl->sendValue("pressure", value);
}

bool KNXInterface::sendSetpoint(float value) {
    return pimpl->sendValue("setpoint", value);
}

bool KNXInterface::sendValvePosition(float value) {
    return pimpl->sendValue("valve", value);
}

bool KNXInterface::sendMode(ThermostatMode mode) {
    return pimpl->sendValue("mode", static_cast<float>(mode));
}

bool KNXInterface::sendHeatingState(bool isHeating) {
    return pimpl->sendStatus("heating", isHeating);
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