# Hardcoded Settings Analysis

| Variable                         | Default Value           | Hardcoded Only (No Change Option) | Available via Web Interface | Changeable via MQTT |
| -------------------------------- | ----------------------- | --------------------------------- | --------------------------- | ------------------- |
| **Network Configuration**        |                         |                                   |                             |                     |
| MQTT Server IP                   | "192.168.178.32"        | No                                | Yes                         | No                  |
| MQTT Port                        | 1883                    | No                                | Yes                         | No                  |
| MQTT Username                    | ""                      | Yes                               | ==No==                      | No                  |
| MQTT Password                    | ""                      | Yes                               | ==No==                      | No                  |
| MQTT Topic Structure             | "thermostat/..."        | Yes                               | ==No==                      | No                  |
| Web Server Port                  | 80                      | Yes                               | ==No==                      | No                  |
| WiFi Signal Check Interval       | 60000ms                 | Yes                               | No                          | No                  |
| WiFi Quality Thresholds          | -60/-70 dBm             | Yes                               | No                          | No                  |
| DNS Test Host                    | "8.8.8.8"               | Yes                               | No                          | No                  |
| WiFi Connection Timeout          | 10000ms                 | Yes                               | No                          | No                  |
| **KNX Configuration**            |                         |                                   |                             |                     |
| KNX Physical Address             | 1.1.159                 | No                                | Yes                         | No                  |
| KNX Group Addresses (Production) | Various                 | Yes                               | ==No==                      | No                  |
| KNX Group Addresses (Test)       | Various                 | Yes                               | ==No==                      | No                  |
| Use Test Addresses Flag          | true/false              | No                                | Yes                         | No                  |
| KNX Debug Flag                   | 0                       | Yes                               | ==No==                      | No                  |
| **Sensor Configuration**         |                         |                                   |                             |                     |
| BME280 I²C Address               | 0x76                    | No                                | Yes                         | No                  |
| BME280 SDA/SCL Pins              | 21/22                   | No                                | Yes                         | No                  |
| Sensor Update Interval           | 30000ms                 | Yes                               | ==No==                      | No                  |
| **PID Controller**               |                         |                                   |                             |                     |
| PID Kp (Proportional)            | 2.0                     | No                                | Yes                         | No                  |
| PID Ki (Integral)                | 0.1                     | No                                | Yes                         | No                  |
| PID Kd (Derivative)              | 0.5                     | No                                | Yes                         | No                  |
| PID Deadband                     | 0.2°C                   | Yes                               | ==No==                      | No                  |
| PID Update Interval              | 10000ms                 | Yes                               | ==No==                      | No                  |
| Temperature Setpoint             | 22.0°C                  | No                                | Yes                         | Yes                 |
| **System Settings**              |                         |                                   |                             |                     |
| System Watchdog Timeout          | 2700000ms (45min)       | Yes                               | ==No==                      | No                  |
| WiFi Watchdog Timeout            | 1800000ms (30min)       | Yes                               | ==No==                      | No                  |
| Max Reconnect Attempts           | 10                      | Yes                               | ==No==                      | No                  |
| Max Consecutive Resets           | 3                       | Yes                               | ==No==                      | No                  |
| Home Assistant Discovery Prefix  | "homeassistant"         | Yes                               | ==No==                      | No                  |
| Persistence Namespaces           | "thermostat"/"watchdog" | Yes                               | ==No==                      | No                  |
| Valve Position                   | 0-100%                  | No                                | Yes                         | Yes                 |
| WiFi SSID/Password               | ""                      | No                                | Yes                         | No                  |

## Summary of Fully Hardcoded Settings (No Change Option)

1. **MQTT Credentials**: Username and password
2. **MQTT Topic Structure**: All topic paths hardcoded
3. **Web Server Port**: Fixed at port 80
4. **WiFi Diagnostics**: Signal check intervals, quality thresholds, test host
5. **KNX Group Addresses**: Both production and test addresses
6. **KNX Debug Flag**: Can only be changed at compile time
7. **Sensor Update Interval**: Fixed at 30 seconds
8. **PID Deadband**: Fixed at 0.2°C
9. **PID Update Interval**: Fixed at 10 seconds
10. **Watchdog Timeouts**: System (45min) and WiFi (30min)
11. **Max Reconnect/Reset Attempts**: 10 reconnects, 3 resets
12. **Home Assistant Integration**: Discovery prefix hardcoded
13. **Persistence Namespaces**: Storage namespaces are hardcoded

# High-Level Design Plan: Adding Hardcoded Settings to Web Interface

## Overview

This plan focuses on extending the thermostat's web interface to include configuration options for currently hardcoded settings. Priority is given to KNX-related settings, followed by other system parameters.

## Architecture Considerations

The implementation will follow the existing pattern:

1. Add settings to `ConfigManager` for persistent storage
2. Extend API endpoints in `WebServerManager`
3. Update HTML/JavaScript in the web interface
4. Modify the underlying components to use these configurable values

## Implementation Plan

### Phase 1: KNX Configuration (Highest Priority)

#### 1. Extended KNX Group Address Storage

```cpp
// Add to config_manager.h
// KNX Production addresses
uint8_t getKnxTempMainGA();
uint8_t getKnxTempMidGA();
uint8_t getKnxTempSubGA();
// Similar for humidity, pressure, valve...

void setKnxTempMainGA(uint8_t main);
// Similar setters for other addresses
```

#### 2. Update ConfigManager Implementation

```cpp
// In config_manager.cpp
uint8_t ConfigManager::getKnxTempMainGA() {
    return _preferences.getUChar("knx_temp_main", KNX_GA_TEMPERATURE_MAIN);
}

void ConfigManager::setKnxTempMainGA(uint8_t main) {
    _preferences.putUChar("knx_temp_main", main);
}
```

