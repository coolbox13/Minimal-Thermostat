# TODO List for ESP32-KNX-Thermostat Fixes

## KNX Address Configuration Fix

- [ ] **Remove compile-time flag**
  - [ ] Edit `config.h` to remove `USE_KNX_TEST_ADDRESSES` define
  - [ ] Update any conditional code using this flag

- [ ] **Update KNXManager**
  - [ ] Modify `setupAddresses()` to read test mode from ConfigManager
  - [ ] Add a method `reloadAddresses()` to update addresses at runtime
  - [ ] Update `knxManager.begin()` to use the runtime setting

- [ ] **Update Web Interface**
  - [ ] Ensure UI toggle correctly updates ConfigManager
  - [ ] Add real-time feedback when toggle changes

- [ ] **Test KNX Addresses**
  - [ ] Test startup with test addresses enabled
  - [ ] Test startup with production addresses enabled
  - [ ] Test toggling during operation

## PID Parameter Persistence Fix

- [ ] **Choose ConfigManager as single source of truth**
  - [ ] Remove duplicate PID storage from PersistenceManager

- [ ] **Update PID Initialization**
  - [ ] Modify `initializePIDController()` to get all values from ConfigManager
  - [ ] Add parameter validation with fallback defaults

- [ ] **Update PID Parameter Update Flow**
  - [ ] Ensure all updates to PID parameters flow through ConfigManager
  - [ ] Update MQTT and KNX handlers to use ConfigManager

- [ ] **Update Web Interface for PID Parameters**
  - [ ] Ensure values are read from ConfigManager
  - [ ] Ensure valid input ranges are enforced

## Quick Implementation Notes

- Start with the simplest changes first
- Don't worry about perfect abstraction - we can refactor later
- Use existing code patterns where possible
- Add simple debug prints to verify changes are working
- Skip complex validation for now - focus on the happy path
- Keep changes targeted to just these two issues for now

## Testing Approach

- Manual testing with Serial monitoring is sufficient
- Verify changes persist across reboots
- Focus on normal operation scenarios first
- Minimal error handling for now
