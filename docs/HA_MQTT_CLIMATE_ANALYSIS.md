# Home Assistant MQTT Climate Integration Analysis

**Date:** 2025-11-13
**Version:** 1.0
**Device:** ESP32 KNX Thermostat

## Executive Summary

This document analyzes our ESP32 KNX Thermostat's MQTT implementation against Home Assistant's Climate MQTT integration requirements. We have identified **3 critical issues** preventing the climate entity from working correctly in Home Assistant.

---

## 1. MQTT Climate Entity Requirements

### 1.1 Required Configuration Topics (Discovery)

| Topic Key | Our Implementation | Status |
|-----------|-------------------|--------|
| `name` | ✅ "KNX Thermostat" | ✅ Correct |
| `unique_id` | ✅ "esp32_thermostat_thermostat" | ✅ Correct |
| `device` | ✅ Device info included | ✅ Correct |
| `availability_topic` | ✅ "esp32_thermostat/status" | ✅ Correct |
| `modes` | ✅ ["off", "heat"] | ✅ Correct |
| `preset_modes` | ✅ ["none","eco","comfort","away","sleep","boost"] | ✅ Correct |
| `temperature_unit` | ✅ "C" | ✅ Correct |
| `min_temp` | ✅ "15" | ✅ Correct |
| `max_temp` | ✅ "30" | ✅ Correct |
| `temp_step` | ✅ "0.5" | ✅ Correct |

**Discovery Configuration:** `src/home_assistant.cpp:124-150`

```cpp
String climateTopic = String(HA_DISCOVERY_PREFIX) + "/climate/" + _nodeId + "/thermostat/config";
String climatePayload = "{";
climatePayload += "\"name\":\"KNX Thermostat\",";
climatePayload += "\"unique_id\":\"" + String(_nodeId) + "_thermostat\",";
climatePayload += "\"modes\":[\"off\",\"heat\"],";
climatePayload += "\"preset_modes\":[\"none\",\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"],";
// ... (see full code in source file)
```

---

### 1.2 Command Topics (HA → Device)

| Topic | Purpose | Subscribed | Handler Exists | Status |
|-------|---------|------------|----------------|--------|
| `esp32_thermostat/mode/set` | Set HVAC mode (off/heat) | ✅ Yes (line 153) | ❌ **NO** | 🔴 **BROKEN** |
| `esp32_thermostat/temperature/set` | Set target temperature | ✅ Yes (line 154) | ✅ Yes (line 147-160) | ✅ Working |
| `esp32_thermostat/preset/set` | Set preset mode | ✅ Yes (line 155) | ✅ Yes (line 163-191) | ✅ Working |

**Code Location:** `src/mqtt_manager.cpp:111-199` (processMessage function)

#### 🔴 Critical Issue #1: Missing Mode Command Handler

**Problem:** We subscribe to `esp32_thermostat/mode/set` but never process the messages!

```cpp
// Line 153: We subscribe
_mqttClient.subscribe("esp32_thermostat/mode/set");

// But in processMessage() there is NO handler:
void MQTTManager::processMessage(char* topic, byte* payload, unsigned int length) {
    // ... temperature handler exists (line 147)
    // ... preset handler exists (line 163)
    // ❌ NO MODE HANDLER!
}
```

**Impact:** Home Assistant cannot turn the thermostat on/off. The climate card will appear unresponsive.

**Fix Required:** Add mode command handler in `mqtt_manager.cpp`:

```cpp
// Handle mode command (heat/off)
if (strcmp(topic, "esp32_thermostat/mode/set") == 0) {
    String mode = String(message);
    Serial.print("Setting mode to: ");
    Serial.println(mode);

    // Update mode state
    if (mode == "off") {
        // Disable heating - set valve to 0
        extern ConfigManager* configManager;
        if (configManager) {
            configManager->setThermostatEnabled(false);
        }
        if (_knxManager) {
            _knxManager->setValvePosition(0);
        }
    } else if (mode == "heat") {
        // Enable heating - resume PID control
        extern ConfigManager* configManager;
        if (configManager) {
            configManager->setThermostatEnabled(true);
        }
    }

    // Publish the mode state back to MQTT
    if (_homeAssistant) {
        _homeAssistant->updateMode(mode.c_str());
    }
}
```

