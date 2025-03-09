#ifndef KNX_INTERFACE_H
#define KNX_INTERFACE_H

#include <Arduino.h>
#include "interfaces/protocol_interface.h"
#include "thermostat_state.h"
#include "protocol_manager.h"

// Forward declarations
class ProtocolManager;

// Enum for thermostat modes
enum ThermostatMode {
  MODE_OFF = 0,
  MODE_COMFORT = 1,
  MODE_ECO = 2,
  MODE_AWAY = 3,
  MODE_BOOST = 4,
  MODE_ANTIFREEZE = 5
};

class KNXInterface : public ProtocolInterface {
public:
  // Constructor
  KNXInterface();

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
  void handleKNXEvent(uint16_t address, uint8_t* data, uint8_t length);
  bool sendValue(uint8_t area, uint8_t line, uint8_t member, const void* data, uint8_t length);
};

#endif // KNX_INTERFACE_H