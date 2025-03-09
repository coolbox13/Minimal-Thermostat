#pragma once

#include <vector>
#include "interfaces/protocol_interface.h"
#include "thermostat_state.h"

class ProtocolManager {
public:
    ProtocolManager(ThermostatState* state);
    virtual ~ProtocolManager() = default;

    bool begin();
    void loop();
    void addProtocol(ProtocolInterface* protocol);
    void removeProtocol(ProtocolInterface* protocol);

private:
    ThermostatState* state;
    std::vector<ProtocolInterface*> protocols;
};