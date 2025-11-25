# Home Assistant Integration Analysis

## Executive Summary

This document analyzes the ESP32-KNX-Thermostat integration with Home Assistant, comparing the implementation against expected HA Climate entity behavior. Several discrepancies and potential issues have been identified.

---

## 1. MQTT Topic Structure

### ESP32 Implementation

| Topic | Direction | Purpose |
|-------|-----------|---------|
| `esp32_thermostat/mode/set` | HA → ESP | Set HVAC mode (heat/off) |
| `esp32_thermostat/mode/state` | ESP → HA | Current HVAC mode |
| `esp32_thermostat/temperature/set` | HA → ESP | Set target temperature |
| `esp32_thermostat/temperature/setpoint` | ESP → HA | Current target temperature |
| `esp32_thermostat/temperature` | ESP → HA | Current room temperature |
| `esp32_thermostat/preset/set` | HA → ESP | Set preset mode |
| `esp32_thermostat/preset/state` | ESP → HA | Current preset mode |
| `esp32_thermostat/action` | ESP → HA | Current action (heating/idle) |
| `esp32_thermostat/valve/position` | ESP → HA | Valve position (0-100%) |
| `esp32_thermostat/status` | ESP → HA | Availability (online/offline) |

### HA MQTT Climate Expected Structure

The implementation follows the HA MQTT Climate specification correctly with proper topic naming.

**✅ CORRECT**: Topic structure aligns with HA expectations.

---

## 2. HVAC Modes Analysis

### ESP32 Implementation
- **Supported modes**: `["off", "heat"]`
- **Mode behavior**:
  - `heat`: Sets `thermostatEnabled = true`, allows PID control
  - `off`: Sets `thermostatEnabled = false`, sets valve to 0%

### HA Climate Entity Expected Modes
- `off`, `heat`, `cool`, `heat_cool`, `auto`, `dry`, `fan_only`

### Discrepancy Analysis

**✅ CORRECT**: For a heating-only thermostat, supporting only `off` and `heat` is appropriate.

**⚠️ POTENTIAL ISSUE**: The thermostat enabled flag (`thermostatEnabled`) is stored but **NOT checked in the main PID control loop**.

Looking at `main.cpp:updatePIDControl()`:
```cpp
void updatePIDControl() {
    // ...
    // Get current temperature from BME280
    float currentTemp = bme280.readTemperature();

    // NO CHECK for configManager->getThermostatEnabled() here!

    // PID controller is ALWAYS updated regardless of mode
    updatePIDController(currentTemp, valvePosition);
    finalValvePosition = getPIDOutput();

    // Valve position is ALWAYS applied
    knxManager.setValvePosition(finalValvePosition);
}
```

**🔴 CRITICAL ISSUE #1**: When HA sets mode to "off", the ESP32:
1. Sets `thermostatEnabled = false` in config
2. Publishes valve position 0 via MQTT (once)
3. **BUT** the main loop continues running PID control and may open the valve again!

The `thermostatEnabled` flag is set but never actually used to disable the PID controller.

---

## 3. HVAC Action Reporting

### ESP32 Implementation
```cpp
// In MQTTManager::setValvePosition()
if (_valvePosition > 0) {
    _mqttClient.publish("esp32_thermostat/action", "heating", true);
} else {
    _mqttClient.publish("esp32_thermostat/action", "idle", true);
}
```

### HA Climate Entity Expected Actions
- `off`, `preheating`, `heating`, `cooling`, `drying`, `idle`, `fan`, `defrosting`

### Analysis

**⚠️ ISSUE #2**: The action reporting logic is based solely on valve position:
- `valve > 0%` → `heating`
- `valve = 0%` → `idle`

**Missing behavior**: When mode is "off", the action should be `off`, not `idle`. Currently, if the thermostat is turned off but the valve is at 0%, it reports `idle` instead of `off`.

---

## 4. Preset Modes Analysis

### ESP32 Implementation
```cpp
// Supported presets (from home_assistant.cpp)
"pr_modes":[\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"]
```

Each preset maps to a stored temperature setpoint:
- `eco` → Lower temperature
- `comfort` → Standard comfort temperature
- `away` → Reduced temperature for away mode
- `sleep` → Night-time temperature
- `boost` → Maximum heating

### HA Climate Expected Presets
- `none`, `eco`, `away`, `boost`, `comfort`, `home`, `sleep`, `activity`

### Discrepancy

**⚠️ ISSUE #3**: The ESP32 does NOT include `none` in the published preset list, but it internally uses `none` as a valid state:
```cpp
// In home_assistant.cpp
const char* validPresets[] = {"none", "eco", "comfort", "away", "sleep", "boost"};
```

The discovery payload only advertises 5 presets (`eco`, `comfort`, `away`, `sleep`, `boost`), but the code validates and uses 6 presets including `none`.

This means:
- HA can't set preset to `none`
- User can't clear preset through HA interface
- After setting any preset, there's no way to go back to "no preset" via HA

---

## 5. Temperature Control Analysis

### ESP32 PID Controller Behavior

The ESP32 uses an adaptive PID controller with:
- **Deadband**: ±0.2°C (configurable) - no action when error is within deadband
- **Anti-windup**: Integral term is clamped to output limits
- **Derivative on measurement**: Prevents "derivative kick" on setpoint changes
- **Auto-tuning**: Simplified Ziegler-Nichols method

### Control Flow
```
1. Read temperature from BME280 (every 30 seconds in main loop)
2. Update PID controller (every PID_UPDATE_INTERVAL, default 10 seconds)
3. PID calculates valve position (0-100%)
4. Valve position sent to KNX
5. MQTT publishes valve position and action
```

