#ifndef KNX_INTERFACE_H
#define KNX_INTERFACE_H

#include <Arduino.h>
#include "interfaces/protocol_interface.h"
#include "thermostat_types.h"
#include "protocol_types.h"

// Forward declarations
class ThermostatState;
class ProtocolManager;

// Forward declare KNX library types
class EspKnxIp;
struct message_t;
struct address_t;

// Protocol source identifier
#define SOURCE_KNX 1

// Command identifiers
#define CMD_SET_TEMPERATURE 1
#define CMD_SET_MODE 2

// KNX DPT (Data Point Type) definitions
#define KNX_DPT_TEMPERATURE     9.001  // 2-byte float
#define KNX_DPT_HUMIDITY       9.007  // 2-byte float
#define KNX_DPT_PRESSURE       9.006  // 2-byte float
#define KNX_DPT_SETPOINT      9.001  // 2-byte float
#define KNX_DPT_VALVE         5.001  // 8-bit unsigned (0-100%)
#define KNX_DPT_MODE          20.102 // 8-bit unsigned (HVAC mode)
#define KNX_DPT_BOOL         1.001  // 1-bit boolean

class KNXInterface : public ProtocolInterface {
public:
    // Constructor and destructor
    KNXInterface();
    virtual ~KNXInterface();

    // ProtocolInterface implementation
    bool begin() override;
    void loop() override;
    bool isConnected() const override;
    bool sendTemperature(float value) override;
    bool sendHumidity(float value) override;
    bool sendPressure(float value) override;
    bool sendSetpoint(float value) override;
    bool sendValvePosition(float value) override;
    bool sendMode(ThermostatMode mode) override;
    bool sendHeatingState(bool isHeating) override;
    ThermostatStatus getLastError() const override;

    // KNX specific methods
    void setPhysicalAddress(uint8_t area, uint8_t line, uint8_t member);
    void setGroupAddress(uint8_t area, uint8_t line, uint8_t member);
    void registerCallbacks(ThermostatState* state, ProtocolManager* manager);

private:
    // Forward declare private implementation
    class Impl;
    Impl* pimpl;

    // KNX state
    bool connected;
    ThermostatStatus lastError;
    ThermostatState* thermostatState;
    ProtocolManager* protocolManager;

    // KNX callback IDs
    uint32_t setpointCallbackId;
    uint32_t modeCallbackId;

    // KNX addresses
    struct {
        uint8_t area;
        uint8_t line;
        uint8_t member;
    } physicalAddress;

    struct {
        uint8_t tempArea;
        uint8_t tempLine;
        uint8_t tempMember;
        uint8_t setpointArea;
        uint8_t setpointLine;
        uint8_t setpointMember;
        uint8_t valveArea;
        uint8_t valveLine;
        uint8_t valveMember;
        uint8_t modeArea;
        uint8_t modeLine;
        uint8_t modeMember;
    } groupAddresses;

    // Internal helpers
    static uint8_t modeToKnx(ThermostatMode mode);
    static ThermostatMode knxToMode(uint8_t value);
};

#endif // KNX_INTERFACE_H