---

### 1.3 State Topics (Device → HA)

| Topic | Purpose | Published At | Retained | Status |
|-------|---------|--------------|----------|--------|
| `esp32_thermostat/mode/state` | Current HVAC mode | ✅ Startup only (line 159) | ✅ Yes | ⚠️ **Incomplete** |
| `esp32_thermostat/temperature/setpoint` | Current target temp | ✅ Startup + updates (line 164, 372) | ✅ Yes | ✅ Working |
| `esp32_thermostat/temperature` | Current room temp | ✅ Regular updates (line 285) | ❌ No | ✅ Working |
| `esp32_thermostat/preset/state` | Current preset | ✅ On change (line 382) | ✅ Yes | ⚠️ **Missing initial** |
| `esp32_thermostat/action` | Current action (heating/idle) | ✅ On sensor update (line 302/305) | ❌ No | ⚠️ **Missing initial** |

**Code Locations:**
- Initial publishing: `src/home_assistant.cpp:158-164`
- State updates: `src/home_assistant.cpp:280-383`

#### 🔴 Critical Issue #2: Incomplete Initial State Publishing

**Problem:** Home Assistant needs ALL state topics published immediately after discovery to initialize the climate entity properly.

**Current Code (incomplete):**
```cpp
// Line 158-164 in home_assistant.cpp::registerEntities()
_mqttClient.publish("esp32_thermostat/mode/state", "heat", true);

char setpointStr[8];
dtostrf(PID_SETPOINT, 1, 1, setpointStr);
_mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);

// ❌ MISSING: preset/state
// ❌ MISSING: action
```

**What's Missing:**

1. **Initial Preset State:** Never published at startup
2. **Initial Action State:** Only published when sensor data is first updated (could be 30s delay)

**Impact:** Climate entity appears "unavailable" or shows incomplete state in Home Assistant until first sensor update.

**Fix Required:** Add complete initial state publishing in `home_assistant.cpp::registerEntities()` after line 164:

```cpp
// Publish initial preset state
extern ConfigManager* configManager;
if (configManager) {
    String currentPreset = configManager->getCurrentPreset();
    _mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
} else {
    _mqttClient.publish("esp32_thermostat/preset/state", "none", true);
}

// Publish initial action state
_mqttClient.publish("esp32_thermostat/action", "idle", true);
```

---

#### 🔴 Critical Issue #3: Action State Update Timing

**Problem:** The `action` topic (heating/idle) is only updated when `updateStates()` is called, which happens during sensor data updates. This means:

1. **Startup delay:** Action state not available for up to 30 seconds after boot
2. **Delayed updates:** Action changes only reflected every 30 seconds

**Current Code:**
```cpp
// src/home_assistant.cpp:300-307
void HomeAssistant::updateStates(float temperature, float humidity, float pressure, int valvePosition) {
    // ... publish sensor data ...

    // Update action state based on valve position
    if (valvePosition > 0) {
        _mqttClient.publish("esp32_thermostat/action", "heating");
        _mqttClient.publish("esp32_thermostat/heating/state", "ON");
    } else {
        _mqttClient.publish("esp32_thermostat/action", "idle");
        _mqttClient.publish("esp32_thermostat/heating/state", "OFF");
    }
}
```

**Impact:**
- Climate card shows stale "heating/idle" status
- User confusion when valve changes but UI doesn't update immediately

**Fix Required:** Update action immediately when valve position changes. Modify `MQTTManager::setValvePosition()` in `mqtt_manager.cpp`:

