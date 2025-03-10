#pragma once

#include <memory>
#include <ArduinoJson.h>
#include "communication/knx/knx_interface_fix.h"
#include "interfaces/protocol_interface.h"
#include "protocol_types.h"
#include "thermostat_types.h"
#include "thermostat_state.h"
#include "esp-knx-ip.h"

// Forward declarations
class ThermostatState;
class ProtocolManager;

// KNX group address structure
struct KnxGroupAddress {
    uint8_t area;   // 0-31 (5 bits)
    uint8_t line;   // 0-7  (3 bits)
    uint8_t member; // 0-255 (8 bits)

    KnxGroupAddress(uint8_t a = 0, uint8_t l = 0, uint8_t m = 0)
        : area(a), line(l), member(m) {}
};

class KNXInterface : public ProtocolInterface {
public:
    // Forward declaration of implementation class
    class Impl;
    
    KNXInterface(ThermostatState* state);
    ~KNXInterface();
    
    // ProtocolInterface methods
    bool begin() override;
    void loop() override;
    bool configure(const JsonDocument& config) override;
    bool isConnected() const override;
    ThermostatStatus getLastError() const override;
    
    // KNX specific methods
    bool setPhysicalAddress(uint8_t area, uint8_t line, uint8_t device);
    bool setGroupAddress(uint8_t main, uint8_t middle, uint8_t sub, const char* name);
    bool sendValue(const char* gaName, float value);
    bool sendStatus(const char* gaName, bool status);
    
    // Core functionality
    void disconnect() override;
    bool reconnect() override;
    
    // Connection configuration
    bool validateConfig() const override;
    void getConfig(JsonDocument& config) const override;
    
    // Data transmission
    bool sendTemperature(float value) override;
    bool sendHumidity(float value) override;
    bool sendPressure(float value) override;
    bool sendSetpoint(float value) override;
    bool sendValvePosition(float value) override;
    bool sendMode(ThermostatMode mode) override;
    bool sendHeatingState(bool isHeating) override;
    
    // Error handling
    const char* getLastErrorMessage() const override;
    void clearError() override;
    
    // Protocol registration
    void registerCallbacks(ThermostatState* state, ProtocolManager* manager) override;
    void unregisterCallbacks() override;
    
    // Protocol identification
    const char* getProtocolName() const override { return "KNX"; }
    CommandSource getCommandSource() const override { return CommandSource::SOURCE_KNX; }
    
    // KNX specific methods
    void setTemperatureGA(uint8_t area, uint8_t line, uint8_t member);
    void setHumidityGA(const KnxGroupAddress& ga);
    void setPressureGA(const KnxGroupAddress& ga);
    void setSetpointGA(uint8_t area, uint8_t line, uint8_t member);
    void setValvePositionGA(const KnxGroupAddress& ga);
    void setModeGA(uint8_t area, uint8_t line, uint8_t member);
    void setHeatingStateGA(const KnxGroupAddress& ga);
    
    // Protocol manager registration
    void registerProtocolManager(ProtocolManager* manager);
    
private:
    ThermostatState* state;
    std::unique_ptr<Impl> pimpl;
    
    // Helper methods
    bool validateGroupAddress(const KnxGroupAddress& ga) const;
    void setupCallbacks();
    void cleanupCallbacks();
    uint8_t modeToKnx(ThermostatMode mode) const;
    ThermostatMode knxToMode(uint8_t value) const;

    ESPKNXIP knx;
    bool enabled;
    bool initialized;

    // Group addresses
    address_t temperatureGA;
    address_t setpointGA;
    address_t valveGA;
    address_t modeGA;

    void handleKnxEvent(message_t const &msg);
    void updateKnxValues();
}; 