#### 3. Update KNXManager to Use Configurable Addresses

```cpp
// In knx_manager.cpp, setupAddresses()
_temperatureAddress = _knx.GA_to_address(
    _configManager->getKnxTempMainGA(),
    _configManager->getKnxTempMidGA(),
    _configManager->getKnxTempSubGA()
);
```

#### 4. Extend Web Interface Config Page

Update `data/config.html` to add KNX address fields:

```html
<!-- In the KNX settings tab -->
<h4>Group Addresses</h4>
<div class="form-group">
    <label>Temperature Sensor:</label>
    <div class="knx-address-inputs">
        <input type="number" name="knx_temp_main" min="0" max="31">
        <span>/</span>
        <input type="number" name="knx_temp_mid" min="0" max="7">
        <span>/</span>
        <input type="number" name="knx_temp_sub" min="0" max="255">
    </div>
</div>
<!-- Similar for other addresses -->
```

#### 5. Update API Endpoint

```javascript
// In web_server.cpp, update the config API handler
// Inside the "/api/config" POST handler
if (jsonDoc.containsKey("knx") && jsonDoc["knx"].containsKey("addresses")) {
    auto addresses = jsonDoc["knx"]["addresses"];
    if (addresses.containsKey("temperature")) {
        auto temp = addresses["temperature"];
        configManager->setKnxTempMainGA(temp["main"]);
        configManager->setKnxTempMidGA(temp["mid"]);
        configManager->setKnxTempSubGA(temp["sub"]);
    }
    // Similarly for other addresses
}
```

### Phase 2: System Configuration Parameters

#### 1. Add MQTT Authentication Settings

Extend `ConfigManager` with MQTT username/password:

```cpp
String getMqttUsername();
String getMqttPassword();
void setMqttUsername(const String& username);
void setMqttPassword(const String& password);
```

Add to `mqtt_manager.cpp`:

```cpp
// When connecting to MQTT broker
mqttClient.connect(clientId.c_str(), 
    configManager->getMqttUsername().c_str(), 
    configManager->getMqttPassword().c_str());
```

#### 2. Add PID & Watchdog Parameters

```cpp
// In config_manager.h
float getPidDeadband();
void setPidDeadband(float deadband);
uint32_t getPidUpdateInterval();
void setPidUpdateInterval(uint32_t interval);
// Similar for watchdog timeouts and reconnect attempts
```

#### 3. Update Web Interface

Modify `config.html` to add new system settings:

```html
<!-- PID Settings Tab -->
<div class="form-row">
    <label for="pid_deadband">Deadband (±°C):</label>
    <input type="number" id="pid_deadband" name="pid_deadband" 
           step="0.1" min="0" max="5" class="form-input">
    <span class="form-hint">Range: 0-5°C (0.1 steps)</span>
</div>
<div class="form-row">
    <label for="pid_interval">Update Interval:</label>
    <input type="number" id="pid_interval" name="pid_interval" 
           min="1000" max="60000" step="1000" class="form-input">
    <span class="form-hint">Milliseconds (1000-60000)</span>
</div>

<!-- System Settings Tab (new) -->
<button class="tab-button" data-tab="system">System</button>
<div id="system" class="tab-content">
    <h3>System Settings</h3>
    <!-- Watchdog settings -->
    <div class="form-row">
        <label for="system_watchdog">System Watchdog:</label>
        <input type="number" id="system_watchdog" name="system_watchdog" 
               min="60" max="3600" class="form-input">
        <span class="form-hint">Seconds (60-3600)</span>
    </div>
    <!-- Similar for other system settings -->
</div>
```

#### 4. Update JavaScript

In `config.js`, add handling for new fields:

```javascript
// In saveConfiguration() function
const config = {
    // Existing configuration...
    system: {
        watchdog_timeout: parseInt(formData.get('system_watchdog')) * 1000,
        wifi_watchdog_timeout: parseInt(formData.get('wifi_watchdog')) * 1000,
        max_reconnect_attempts: parseInt(formData.get('max_reconnect'))
    },
    mqtt: {
        // Existing MQTT config...
        username: formData.get('mqtt_username'),
        password: formData.get('mqtt_password')
    },
    pid: {
        // Existing PID config...
        deadband: parseFloat(formData.get('pid_deadband')),
        update_interval: parseInt(formData.get('pid_interval'))
    }
};
```

### Phase 3: Update Component Code

For each component, modify the code to use the config values instead of hardcoded constants:

```cpp
// In adaptive_pid_controller.cpp
void updatePIDController() {
    // Use configurable interval
    if (currentTime - lastPIDUpdate > configManager->getPidUpdateInterval()) {
        // Use configurable deadband
        g_pid_input.deadband = configManager->getPidDeadband();
        // ...rest of function
    }
}
```

## Implementation Strategy

1. **Incremental Changes**: Implement one section at a time, testing thoroughly before moving to the next.
2. **Backward Compatibility**: Ensure defaults match current hardcoded values for smooth upgrades.
3. **UI Organization**: Group related settings logically, with clear labels and input constraints.
4. **Client-Side Validation**: Add JavaScript validation to ensure input limits are respected.
5. **Error Handling**: Provide clear feedback for invalid settings and recovery options.

## Testing Plan

1. Verify KNX addresses change correctly on the KNX bus
2. Test MQTT reconnection with credentials
3. Confirm PID behavior with modified parameters
4. Validate system stability with adjusted watchdog timings

## Timeline Estimation

- Phase 1 (KNX configuration): 1-2 days
- Phase 2 (System parameters): 2-3 days
- Phase 3 (Component updates): 1-2 days
- Testing and refinement: 2-3 days

Total: Approximately 6-10 days of development effort