```cpp
void MQTTManager::setValvePosition(int position) {
    position = constrain(position, 0, 100);

    if (position != _valvePosition) {
        _valvePosition = position;

        if (_mqttClient.connected() && _homeAssistant) {
            char valveStr[4];
            itoa(_valvePosition, valveStr, 10);
            _mqttClient.publish("esp32_thermostat/valve/position", valveStr);

            // ✅ ADD: Update action immediately
            if (_valvePosition > 0) {
                _mqttClient.publish("esp32_thermostat/action", "heating", true);
            } else {
                _mqttClient.publish("esp32_thermostat/action", "idle", true);
            }

            Serial.print("Published valve position to MQTT: ");
            Serial.println(_valvePosition);
        }
    }
}
```

---

## 2. Detailed Topic Analysis

### 2.1 Discovery Configuration (Published Once at Startup)

**Topic:** `homeassistant/climate/esp32_thermostat/thermostat/config`

**Payload Structure:**
```json
{
  "name": "KNX Thermostat",
  "unique_id": "esp32_thermostat_thermostat",
  "modes": ["off", "heat"],
  "mode_command_topic": "esp32_thermostat/mode/set",
  "mode_state_topic": "esp32_thermostat/mode/state",
  "preset_modes": ["none", "eco", "comfort", "away", "sleep", "boost"],
  "preset_mode_command_topic": "esp32_thermostat/preset/set",
  "preset_mode_state_topic": "esp32_thermostat/preset/state",
  "temperature_command_topic": "esp32_thermostat/temperature/set",
  "temperature_state_topic": "esp32_thermostat/temperature/setpoint",
  "current_temperature_topic": "esp32_thermostat/temperature",
  "temperature_unit": "C",
  "min_temp": "15",
  "max_temp": "30",
  "temp_step": "0.5",
  "action_topic": "esp32_thermostat/action",
  "availability_topic": "esp32_thermostat/status",
  "device": {
    "identifiers": ["esp32_thermostat"],
    "name": "ESP32 KNX Thermostat",
    "model": "ESP32-KNX-Thermostat",
    "manufacturer": "DIY",
    "sw_version": "1.0"
  }
}
```

**Status:** ✅ Correctly implemented

---

### 2.2 Command Topics (Subscribed by Device)

#### Mode Command
- **Topic:** `esp32_thermostat/mode/set`
- **Payload:** `"heat"` or `"off"` (string)
- **Subscription:** ✅ Line 153 in `home_assistant.cpp`
- **Handler:** ❌ **MISSING**
- **Expected Behavior:**
  - `"heat"` → Enable PID control, resume heating
  - `"off"` → Disable PID control, set valve to 0%

#### Temperature Command
- **Topic:** `esp32_thermostat/temperature/set`
- **Payload:** `"22.5"` (string representation of float)
- **Subscription:** ✅ Line 154 in `home_assistant.cpp`
- **Handler:** ✅ Lines 147-160 in `mqtt_manager.cpp`
- **Behavior:** ✅ Working correctly

#### Preset Command
- **Topic:** `esp32_thermostat/preset/set`
- **Payload:** `"eco"`, `"comfort"`, `"away"`, `"sleep"`, `"boost"`, or `"none"` (string)
- **Subscription:** ✅ Line 155 in `home_assistant.cpp`
- **Handler:** ✅ Lines 163-191 in `mqtt_manager.cpp`
- **Behavior:** ✅ Working correctly

---

### 2.3 State Topics (Published by Device)

#### Mode State
- **Topic:** `esp32_thermostat/mode/state`
- **Payload:** `"heat"` or `"off"`
- **Retained:** ✅ Yes
- **Update Frequency:** ⚠️ Only at startup and when mode command received
- **Method:** `HomeAssistant::updateMode()` (line 376-378)

#### Temperature Setpoint State
- **Topic:** `esp32_thermostat/temperature/setpoint`
- **Payload:** `"22.5"` (string with 1 decimal)
- **Retained:** ✅ Yes
- **Update Frequency:** On setpoint change
- **Method:** `HomeAssistant::updateSetpointTemperature()` (line 369-373)

