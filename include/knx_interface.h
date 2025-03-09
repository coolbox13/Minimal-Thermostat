#ifndef KNX_INTERFACE_H
#define KNX_INTERFACE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <memory>

// Include our interfaces and types first
#include "interfaces/protocol_interface.h"
#include "thermostat_types.h"
#include "protocol_types.h"

// Forward declarations
class ThermostatState;
class ProtocolManager;

// Forward declare KNX library types
namespace EspKnxIp {
    class EspKnxIp;
    struct message_t;
    struct address_t;
}

// Protocol source identifier
#define SOURCE_KNX 1

// Command identifiers
#define CMD_SET_TEMPERATURE 1
#define CMD_SET_MODE 2

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

class KNXInterface : public ProtocolInterface {
public:
    // Constructor and destructor
    KNXInterface();
    virtual ~KNXInterface();

    // ProtocolInterface implementation
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
    struct GroupAddress {
        uint8_t area;
        uint8_t line;
        uint8_t member;
    };

    void setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member);
    void setTemperatureGA(const GroupAddress& ga);
    void setHumidityGA(const GroupAddress& ga);
    void setPressureGA(const GroupAddress& ga);
    void setSetpointGA(const GroupAddress& ga);
    void setValvePositionGA(const GroupAddress& ga);
    void setModeGA(const GroupAddress& ga);
    void setHeatingStateGA(const GroupAddress& ga);

private:
    // Forward declare private implementation
    class Impl;
    std::unique_ptr<Impl> pimpl;

    // Internal helpers
    static uint8_t modeToKnx(ThermostatMode mode);
    static ThermostatMode knxToMode(uint8_t value);
    bool validateGroupAddress(const GroupAddress& ga) const;
    void setupCallbacks();
    void cleanupCallbacks();
};

#endif // KNX_INTERFACE_H