### HA Climate Entity Expectations

HA Climate entities expect:
1. When mode is `off`, no heating should occur
2. Temperature setpoint changes should take immediate effect
3. Action should reflect actual system state

### Issues

**🔴 CRITICAL ISSUE #4**: PID runs continuously regardless of mode.

The PID controller is updated in `main.cpp` every `PID_UPDATE_INTERVAL` milliseconds:
```cpp
if (currentTime - lastPIDUpdate > configManager->getPidUpdateInterval()) {
    updatePIDControl();  // ALWAYS called, no mode check
    lastPIDUpdate = currentTime;
}
```

Even when HA sets mode to "off", the PID continues calculating and potentially sending valve commands.

---

## 6. State Synchronization

### Issue with Mode State

**⚠️ ISSUE #5**: Mode state is only published when HA sends a command.

Looking at `mqtt_manager.cpp`:
```cpp
// Handle mode command (heat/off)
if (strcmp(topic, "esp32_thermostat/mode/set") == 0) {
    // ...
    configManager->setThermostatEnabled(enabled);

    // Publish mode state confirmation
    if (_homeAssistant) {
        _homeAssistant->updateMode(mode.c_str());
    }
}
```

The mode state is only published as a **response** to an HA command. If the ESP32 reboots or the mode changes internally (e.g., from web interface), HA won't be notified unless sensor data is published.

**Initial state publishing** does occur in `home_assistant.cpp:registerEntities()`, but if the mode changes via the ESP32 web interface, HA is not informed.

---

## 7. Discovery Configuration Issues

### Abbreviated Key Usage

The ESP32 uses abbreviated MQTT discovery keys:
```cpp
climatePayload += "\"mode_cmd_t\":\"" + ...  // Abbreviated
climatePayload += "\"mode_stat_t\":\"" + ... // Abbreviated
```

**✅ CORRECT**: Abbreviated keys are valid and reduce message size.

### Missing Configuration

**⚠️ ISSUE #6**: No `initial` temperature setting in discovery.

HA allows setting an `initial` temperature for the climate entity. Without this, HA uses a default which may not match the ESP32's actual setpoint on startup.

---

## 8. Summary of Issues

| # | Severity | Issue | Impact | Status |
|---|----------|-------|--------|--------|
| 1 | 🔴 Critical | PID runs regardless of mode="off" | Heating may continue when user thinks it's off | ✅ FIXED |
| 2 | ⚠️ Medium | Action reports "idle" when mode is "off" | Incorrect state display in HA | ✅ FIXED |
| 3 | ⚠️ Medium | "none" preset not available in HA | User can't clear preset via HA | ✅ FIXED |
| 4 | 🔴 Critical | Same as #1 - no mode check in control loop | System ignores off command | ✅ FIXED |
| 5 | ⚠️ Medium | Mode changes via web UI not published to HA | HA shows stale mode | ⏳ Pending |
| 6 | 📋 Low | No initial temperature in discovery | Minor startup sync issue | ⏳ Pending |

---

## 9. Recommended Fixes

### Fix for Issue #1 & #4 (Critical)

In `main.cpp:updatePIDControl()`, add a mode check:

```cpp
void updatePIDControl() {
    ConfigManager* configManager = ConfigManager::getInstance();

    // CHECK THERMOSTAT MODE FIRST
    if (!configManager->getThermostatEnabled()) {
        // Mode is OFF - ensure valve is closed and skip PID
        knxManager.setValvePosition(0);
        mqttManager.setValvePosition(0);
        return;  // Don't run PID
    }

    // ... rest of existing PID control code
}
```

### Fix for Issue #2 (Action Reporting)

In `mqtt_manager.cpp:setValvePosition()`:

```cpp
void MQTTManager::setValvePosition(int position) {
    // ...
    ConfigManager* configManager = ConfigManager::getInstance();

    const char* action;
    if (!configManager->getThermostatEnabled()) {
        action = "off";  // Mode is off
    } else if (position > 0) {
        action = "heating";
    } else {
        action = "idle";
    }
    _mqttClient.publish("esp32_thermostat/action", action, true);
}
```

### Fix for Issue #3 (Preset None)

In `home_assistant.cpp:registerEntities()`:

```cpp
// Change from:
climatePayload += "\"pr_modes\":[\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"],";

// To:
climatePayload += "\"pr_modes\":[\"none\",\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"],";
```

### Fix for Issue #5 (Mode Sync)

Add a method to publish current mode state and call it when mode changes via any interface:

```cpp
// In HomeAssistant class
void HomeAssistant::syncModeState() {
    ConfigManager* configManager = ConfigManager::getInstance();
    const char* mode = configManager->getThermostatEnabled() ? "heat" : "off";
    _mqttClient.publish("esp32_thermostat/mode/state", mode, true);
}
```

Call this method:
- After web interface mode changes
- On MQTT reconnection
- Periodically (e.g., every minute)

---

## 10. Conclusion

The ESP32-KNX-Thermostat has a **functional but incomplete** HA integration. The most critical issue is that **the "off" mode is essentially ignored** - the PID controller continues to run and potentially heat even when the user expects the system to be off.

This could lead to:
1. Unexpected heating when user thinks system is off
2. Energy waste
3. User confusion about system state

**Recommended priority**:
1. Fix critical mode check issue (#1/#4) immediately
2. Fix action reporting (#2) for accurate HA display
3. Fix preset "none" (#3) for complete functionality
4. Fix mode sync (#5) for better state consistency