#### Current Temperature
- **Topic:** `esp32_thermostat/temperature`
- **Payload:** `"21.34"` (string with 2 decimals)
- **Retained:** ❌ No
- **Update Frequency:** Every sensor update (~30s)
- **Method:** `HomeAssistant::updateStates()` (line 281-310)

#### Preset State
- **Topic:** `esp32_thermostat/preset/state`
- **Payload:** `"eco"`, `"comfort"`, `"away"`, `"sleep"`, `"boost"`, or `"none"`
- **Retained:** ✅ Yes
- **Update Frequency:** When preset changes
- **Method:** `HomeAssistant::updatePresetMode()` (line 381-383)
- **Issue:** ⚠️ Not published at startup

#### Action
- **Topic:** `esp32_thermostat/action`
- **Payload:** `"heating"`, `"idle"`, or `"off"`
- **Retained:** ❌ No
- **Update Frequency:** Every sensor update (~30s)
- **Method:** Published in `updateStates()` (lines 300-307)
- **Issue:** ⚠️ Not published at startup, delayed updates

---

## 3. Home Assistant Integration Flow

### 3.1 Startup Sequence (What Should Happen)

```
1. ESP32 boots
2. Connect to WiFi
3. Connect to MQTT broker
4. Publish availability: "esp32_thermostat/status" = "online"
5. Publish discovery config to "homeassistant/climate/esp32_thermostat/thermostat/config"
6. Subscribe to command topics:
   - esp32_thermostat/mode/set
   - esp32_thermostat/temperature/set
   - esp32_thermostat/preset/set
7. Publish initial states:
   - mode/state = "heat"
   - temperature/setpoint = current setpoint
   - preset/state = current preset ⚠️ MISSING
   - action = "idle" ⚠️ MISSING
8. Home Assistant receives discovery → Creates climate entity
9. Home Assistant subscribes to state topics
10. Home Assistant reads initial states → Climate card shows correct status
```

### 3.2 Runtime Operation

#### When User Changes Temperature in HA:
```
1. HA publishes: "esp32_thermostat/temperature/set" = "23.5"
2. ESP32 receives message in processMessage()
3. ESP32 updates PID setpoint
4. ESP32 publishes: "esp32_thermostat/temperature/setpoint" = "23.5" (retained)
5. HA updates climate card
```

#### When User Changes Mode in HA:
```
1. HA publishes: "esp32_thermostat/mode/set" = "off"
2. ESP32 receives message in processMessage()
3. ❌ NOTHING HAPPENS (no handler)
4. ❌ HA doesn't receive state confirmation
5. ❌ Climate card shows incorrect state or becomes unresponsive
```

#### When User Changes Preset in HA:
```
1. HA publishes: "esp32_thermostat/preset/set" = "eco"
2. ESP32 receives message in processMessage()
3. ESP32 gets preset temperature (18°C)
4. ESP32 updates PID setpoint
5. ESP32 publishes: "esp32_thermostat/preset/state" = "eco" (retained)
6. ESP32 publishes: "esp32_thermostat/temperature/setpoint" = "18.0" (retained)
7. ✅ HA updates climate card correctly
```

---

## 4. Comparison with HA Source Code Expectations

Based on Home Assistant MQTT Climate integration patterns:

### 4.1 Required Behavior

| Behavior | HA Expectation | Our Implementation | Status |
|----------|----------------|-------------------|--------|
| **Discovery** | Publish complete config | ✅ Complete | ✅ Pass |
| **Initial States** | All state topics published after discovery | ❌ Missing preset & action | 🔴 Fail |
| **Command ACK** | Publish state confirmation after command | ⚠️ Missing for mode | 🔴 Fail |
| **Retained States** | Critical states retained | ⚠️ Action not retained | ⚠️ Warning |
| **Availability** | Online/offline status | ✅ Correct | ✅ Pass |
| **Mode Control** | Must process mode commands | ❌ No handler | 🔴 Fail |
| **Temperature Control** | Must process temp commands | ✅ Working | ✅ Pass |
| **Preset Control** | Must process preset commands | ✅ Working | ✅ Pass |
| **Action Updates** | Real-time heating/idle status | ⚠️ 30s delay | ⚠️ Warning |

