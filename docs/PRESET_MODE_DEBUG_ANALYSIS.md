# Home Assistant Climate Entity Preset Mode Debug Analysis

**Date:** 2025-11-14
**Issue:** HA no longer recognizes ESP32 as climate entity after preset modes were added
**Branch:** claude/fix-ha-climate-preset-issue-011aj6GB4i2uNbiYS3MUJHN7

---

## Executive Summary

The climate entity **worked perfectly before November 12, 2025**. After adding preset modes to the MQTT discovery configuration on November 13, Home Assistant stopped recognizing the ESP32 as a climate entity.

### Root Cause Hypothesis

Based on extensive research and code analysis, I've identified **3 critical potential issues**:

1. **MQTT State Topic Replay Timing Issue**
2. **Preset State Value Validation**
3. **Discovery Message Size and Timing**

---

## Timeline Analysis

### Working Configuration (Before Nov 12)
- **Commit:** 67c2558
- **Status:** Climate entity working perfectly
- **Features:** Basic climate with mode (off/heat) and temperature control
- **No preset modes** in discovery config

### Breaking Change (Nov 13)
- **Commit:** 9956207
- **Change:** Added preset_modes to MQTT discovery
- **Added lines:**
  ```cpp
  climatePayload += "\"preset_modes\":[\"none\",\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"],";
  climatePayload += "\"preset_mode_command_topic\":\"esp32_thermostat/preset/set\",";
  climatePayload += "\"preset_mode_state_topic\":\"esp32_thermostat/preset/state\",";
  ```
- **Result:** Climate entity no longer appears in HA

---

## Critical Research Finding

From Home Assistant MQTT documentation and community forums:

> **"After the configs have been published, the state topics will need an update, and state updates also need to be re-published after a config has been processed. As soon as a config is received, the setup will subscribe to any state topics, and if a retained message is available at a state topic, this message will be replayed so that the state can be restored."**

This means:
1. HA publishes discovery config
2. HA immediately subscribes to all state topics listed in the config
3. HA expects to receive retained messages on those topics
4. If state topics are missing, invalid, or timing is wrong → entity becomes unavailable

---

## Issue #1: MQTT Message Sequencing Problem

### Current Sequence (src/home_assistant.cpp:153-186)

```
1. Publish climate discovery config (line 154)
   └─> HA receives and subscribes to all state topics immediately
2. delay(100ms) (line 153)
3. Subscribe to command topics (line 159-161)
4. Publish mode/state = "heat" (line 165)
5. Publish temperature/setpoint (line 170)
6. Publish preset/state = getCurrentPreset() (line 176)
7. Publish action = "idle" (line 185)
```

### The Problem

**Race Condition:** HA subscribes to state topics faster than ESP32 publishes them!

When HA receives the discovery config, it:
1. Immediately subscribes to `preset/state` topic
2. Expects a retained message
3. **If no message exists yet** → entity initialization fails

### Expected Sequence

State topics should be published **BEFORE** discovery config:

```
1. Publish mode/state = "heat" (RETAINED)
2. Publish temperature/setpoint (RETAINED)
3. Publish preset/state = "none" (RETAINED)  ← MUST exist FIRST
4. Publish action = "idle" (RETAINED)
5. delay(100ms) for MQTT to process
6. Publish climate discovery config
7. Subscribe to command topics
```

---

## Issue #2: Preset State Value Validation

### Current Code (src/config_manager.cpp:333-335)

```cpp
String ConfigManager::getCurrentPreset() {
    return _preferences.getString("preset_cur", "none");
}
```

### Potential Problems

1. **First Boot:** If NVS is empty, returns "none" ✅ (This is correct)
2. **After Boot:** If preset was never set, returns "none" ✅ (Correct)
3. **Case Sensitivity:** MQTT topics are case-sensitive - is "none" vs "None" an issue?
4. **Timing:** `getCurrentPreset()` is called AFTER discovery is published

### Discovery Config Lists Valid Presets

```cpp
"preset_modes":["none","eco","comfort","away","sleep","boost"]
```

HA validates that preset_state messages match this list. If it receives:
- Invalid preset name → entity fails
- Empty string → entity fails
- null → entity fails

---

## Issue #3: Discovery Message Size and Buffer

