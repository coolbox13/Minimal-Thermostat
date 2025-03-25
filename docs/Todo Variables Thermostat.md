# ESP32-KNX-Thermostat Implementation Plan

## Overview

This document outlines the detailed task list for extending the ESP32-KNX-Thermostat's web interface to include configuration options for currently hardcoded settings. Implementation will proceed in phases, with KNX-related settings as the highest priority.

## Phase 1: KNX Configuration (High Priority)

### 1.1 Backend: ConfigManager Extension

- [ ] 1.1.1 Add group address getters/setters to `config_manager.h`
    - [ ] Temperature address (Main/Mid/Sub)
    - [ ] Humidity address (Main/Mid/Sub)
    - [ ] Pressure address (Main/Mid/Sub)
    - [ ] Valve address (Production, Main/Mid/Sub)
    - [ ] Valve address (Test, Main/Mid/Sub)
- [ ] 1.1.2 Implement methods in `config_manager.cpp`
    - [ ] Add default constants matching current hardcoded values
    - [ ] Implement getters with fallback to defaults
    - [ ] Implement setters with appropriate validation
- [ ] 1.1.3 Add KNX debug flag getter/setter
    - [ ] Add `getKnxDebugEnabled()` and `setKnxDebugEnabled(bool)`
    - [ ] Update logging code to respect this setting

### 1.2 KNX Manager Updates

- [ ] 1.2.1 Modify `setupAddresses()` in `knx_manager.cpp`
    - [ ] Replace hardcoded address values with ConfigManager calls
    - [ ] Add reloadAddresses() method to refresh from config
- [ ] 1.2.2 Update address initialization in constructor
- [ ] 1.2.3 Add debug/logging improvements for address usage

### 1.3 Web Interface Updates

- [ ] 1.3.1 Extend KNX tab in `data/config.html`
    - [ ] Add group address input fields with proper validation
    - [ ] Add toggle for KNX debug mode
    - [ ] Ensure proper labels and hints
- [ ] 1.3.2 Update `data/config.js`
    - [ ] Add functions to load KNX addresses from API
    - [ ] Extend save function to include new KNX fields
    - [ ] Add field validation for KNX addresses
    - [ ] Update error handling for KNX configuration

### 1.4 API Extensions

- [ ] 1.4.1 Update `/api/config` GET handler
    - [ ] Include all KNX group addresses in response
    - [ ] Add KNX debug flag to response
- [ ] 1.4.2 Update `/api/config` POST handler
    - [ ] Parse KNX address fields from request
    - [ ] Validate address values
    - [ ] Store updated values in ConfigManager
    - [ ] Trigger KNXManager address reload

### 1.5 Testing Phase 1

- [ ] 1.5.1 Test persistence of KNX settings
- [ ] 1.5.2 Verify address changes are applied at runtime
- [ ] 1.5.3 Test KNX communication using configured addresses
- [ ] 1.5.4 Test debug flag functionality

## Phase 2: System Parameters

### 2.1 MQTT Authentication

- [ ] 2.1.1 Add MQTT credential getters/setters to ConfigManager
    - [ ] Add `getMqttUsername()` and `setMqttUsername(String)`
    - [ ] Add `getMqttPassword()` and `setMqttPassword(String)`
- [ ] 2.1.2 Update MQTT connection code
    - [ ] Modify `mqtt_manager.cpp` to use credentials from config
    - [ ] Ensure secure handling of credentials

### 2.2 PID Controller Parameters

- [ ] 2.2.1 Add PID parameter getters/setters to ConfigManager
    - [ ] Add `getPidDeadband()` and `setPidDeadband(float)`
    - [ ] Add `getPidUpdateInterval()` and `setPidUpdateInterval(uint32_t)`
- [ ] 2.2.2 Update PID controller code
    - [ ] Modify `updatePIDController()` to use configurable interval
    - [ ] Update PID initialization to use configurable deadband

### 2.3 Watchdog Configuration

- [ ] 2.3.1 Add watchdog getters/setters to ConfigManager
    - [ ] Add `getSystemWatchdogTimeout()` and `setSystemWatchdogTimeout(uint32_t)`
    - [ ] Add `getWifiWatchdogTimeout()` and `setWifiWatchdogTimeout(uint32_t)`
    - [ ] Add `getMaxReconnectAttempts()` and `setMaxReconnectAttempts(uint8_t)`
    - [ ] Add `getMaxConsecutiveResets()` and `setMaxConsecutiveResets(uint8_t)`