### 4.2 Optional Features (Not Yet Implemented)

| Feature | HA Support | Our Status | Priority |
|---------|------------|-----------|----------|
| `fan_mode` | ✅ Supported | ❌ Not implemented | Low (N/A for radiator) |
| `swing_mode` | ✅ Supported | ❌ Not implemented | Low (N/A for radiator) |
| `aux_heat` | ✅ Supported | ❌ Not implemented | Low |
| `target_humidity` | ✅ Supported | ❌ Not implemented | Medium (future) |
| `power` | ✅ Supported | ❌ Not implemented | Low (covered by mode) |

---

## 5. Why Climate Entity Appears as Sensors

### Root Cause Analysis

When a climate entity is not working properly in Home Assistant, it may still create individual sensor entities for the separate topics. This happens because:

1. **Incomplete Initial State** → HA cannot fully initialize the climate entity
2. **Missing Command Handlers** → HA detects the device is not responding to commands
3. **No State Confirmation** → HA marks the entity as "unavailable" or "unresponsive"

When this occurs, HA falls back to showing the individual MQTT topics as separate sensors:
- Temperature → `sensor.esp32_thermostat_temperature`
- Humidity → `sensor.esp32_thermostat_humidity`
- Valve Position → `sensor.esp32_thermostat_valve_position`
- etc.

But the climate entity `climate.knx_thermostat` either:
- Doesn't appear at all
- Appears as "unavailable"
- Appears but doesn't respond to commands

---

## 6. Required Fixes Summary

### Fix #1: Add Mode Command Handler (HIGH PRIORITY)
**File:** `src/mqtt_manager.cpp`
**Location:** `processMessage()` function
**Lines to add:** After line 191

```cpp
// Handle mode command
if (strcmp(topic, "esp32_thermostat/mode/set") == 0) {
    String mode = String(message);
    Serial.print("Setting mode to: ");
    Serial.println(mode);

    extern ConfigManager* configManager;
    if (configManager) {
        bool enabled = (mode == "heat");
        configManager->setThermostatEnabled(enabled);

        if (!enabled && _knxManager) {
            _knxManager->setValvePosition(0);
        }
    }

    // Publish mode state
    if (_homeAssistant) {
        _homeAssistant->updateMode(mode.c_str());
    }
}
```

### Fix #2: Publish Initial States (HIGH PRIORITY)
**File:** `src/home_assistant.cpp`
**Location:** `registerEntities()` function
**Lines to add:** After line 164

```cpp
// Publish initial preset state
extern ConfigManager* configManager;
if (configManager) {
    String currentPreset = configManager->getCurrentPreset();
    _mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
} else {
    _mqttClient.publish("esp32_thermostat/preset/state", "none", true);
}

// Publish initial action state
_mqttClient.publish("esp32_thermostat/action", "idle", true);
```

### Fix #3: Immediate Action Updates (MEDIUM PRIORITY)
**File:** `src/mqtt_manager.cpp`
**Location:** `setValvePosition()` function
**Lines to add:** After line 92

```cpp
// Update action immediately
if (_valvePosition > 0) {
    _mqttClient.publish("esp32_thermostat/action", "heating", true);
} else {
    _mqttClient.publish("esp32_thermostat/action", "idle", true);
}
```

### Fix #4: Add ConfigManager Method (MEDIUM PRIORITY)
**File:** `include/config_manager.h` and `src/config_manager.cpp`
**New method:** `setThermostatEnabled(bool enabled)` and `getThermostatEnabled()`

