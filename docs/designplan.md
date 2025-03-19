# Design Plan: Fixing WiFi Reconnection and Watchdog Timer Issues

## Current Issues Analysis

After a detailed analysis of the ESP32-KNX-Thermostat codebase, I've identified two related issues that affect system reliability:

### 1. WiFi Reconnection Logic Issue

**Problem**: The WiFi reconnection logic has potential bugs that could lead to the device getting stuck in a disconnected state:

- In `checkWiFiConnection()`, when multiple reconnection attempts fail, the code sets `configPortalActive = true` and starts the WiFiManager config portal.
- However, there's no mechanism to reset `configPortalActive` back to false when the connection is successfully established through the portal.
- This leads to a situation where `configPortalActive` remains true indefinitely, which:
  1. Prevents the WiFi watchdog timer from functioning correctly
  2. Could leave the device in an ambiguous state where it thinks the config portal is active even though it's connected

Additionally, the reconnection logic doesn't properly handle the WiFiManager's retry settings or provide clear feedback about connection status.

### 2. Watchdog Timer Implementation Issues

**Problem**: The current implementation uses two different watchdog mechanisms with inconsistent timeouts:

- The ESP task watchdog is initialized with a 45-minute timeout (`WATCHDOG_TIMEOUT` defined as 2700000ms in config.h)
- A custom WiFi watchdog is implemented with a 30-minute timeout (`WIFI_WATCHDOG_TIMEOUT` defined as 1800000ms in main.cpp)
- These two mechanisms are not coordinated, which could lead to unpredictable reboots depending on which timer triggers first
- The custom WiFi watchdog only checks for disconnection time but doesn't track whether WiFi connections are stable but not functional

Additionally, the system doesn't log enough information about watchdog states to help diagnose issues when they occur.

## Proposed Solutions

### 1. WiFi Reconnection Logic Improvements

1. **Proper Config Portal State Management**:
   - Add a proper state tracking mechanism for the WiFiManager config portal
   - Implement a callback for when WiFi connects successfully through the portal
   - Ensure `configPortalActive` is reset to false when connection is established

2. **Enhanced WiFiManager Integration**:
   - Utilize WiFiManager's built-in event callbacks properly
   - Implement the `setSaveConfigCallback()` to detect when new credentials are saved
   - Add a `setAPCallback()` to track when the access point is started

3. **Improved Reconnection Strategy**:
   - Implement an exponential backoff strategy for reconnection attempts
   - Add more granular logging during the reconnection process
   - Add a mechanism to detect "connected but no internet" scenarios

4. **Connection Quality Monitoring**:
   - Add periodic WiFi signal strength checking
   - Implement a connectivity test to verify not just WiFi connection but actual internet access
   - Store connectivity history to detect patterns of instability

### 2. Watchdog Timer Harmonization

1. **Unified Watchdog Approach**:
   - Consolidate the two watchdog mechanisms into a single, coordinated approach
   - Define a single watchdog timeout value in config.h to be used by both mechanisms
   - Implement a hierarchical watchdog strategy:
     * Level 1: WiFi connectivity watchdog (shorter timeout)
     * Level 2: System operation watchdog (longer timeout)

2. **Improved Watchdog Functionality**:
   - Add selective watchdog disabling during known long operations (e.g., OTA updates)
   - Implement a "soft reboot" attempt before triggering a hard reset
   - Add pre-reset logging to capture system state before reboot

3. **Recovery Mechanism Enhancements**:
   - Add persistent logging of reboot reasons
   - Implement a "safe mode" boot option after multiple watchdog-triggered reboots
   - Add a mechanism to test networking functionality before disabling watchdog

4. **Status Reporting**:
   - Add watchdog status to the web interface
   - Implement MQTT reporting of watchdog events
   - Create a dedicated log category for watchdog and connectivity events

## Implementation Plan

### Phase 1: Refactor WiFi Connection Management

1. Create a dedicated WiFiConnectionManager class to encapsulate all WiFi-related functionality:
   - Move WiFi initialization, monitoring, and reconnection logic into the class
   - Implement proper state tracking with an enum for connection states
   - Add callback registration for connection events

2. Implement improved WiFiManager callbacks:
   ```cpp
   // pseudocode
   wifiManager.setAPCallback([](WiFiManager* wifiManager) {
     configPortalActive = true;
     LOG_I(TAG_WIFI, "Config portal started at IP: %s", WiFi.softAPIP().toString().c_str());
   });

   wifiManager.setSaveConfigCallback([]() {
     LOG_I(TAG_WIFI, "Configuration saved, connection established");
     configPortalActive = false;
     lastConnectedTime = millis();
   });
   ```

3. Add WiFi connection state tracking:
   ```cpp
   enum class WiFiConnectionState {
     DISCONNECTED,
     CONNECTING,
     CONNECTED,
     CONFIG_PORTAL_ACTIVE,
     CONNECTION_LOST
   };
   ```

### Phase 2: Watchdog Timer Integration

1. Define a single watchdog configuration in config.h:
   ```cpp
   #define SYSTEM_WATCHDOG_TIMEOUT 2700000  // 45 minutes for system watchdog
   #define WIFI_WATCHDOG_TIMEOUT   900000   // 15 minutes for WiFi watchdog
   ```

2. Create a WatchdogManager class to coordinate all watchdog functionality:
   - Initialize system watchdog with ESP task watchdog
   - Track WiFi connectivity with custom WiFi watchdog
   - Add functions to temporarily disable watchdog during operations like OTA

3. Implement staged recovery in watchdog triggers:
   - Stage 1: Try to reconnect WiFi
   - Stage 2: Try soft reboot of WiFi subsystem
   - Stage 3: Full system reboot

### Phase 3: Logging and Diagnostics

1. Enhance logging for WiFi and watchdog events:
   - Add detailed state transition logging
   - Log WiFi signal strength periodically
   - Create persistent log of reboot reasons

2. Add connectivity diagnostics:
   - Implement ping test to verify internet connectivity
   - Log DNS resolution success/failure
   - Track latency as a connectivity quality indicator

3. Update web interface with connectivity status:
   - Add connection quality indicator
   - Show time since last reconnect
   - Display watchdog status

## Migration Strategy

1. Implement the changes incrementally:
   - First refactor WiFi connection management without changing behavior
   - Then improve the reconnection logic
   - Finally integrate the watchdog improvements

2. Add extensive testing at each stage:
   - Test reconnection with simulated WiFi failures
   - Verify config portal functionality 
   - Confirm watchdog triggers work as expected

3. Implement fallback mechanism:
   - Add ability to revert to old behavior via configuration
   - Keep backward compatibility with existing settings

4. Deploy and validate:
   - Test in development environment first
   - Monitor closely after deployment
   - Collect feedback on stability improvements

## Conclusion

The proposed redesign addresses both the WiFi reconnection logic issues and the watchdog timer inconsistencies. By creating dedicated managers for WiFi connection and watchdog functionality, we can improve code organization while also enhancing the reliability of the system. The staged approach to implementation ensures that each improvement builds on a stable foundation.

Once implemented, these changes should significantly improve the robustness of the ESP32-KNX-Thermostat against WiFi connectivity issues, reducing the likelihood of the device becoming unresponsive or stuck in an invalid state.
