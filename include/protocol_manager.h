#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include "thermostat_state.h"

// Forward declarations to resolve circular dependency
class KNXInterface;
class MQTTInterface;

// Different command sources to handle priority
enum CommandSource {
  SOURCE_LOCAL = 0,    // UI or button on device
  SOURCE_KNX = 1,      // KNX bus command
  SOURCE_MQTT = 2,     // MQTT command
  SOURCE_SCHEDULE = 3, // Scheduled change
  SOURCE_WEB_API = 4   // Web API call
};

// Types of commands that can be received
enum CommandType {
  CMD_SET_TEMPERATURE = 0,
  CMD_SET_MODE = 1,
  CMD_SET_VALVE_POSITION = 2
};

class ProtocolManager {
public:
  // Constructor
  ProtocolManager();
  
  // Initialize with thermostat state
  void begin(ThermostatState* state);
  
  // Register protocol interfaces
  void registerProtocols(KNXInterface* knx, MQTTInterface* mqtt);
  
  // Handle incoming commands from various sources
  void handleIncomingCommand(CommandSource source, CommandType cmd, float value);
  
  // Update a specific property across all protocols
  void broadcastState(ThermostatState* state);
  
  // Process loop (call in main loop)
  void loop();

private:
  ThermostatState* thermostatState;
  KNXInterface* knxInterface;
  MQTTInterface* mqttInterface;
  
  // Last command source for conflict resolution
  CommandSource lastCommandSource;
  unsigned long lastCommandTime;
  
  // Conflict resolution - return true if source a has higher priority than b
  bool hasPriority(CommandSource a, CommandSource b);
  
  // Command cooldown time in ms (prevents command loops)
  static constexpr unsigned long COMMAND_COOLDOWN = 1000;
};

#endif // PROTOCOL_MANAGER_H