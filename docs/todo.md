# WiFi & Watchdog Improvement Tasks

This document outlines the specific tasks required to implement the improvements described in the design plan for fixing WiFi reconnection and watchdog timer issues.

## Phase 1: WiFi Connection Management Refactoring

### 1. Create WiFiConnectionManager Class
- [x] Define new WiFiConnectionManager class in `wifi_connection.h`
- [x] Implement constructor and basic initialization
- [x] Create enum for connection states:
  ```cpp
  enum class WiFiConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    CONFIG_PORTAL_ACTIVE,
    CONNECTION_LOST
  };
  ```
- [x] Add private member variables:
  - [x] Connection state tracking
  - [x] Last connected time
  - [x] Reconnection attempt counter
  - [x] Signal strength history

### 2. Implement Core WiFi Functions
- [x] Move WiFi initialization from `setupWiFi()` to the manager
- [x] Implement `begin()` method that initializes WiFi with stored credentials
- [x] Create `connect()` method with timeout parameter
- [x] Implement `startConfigPortal()` method
- [x] Add `getStatus()` method to retrieve current connection status
- [x] Create `getSignalStrength()` method to get current RSSI
- [x] Add internet connectivity testing functionality
- [x] Implement detailed connection reporting

### 3. Add Callback System
- [x] Define callback function types for connection events
- [x] Add callback registration methods
- [x] Implement state change notifications
- [x] Set up WiFiManager callbacks:
  - [x] AP mode start callback
  - [x] Save configuration callback
  - [x] Connection success callback

### 4. Improve Reconnection Strategy
- [x] Implement exponential backoff algorithm for reconnection attempts
- [x] Add maximum reconnection limit with config portal fallback
- [x] Create mechanism to disable WiFi watchdog during intentional operations
- [x] Add connectivity testing beyond WiFi connection state

### 5. Integrate with Main Application
- [x] Update `main.cpp` to use new WiFiConnectionManager
- [x] Replace `setupWiFi()` and `checkWiFiConnection()` with manager calls
- [x] Ensure proper initialization sequence with other components
- [x] Validate state transitions behave as expected 

## Phase 2: Watchdog Timer Integration

### 1. Define Watchdog Configuration
- [ ] Add clear watchdog timeout definitions to `config.h`:
  ```cpp
  // Existing 45-minute system watchdog
  #define SYSTEM_WATCHDOG_TIMEOUT 2700000 
  // New 15-minute WiFi watchdog
  #define WIFI_WATCHDOG_TIMEOUT 900000 
  // Maximum reconnection attempts before reboot
  #define MAX_RECONNECT_ATTEMPTS 10
  ```
- [ ] Update existing watchdog code to use these definitions consistently

### 2. Create WatchdogManager Class
- [ ] Define new WatchdogManager class in `watchdog_manager.h`
- [ ] Implement constructor and initialization
- [ ] Create methods to:
  - [ ] Initialize system watchdog
  - [ ] Reset system watchdog
  - [ ] Enable/disable WiFi watchdog
  - [ ] Register reboot reasons

### 3. Implement Hierarchical Watchdog Strategy
- [ ] Configure ESP task watchdog for system-level monitoring
- [ ] Implement WiFi-specific watchdog in the WatchdogManager
- [ ] Create coordination mechanism between the two watchdog levels
- [ ] Add selective watchdog disabling for long operations

### 4. Add Recovery Mechanisms
- [ ] Implement staged recovery approach:
  - [ ] Attempt WiFi reconnection
  - [ ] Reset WiFi subsystem without full reboot
  - [ ] Perform controlled system reboot
- [ ] Create persistent storage for reboot reasons
- [ ] Add "safe mode" boot option after consecutive watchdog resets
- [ ] Implement network connectivity test before continuing operation

### 5. Integrate Watchdog with WiFiConnectionManager
- [ ] Connect WiFiConnectionManager state changes to watchdog reset
- [ ] Ensure watchdog is disabled during config portal
- [ ] Implement proper watchdog reset on successful connections
- [ ] Add logging of watchdog state changes

## Phase 3: Logging and Diagnostics

### 1. Enhance WiFi and Watchdog Logging
- [ ] Create dedicated log tags for WiFi and watchdog events
- [ ] Add detailed state transition logging in WiFiConnectionManager
- [ ] Log signal strength periodically
- [ ] Create persistent log for reboot reasons
- [ ] Add log for connectivity test results

### 2. Implement Connectivity Diagnostics
- [ ] Add internet connectivity test (DNS or HTTP ping)
- [ ] Create signal quality tracking
- [ ] Implement connection quality assessment algorithm
- [ ] Add periodic connectivity status reporting

### 3. Update Web Interface
- [ ] Add WiFi connection status to web dashboard
- [ ] Create signal strength indicator
- [ ] Add watchdog status display
- [ ] Show reboot history and reasons
- [ ] Add manual reconnect option

## Phase 4: Testing and Validation

### 1. Test WiFi Reconnection
- [ ] Test automatic reconnection after WiFi loss
- [ ] Verify exponential backoff works correctly
- [ ] Confirm config portal activates after maximum reconnection attempts
- [ ] Validate state transitions through all connection states

### 2. Test Watchdog Functionality
- [ ] Verify system watchdog triggers reboot after timeout
- [ ] Test WiFi watchdog recovery steps
- [ ] Confirm watchdog disabling works during operations like OTA
- [ ] Validate reboot reason logging

### 3. Stress Testing
- [ ] Test system with intermittent WiFi connection
- [ ] Verify long-term stability (24+ hour test)
- [ ] Test recovery from completely lost WiFi network
- [ ] Validate behavior when moving between WiFi networks

### 4. Documentation
- [ ] Update code comments for new classes
- [ ] Add detailed documentation for WiFiConnectionManager
- [ ] Create documentation for WatchdogManager
- [ ] Add system behavior notes to project README
- [ ] Document recovery procedures for users

## Implementation Notes

1. **Keep Backward Compatibility:**
   - Maintain support for existing configuration formats
   - Ensure upgrading devices transition smoothly to new logic

2. **Error Handling:**
   - Add comprehensive error handling for all new functionality
   - Implement fallbacks for all critical operations

3. **Performance Considerations:**
   - Ensure WiFi monitoring doesn't impact main loop timing
   - Optimize memory usage in new manager classes
   - Consider using tasks for network operations if needed

4. **Configuration:**
   - Make timeouts and retry counts configurable 
   - Allow users to adjust sensitivity of watchdog mechanisms
   - Provide option to disable automatic reboot if desired