- [ ] 2.3.2 Update watchdog code
    - [ ] Modify `watchdog_manager.cpp` to use configuration values
    - [ ] Ensure proper validation of timeout values

### 2.4 Sensor Update Interval

- [ ] 2.4.1 Add sensor interval getter/setter to ConfigManager
    - [ ] Add `getSensorUpdateInterval()` and `setSensorUpdateInterval(uint32_t)`
- [ ] 2.4.2 Update main loop to use configurable interval
    - [ ] Modify sensor update check in `main.cpp`

### 2.5 Web Interface Updates

- [ ] 2.5.1 Add MQTT credentials to MQTT tab
    - [ ] Add username and password fields with appropriate security
- [ ] 2.5.2 Add PID parameters to PID tab
    - [ ] Add deadband field with validation
    - [ ] Add update interval field with validation
- [ ] 2.5.3 Create new "System" tab
    - [ ] Add watchdog timeout fields
    - [ ] Add max attempts fields
    - [ ] Add sensor update interval field
- [ ] 2.5.4 Update JavaScript
    - [ ] Add functions to load new fields from API
    - [ ] Extend save function for new parameters
    - [ ] Add appropriate validation

### 2.6 API Extensions

- [ ] 2.6.1 Update `/api/config` GET handler
    - [ ] Include all new parameters in response
- [ ] 2.6.2 Update `/api/config` POST handler
    - [ ] Parse new fields from request
    - [ ] Validate values
    - [ ] Store in ConfigManager

### 2.7 Testing Phase 2

- [ ] 2.7.1 Test MQTT authentication
- [ ] 2.7.2 Verify PID behavior with custom parameters
- [ ] 2.7.3 Test watchdog functionality with custom timeouts
- [ ] 2.7.4 Verify sensor update frequency changes

## Phase 3: Advanced Settings

### 3.1 MQTT Topic Structure

- [ ] 3.1.1 Add MQTT topic getters/setters to ConfigManager
    - [ ] Add base topic getter/setter
    - [ ] Add methods for individual topic components
- [ ] 3.1.2 Update MQTT manager
    - [ ] Refactor to use configurable topics
    - [ ] Update subscription logic

### 3.2 Home Assistant Integration

- [ ] 3.2.1 Add HA settings to ConfigManager
    - [ ] Add `getHaDiscoveryPrefix()` and setter
    - [ ] Add HA integration enable/disable toggle
- [ ] 3.2.2 Update HomeAssistant integration class
    - [ ] Use configurable prefix
    - [ ] Respect enable/disable setting

### 3.3 Web Interface Updates

- [ ] 3.3.1 Add MQTT topic configuration
    - [ ] Create fields for topic template
    - [ ] Add preview of resulting topics
- [ ] 3.3.2 Add Home Assistant settings
    - [ ] Add discovery prefix field
    - [ ] Add integration toggle

### 3.4 API Extensions

- [ ] 3.4.1 Update API handlers for new settings
- [ ] 3.4.2 Add validation for topic strings

### 3.5 Testing Phase 3

- [ ] 3.5.1 Verify MQTT topic changes
- [ ] 3.5.2 Test Home Assistant discovery with custom prefix
- [ ] 3.5.3 Verify backwards compatibility

## Final Integration and Testing

### 4.1 Integration Testing

- [ ] 4.1.1 Complete system test with all parameters modified
- [ ] 4.1.2 Test upgrade path from previous version
- [ ] 4.1.3 Performance testing with modified parameters

### 4.2 Documentation

- [ ] 4.2.1 Update README with new configuration options
- [ ] 4.2.2 Update web interface help text
- [ ] 4.2.3 Document default/recommended values

### 4.3 Release Preparation

- [ ] 4.3.1 Final code review
- [ ] 4.3.2 Version number update
- [ ] 4.3.3 Create release package

## Progress Tracking

| Phase | Started | Completed | Notes |
| ----- | ------- | --------- | ----- |
| 1.1   |         |           |       |
| 1.2   |         |           |       |
| 1.3   |         |           |       |
| 1.4   |         |           |       |
| 1.5   |         |           |       |
| 2.1   |         |           |       |
| 2.2   |         |           |       |
| 2.3   |         |           |       |
| 2.4   |         |           |       |
| 2.5   |         |           |       |
| 2.6   |         |           |       |
| 2.7   |         |           |       |
| 3.1   |         |           |       |
| 3.2   |         |           |       |
| 3.3   |         |           |       |
| 3.4   |         |           |       |
| 3.5   |         |           |       |
| 4.1   |         |           |       |
| 4.2   |         |           |       |
| 4.3   |         |           |       |