### Current Setup
- **Buffer Size:** 1024 bytes (mqtt_manager.cpp:34)
- **Discovery Payload Size:** 864 bytes
- **Margin:** 160 bytes ✅ (Should be fine)

This is likely **NOT** the issue, but worth monitoring.

---

## Issue #4: Missing Initial State at Broker

### The Retained Message Problem

When Home Assistant **restarts**, it:
1. Connects to MQTT broker
2. Subscribes to discovery topic `homeassistant/climate/+/+/config`
3. Receives retained discovery configs
4. For each config, subscribes to state topics
5. **Expects retained messages on state topics**

If ESP32 boots AFTER HA has already started:
- ✅ Works fine (ESP32 publishes everything fresh)

If HA restarts while ESP32 is already running:
- ❌ **PROBLEM:** HA doesn't see the preset state because:
  - Discovery config is retained ✅
  - But state topics might not have retained messages ❌

### Current Retained Flags

From src/home_assistant.cpp:

```cpp
// Line 165 - Mode state
_mqttClient.publish("esp32_thermostat/mode/state", "heat", true); ✅ RETAINED

// Line 170 - Setpoint
_mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true); ✅ RETAINED

// Line 176 - Preset state
_mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true); ✅ RETAINED

// Line 185 - Action
_mqttClient.publish("esp32_thermostat/action", "idle", true); ✅ RETAINED
```

All have retained flag = `true` ✅

But they're published **AFTER** discovery, which is the problem!

---

## Issue #5: ConfigManager Not Initialized Yet?

### Potential Race Condition

From src/home_assistant.cpp:173-181:

```cpp
extern ConfigManager* configManager;
if (configManager) {
    String currentPreset = configManager->getCurrentPreset();
    _mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
    Serial.print("Published initial preset state: ");
    Serial.println(currentPreset);
} else {
    _mqttClient.publish("esp32_thermostat/preset/state", "none", true);
    Serial.println("Published initial preset state: none");
}
```

**Question:** Is `configManager` actually initialized when `HomeAssistant::registerEntities()` is called?

Need to check the initialization order in main.cpp.

---

## Diagnostic Plan

To identify the exact issue, we need to:

### 1. Add Comprehensive Debug Logging
- Log exact values being published to preset/state
- Log timing of discovery vs state publishing
- Log whether configManager is null

### 2. Check MQTT Broker State
- Use `mosquitto_sub` to monitor all topics
- Verify retained messages exist
- Check message arrival order

### 3. Check Home Assistant Logs
- Look for MQTT climate entity errors
- Check for validation failures
- Look for "unavailable" reasons

### 4. Test State Publishing Order
- Publish states BEFORE discovery
- Add longer delays between messages
- Verify retained flags work

---

## Proposed Fixes

### Fix #1: Reorder State Publishing (HIGH PRIORITY)

**File:** `src/home_assistant.cpp`
**Function:** `registerEntities()`

**Change:**
```cpp
void HomeAssistant::registerEntities() {
    // ... existing sensor registration code ...

    // ===== PUBLISH ALL STATES FIRST (BEFORE DISCOVERY) =====

    // 1. Publish mode state
    _mqttClient.publish("esp32_thermostat/mode/state", "heat", true);
    Serial.println("Published initial mode state: heat");

    // 2. Publish setpoint
    char setpointStr[8];
    dtostrf(PID_SETPOINT, 1, 1, setpointStr);
    _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
    Serial.print("Published initial setpoint: ");
    Serial.println(setpointStr);

    // 3. Publish preset state
    extern ConfigManager* configManager;
    String currentPreset = "none"; // Default
    if (configManager) {
        currentPreset = configManager->getCurrentPreset();
        Serial.print("ConfigManager preset: ");
        Serial.println(currentPreset);
    } else {
        Serial.println("WARNING: ConfigManager is null, using default preset 'none'");
    }
    _mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
    Serial.print("Published initial preset state: ");
    Serial.println(currentPreset);

    // 4. Publish action state
    _mqttClient.publish("esp32_thermostat/action", "idle", true);
    Serial.println("Published initial action state: idle");

    // 5. Wait for MQTT broker to process retained messages
    delay(200);
    Serial.println("State topics published, waiting for broker...");

    // ===== NOW PUBLISH DISCOVERY CONFIG =====

    // Climate entity discovery
    String climateTopic = String(HA_DISCOVERY_PREFIX) + "/climate/" + _nodeId + "/thermostat/config";
    String climatePayload = "{";
    // ... existing payload construction ...

    bool climateSuccess = _mqttClient.publish(climateTopic.c_str(), climatePayload.c_str(), true);
    Serial.print("Published climate discovery config: ");
    Serial.println(climateSuccess ? "Success" : "FAILED");

    // Subscribe to command topics
    _mqttClient.subscribe("esp32_thermostat/mode/set");
    _mqttClient.subscribe("esp32_thermostat/temperature/set");
    _mqttClient.subscribe("esp32_thermostat/preset/set");
    Serial.println("Subscribed to thermostat control topics");
}
```

