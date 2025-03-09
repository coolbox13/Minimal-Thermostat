#ifndef KNX_INTERFACE_H
#define KNX_INTERFACE_H

#include <memory>
#include <ArduinoJson.h>
#include <esp-knx-ip.h>
#include "interfaces/protocol_interface.h"
#include "protocol_types.h"
#include "thermostat_types.h"

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
    // Constructor
    KNXInterface();
    
    // Destructor
    virtual ~KNXInterface();
    
    // Core functionality
    bool begin() override;
    void loop() override;
    bool isConnected() const override;
    void disconnect() override;
    bool reconnect() override;
    
    // Connection configuration
    bool configure(const JsonDocument& config) override;
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
    ThermostatStatus getLastError() const override;
    const char* getLastErrorMessage() const override;
    void clearError() override;
    
    // Protocol registration
    void registerCallbacks(ThermostatState* state, ProtocolManager* manager) override;
    void unregisterCallbacks() override;
    
    // Protocol identification
    const char* getProtocolName() const override { return "KNX"; }
    CommandSource getCommandSource() const override { return CommandSource::SOURCE_KNX; }
    
    // KNX specific methods
    void setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member);
    void setTemperatureGA(const KnxGroupAddress& ga);
    void setHumidityGA(const KnxGroupAddress& ga);
    void setPressureGA(const KnxGroupAddress& ga);
    void setSetpointGA(const KnxGroupAddress& ga);
    void setValvePositionGA(const KnxGroupAddress& ga);
    void setModeGA(const KnxGroupAddress& ga);
    void setHeatingStateGA(const KnxGroupAddress& ga);
    
private:
    // Private implementation
    class Impl;
    std::unique_ptr<Impl> pimpl;
    
    // Helper methods
    bool validateGroupAddress(const KnxGroupAddress& ga) const;
    void setupCallbacks();
    void cleanupCallbacks();
    uint8_t modeToKnx(ThermostatMode mode) const;
    ThermostatMode knxToMode(uint8_t value) const;
};

#endif // KNX_INTERFACE_H 