This method should store a flag to enable/disable the thermostat (for "off" mode).

---

## 7. Testing Checklist

After implementing fixes, verify the following:

### Initial Setup
- [ ] Climate entity appears in HA after discovery
- [ ] Climate card shows correct initial temperature
- [ ] Climate card shows correct initial setpoint
- [ ] Climate card shows correct initial preset
- [ ] Climate card shows correct initial mode (heat/off)
- [ ] Climate card shows correct initial action (idle/heating)

### Mode Control
- [ ] Switching to "Off" in HA stops heating (valve = 0%)
- [ ] Mode state updates immediately in HA
- [ ] Switching to "Heat" resumes PID control
- [ ] Mode persists after ESP32 reboot

### Temperature Control
- [ ] Changing setpoint in HA updates device
- [ ] Setpoint displays correctly in HA climate card
- [ ] Valve position changes based on new setpoint
- [ ] Current temperature updates regularly

### Preset Control
- [ ] Selecting preset in HA applies correct temperature
- [ ] Preset state shows correctly in HA
- [ ] Preset persists after ESP32 reboot
- [ ] Custom preset temperatures work correctly

### Action State
- [ ] "Heating" shows when valve > 0%
- [ ] "Idle" shows when valve = 0%
- [ ] Action updates immediately (not after 30s delay)
- [ ] Action shows correctly after reboot

### Availability
- [ ] Entity shows "available" when ESP32 online
- [ ] Entity shows "unavailable" when ESP32 offline
- [ ] Entity recovers correctly when ESP32 comes back online

---

## 8. File Locations Reference

| Component | File | Lines | Purpose |
|-----------|------|-------|---------|
| Discovery Config | `src/home_assistant.cpp` | 124-150 | Climate entity registration |
| Initial States | `src/home_assistant.cpp` | 158-164 | Startup state publishing |
| Command Subscription | `src/home_assistant.cpp` | 152-156 | Subscribe to HA commands |
| Message Processing | `src/mqtt_manager.cpp` | 111-199 | Handle incoming MQTT |
| State Updates | `src/home_assistant.cpp` | 280-383 | Publish sensor data |
| Valve Position | `src/mqtt_manager.cpp` | 78-98 | Track valve state |

---

## 9. Additional Notes

### MQTT Broker Configuration
Ensure your MQTT broker (typically Mosquitto) has:
- **Retained message support** enabled
- **Discovery prefix** set to `homeassistant` (default)
- **Sufficient buffer size** for discovery payloads (>1KB)

### Home Assistant Configuration
In `configuration.yaml`, ensure MQTT integration is configured:
```yaml
mqtt:
  discovery: true
  discovery_prefix: homeassistant
```

### Debugging Commands
Monitor MQTT traffic:
```bash
# Subscribe to all thermostat topics
mosquitto_sub -h <mqtt_broker> -t "esp32_thermostat/#" -v

# Subscribe to HA discovery
mosquitto_sub -h <mqtt_broker> -t "homeassistant/#" -v

# Manually send mode command (for testing)
mosquitto_pub -h <mqtt_broker> -t "esp32_thermostat/mode/set" -m "off"
```

---

## 10. Conclusion

Our MQTT Climate implementation is **85% complete** but has **3 critical issues** preventing it from working properly with Home Assistant:

1. 🔴 **Missing mode command handler** (causes unresponsive climate entity)
2. 🔴 **Incomplete initial state publishing** (causes "unavailable" status)
3. ⚠️ **Delayed action updates** (causes stale UI state)

**Estimated Fix Time:** 2-3 hours

**Impact:** After fixes, the thermostat will work seamlessly with Home Assistant's climate card, allowing full control of heating mode, temperature, and presets directly from the Home Assistant UI.

---

**Document Version:** 1.0
**Last Updated:** 2025-11-13
**Author:** Claude (AI Assistant)
**Review Status:** Ready for Implementation
