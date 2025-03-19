# Design Plan: Fixing KNX Address Configuration and PID Parameter Persistence

This design plan addresses two key issues in the ESP32-KNX-Thermostat codebase:

1. **KNX Address Configuration Inconsistency**: The current implementation uses a compile-time flag for KNX address selection, while offering a runtime toggle in the UI that's not properly applied.

2. **PID Controller Parameter Persistence**: There are two separate mechanisms for storing PID parameters, leading to potential inconsistencies.

## 1. KNX Address Configuration Solution

### Current Issue
- Compile-time flag `USE_KNX_TEST_ADDRESSES` in `config.h` determines which address set is used
- Web interface has a "Use Test Addresses" toggle that is stored in ConfigManager
- KNXManager doesn't read the runtime configuration, only the compile-time flag. I wannt to be able to choose if i want to use a dummy knx address or a real one. So the 2nd option is to use a dummy address for testing purposes. Both test address and dummy address should be configurable.

### Proposed Solution
1. **Remove Compile-Time Flag**:
   - Remove the `USE_KNX_TEST_ADDRESSES` define from `config.h`
   - Make the toggle fully runtime-based

2. **Centralize KNX Address Management**:
   - Modify KNXManager to read test/production flag from ConfigManager at runtime
   - Implement a mechanism to update addresses when the toggle changes
   - Add proper function to read the current status

3. **Ensure Persistence**:
   - Use the "Use Test Addresses" setting from ConfigManager consistently
   - Ensure proper saving to NVS (non-volatile storage)

4. **Updates and Events**:
   - Create a mechanism to trigger address reconfiguration when the setting changes
   - Ensure proper validation and error handling

### Implementation Plan
1. Update `KNXManager` class to:
   - Add a method to read the current test/production mode from ConfigManager
   - Modify `setupAddresses()` to use this runtime setting
   - Add a method to reload addresses when the setting changes
   - Add callback registration for config changes

2. Modify `ConfigManager` to:
   - Ensure proper storage and retrieval of KNX address mode
   - Add notification mechanism for changes to the test address setting

3. Add proper UI feedback in the web interface

## 2. PID Controller Parameter Persistence Solution

### Current Issue
- Two separate mechanisms for storing PID parameters:
  - `ConfigManager` class (used by web UI)
  - `PersistenceManager` class (separate implementation)
- Parameters are loaded from ConfigManager but not properly synchronized with PersistenceManager
- Risk of parameter inconsistency and confusion about which values are authoritative

### Proposed Solution
1. **Single Source of Truth**:
   - Choose one storage mechanism (ConfigManager) as the authoritative source
   - Deprecate or remove redundant storage in PersistenceManager
   - Alternatively, make PersistenceManager wrap ConfigManager for PID values

2. **Synchronization**:
   - If both systems must be kept, implement proper synchronization
   - Ensure all PID parameter updates flow through a single update path

3. **Consistent Initialization**:
   - Ensure PID controller is initialized with parameters from the chosen storage system
   - Add validation for loaded parameters (min/max bounds, default fallbacks)

4. **Simplified API**:
   - Create clear, documented APIs for reading/writing PID parameters
   - Ensure all code uses these APIs rather than direct access

### Implementation Plan
1. Option A: Consolidate into ConfigManager
   - Move all PID-related storage to ConfigManager
   - Add additional parameters from PersistenceManager if required
   - Update all code to use ConfigManager APIs

2. Option B: Make PersistenceManager a wrapper
   - Modify PersistenceManager to delegate to ConfigManager
   - Maintain compatibility for existing code
   - Gradually migrate to using ConfigManager directly

3. Update initialization in `main.cpp` to:
   - Load parameters from the single source
   - Apply validation and defaults
   - Use a consistent method to update the PID controller

4. Document the chosen approach and update comments

## Implementation Sequence and Dependencies

1. **KNX Address Configuration**
   - Update KNXManager first
   - Test with manual configuration changes
   - Then integrate UI changes

2. **PID Parameter Persistence**
   - Choose and implement the storage approach first
   - Update initialization code
   - Then update any UI or control paths

## Testing Plan

1. **KNX Address Configuration Testing**
   - Verify correct addresses are used at startup based on saved configuration
   - Test toggling between test/production mode
   - Verify persistence across reboots
   - Test edge cases (invalid addresses, etc.)

2. **PID Parameter Persistence Testing**
   - Verify parameters are correctly loaded at startup
   - Test parameter updates via web UI
   - Test parameter updates via MQTT (if applicable)
   - Verify persistence across reboots
   - Test with extreme values to ensure validation works