### Fix #2: Add State Re-publishing After Discovery

**Rationale:** Some sources suggest re-publishing states after discovery to ensure HA picks them up.

```cpp
// After discovery is published
delay(100);

// Re-publish all states to ensure HA sees them
_mqttClient.publish("esp32_thermostat/mode/state", "heat", true);
_mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
_mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
_mqttClient.publish("esp32_thermostat/action", "idle", true);
Serial.println("Re-published all state topics after discovery");
```

### Fix #3: Validate Preset Value

Add validation to ensure preset is in the valid list:

```cpp
// Validate preset is in allowed list
const char* validPresets[] = {"none", "eco", "comfort", "away", "sleep", "boost"};
bool isValidPreset = false;
for (int i = 0; i < 6; i++) {
    if (currentPreset.equals(validPresets[i])) {
        isValidPreset = true;
        break;
    }
}

if (!isValidPreset) {
    Serial.print("WARNING: Invalid preset '");
    Serial.print(currentPreset);
    Serial.println("', resetting to 'none'");
    currentPreset = "none";
    if (configManager) {
        configManager->setCurrentPreset("none");
    }
}
```

---

## Testing Checklist

After implementing fixes:

### Test 1: Fresh ESP32 Boot
- [ ] ESP32 boots and connects to MQTT
- [ ] Check serial output for state publishing order
- [ ] Verify climate entity appears in HA
- [ ] Check HA shows correct preset = "none"

### Test 2: HA Restart (ESP32 Running)
- [ ] Restart Home Assistant
- [ ] Climate entity should reappear immediately
- [ ] Check HA reads retained preset state
- [ ] Verify entity is not "unavailable"

### Test 3: Change Preset
- [ ] Set preset to "eco" from HA
- [ ] Verify ESP32 receives command
- [ ] Check temperature changes to eco value
- [ ] Verify preset state updates in HA

### Test 4: MQTT Broker Monitoring
```bash
# Monitor all thermostat topics
mosquitto_sub -h <broker> -t "esp32_thermostat/#" -v

# Monitor discovery
mosquitto_sub -h <broker> -t "homeassistant/climate/#" -v
```

Expected output order:
1. esp32_thermostat/mode/state heat
2. esp32_thermostat/temperature/setpoint 21.0
3. esp32_thermostat/preset/state none
4. esp32_thermostat/action idle
5. homeassistant/climate/esp32_thermostat/thermostat/config {...}

### Test 5: Preset Validation
- [ ] Manually corrupt preset in NVS
- [ ] Reboot ESP32
- [ ] Verify it resets to "none"
- [ ] Climate entity still works

---

## Additional Investigation Needed

1. **Check HA Logs:** Look for specific error messages about the climate entity
2. **Verify MQTT Broker:** Confirm retained messages are stored
3. **Check ConfigManager Init:** Verify it's initialized before HA discovery
4. **Monitor Serial Output:** Look for any errors or warnings during discovery

---

## References

- Home Assistant MQTT Climate: https://www.home-assistant.io/integrations/climate.mqtt/
- MQTT Discovery Best Practices: Home Assistant Community Forums
- Related GitHub Issues:
  - #63198: Climate entities through MQTT Discovery don't receive messages
  - #42946: MQTT Climate Preset_mode Away=OFF state set wrong

---

## Next Steps

1. ✅ Create this analysis document
2. ⏳ Implement Fix #1 (Reorder state publishing)
3. ⏳ Add comprehensive debug logging
4. ⏳ Test with actual Home Assistant instance
5. ⏳ Monitor MQTT broker traffic
6. ⏳ Iterate based on findings

---

**Analysis Version:** 1.0
**Author:** Claude (AI Assistant)
**Status:** Ready for Implementation
