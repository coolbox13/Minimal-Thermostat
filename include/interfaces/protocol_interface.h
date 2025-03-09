#ifndef PROTOCOL_INTERFACE_H
#define PROTOCOL_INTERFACE_H

#include <Arduino.h>

// Forward declarations
enum class ThermostatMode;
enum class ThermostatStatus;

class ProtocolInterface {
public:
    virtual ~ProtocolInterface() = default;

    // Core functionality
    virtual bool begin() = 0;
    virtual void loop() = 0;
    virtual bool isConnected() const = 0;

    // Data transmission
    virtual bool sendTemperature(float value) = 0;
    virtual bool sendHumidity(float value) = 0;
    virtual bool sendPressure(float value) = 0;
    virtual bool sendSetpoint(float value) = 0;
    virtual bool sendValvePosition(float value) = 0;
    virtual bool sendMode(ThermostatMode mode) = 0;
    virtual bool sendHeatingState(bool isHeating) = 0;

    // Error handling
    virtual ThermostatStatus getLastError() const = 0;
};

#endif // PROTOCOL_INTERFACE_H 