# Home Assistant MQTT Discovery Guide

This document explains how MQTT discovery works with Home Assistant and how it's implemented in the ESP32-KNX-Thermostat project.

## Table of Contents
- [Overview](#overview)
- [How MQTT Discovery Works](#how-mqtt-discovery-works)
- [Key Concepts](#key-concepts)
- [Implementation Details](#implementation-details)
- [Troubleshooting](#troubleshooting)
- [Testing and Validation](#testing-and-validation)

## Overview

MQTT Discovery allows devices to automatically register their entities (sensors, climate controls, etc.) with Home Assistant without manual YAML configuration. When a device publishes a discovery message, Home Assistant automatically creates the corresponding entities.

**Benefits:**
- Zero manual configuration in Home Assistant
- Automatic device and entity creation
- Self-documenting through discovery messages
- Easy updates - just republish discovery messages

## How MQTT Discovery Works

### Discovery Message Flow

```
1. Device publishes discovery message
   └─> homeassistant/<component>/<node_id>/<entity>/config

2. Home Assistant receives and validates message
   └─> Creates entity in MQTT integration

3. Device publishes state messages
   └─> <node_id>/<entity_type>/state

4. Home Assistant subscribes to state topics
   └─> Updates entity values in real-time
```

### Discovery Topic Structure

```
homeassistant/<component>/<node_id>/<object_id>/config
```

**Components:**
- `climate` - Thermostats and HVAC controls
- `sensor` - Temperature, humidity, pressure sensors
- `binary_sensor` - On/off sensors
- `switch` - Controllable switches
- `light` - Lighting controls

**Example:**
```
homeassistant/climate/esp32_thermostat/config
homeassistant/sensor/esp32_thermostat/temperature/config
```

## Key Concepts

### 1. Abbreviated vs Full Field Names

Home Assistant supports both abbreviated and full field names to reduce payload size.

**Common Abbreviations:**
| Full Key | Abbreviated | Description |
|----------|-------------|-------------|
| `unique_id` | `uniq_id` | Unique entity identifier |
| `device` | `dev` | Device information |
| `availability_topic` | `avty_t` | Availability topic |
| `mode_command_topic` | `mode_cmd_t` | Mode command topic |
| `mode_state_topic` | `mode_stat_t` | Mode state topic |
| `temperature_command_topic` | `temp_cmd_t` | Temperature command topic |
| `temperature_state_topic` | `temp_stat_t` | Temperature state topic |
| `current_temperature_topic` | `curr_temp_t` | Current temp topic |
| `temperature_unit` | `temp_unit` | Temperature unit |
| `preset_mode_command_topic` | `pr_mode_cmd_t` | Preset command topic |
| `preset_mode_state_topic` | `pr_mode_stat_t` | Preset state topic |
| `preset_modes` | `pr_modes` | Available preset modes |
| `action_topic` | `act_t` | Current action topic |
| `payload_available` | `pl_avail` | Available payload |
| `payload_not_available` | `pl_not_avail` | Not available payload |

### 2. Device Information

Device info groups multiple entities under a single device in Home Assistant.

**Abbreviated Device Keys:**
| Full Key | Abbreviated |
|----------|-------------|
| `identifiers` | `ids` |
| `manufacturer` | `mf` |
| `model` | `mdl` |
| `sw_version` | `sw` |

**Example:**
```json
{
  "dev": {
    "ids": ["esp32_thermostat"],
    "name": "ESP32 KNX Thermostat",
    "mf": "DIY",
    "mdl": "ESP32-KNX-Thermostat",
    "sw": "1.5"
  }
}
```

### 3. Preset Modes - Critical Discovery Rules

**IMPORTANT:** When using preset modes, you MUST use consistent key styles:

✅ **ALL Abbreviated Keys** (RECOMMENDED):
```json
{
  "pr_mode_cmd_t": "esp32_thermostat/preset/set",
  "pr_mode_stat_t": "esp32_thermostat/preset/state",
  "pr_modes": ["eco", "comfort", "away", "sleep", "boost"]
}
```

✅ **ALL Full Keys**:
```json
{
  "preset_mode_command_topic": "esp32_thermostat/preset/set",
  "preset_mode_state_topic": "esp32_thermostat/preset/state",
  "preset_modes": ["eco", "comfort", "away", "sleep", "boost"]
}
```

❌ **Mixed Keys** (WILL FAIL):
```json
{
  "mode_cmd_t": "esp32_thermostat/mode/set",  // Abbreviated
  "preset_mode_command_topic": "esp32_thermostat/preset/set",  // Full - ERROR!
  "pr_modes": ["eco", "comfort"]  // Abbreviated
}
```

**Validation Error:**
```
Error 'some but not all values in the same group of inclusion 'preset_modes'
```

Home Assistant validates that ALL fields in the preset group use the same key style.

### 4. Retained Messages

Discovery messages MUST be retained (`retain: true` or `"ret": true`) so Home Assistant receives them even after restart.

**State messages should also be retained** for immediate entity availability after Home Assistant restart.

## Implementation Details

### Climate Entity Discovery (ESP32-KNX-Thermostat)

**Current Implementation (v1.5):**

```cpp
String climatePayload = "{";
climatePayload += "\"name\":\"Thermostat\",";
climatePayload += "\"uniq_id\":\"" + String(_nodeId) + "_climate\",";
climatePayload += "\"dev\":" + deviceInfo + ",";

// Mode control (abbreviated)
climatePayload += "\"mode_cmd_t\":\"" + String(_nodeId) + "/mode/set\",";
climatePayload += "\"mode_stat_t\":\"" + String(_nodeId) + "/mode/state\",";
climatePayload += "\"modes\":[\"off\",\"heat\"],";

// Temperature control (abbreviated)
climatePayload += "\"temp_cmd_t\":\"" + String(_nodeId) + "/temperature/set\",";
climatePayload += "\"temp_stat_t\":\"" + String(_nodeId) + "/temperature/setpoint\",";
climatePayload += "\"curr_temp_t\":\"" + String(_nodeId) + "/temperature\",";
climatePayload += "\"min_temp\":15,";
climatePayload += "\"max_temp\":30,";
climatePayload += "\"temp_step\":0.5,";
climatePayload += "\"temp_unit\":\"C\",";

// Preset modes (abbreviated) - verified working with HA
climatePayload += "\"pr_mode_cmd_t\":\"" + String(_nodeId) + "/preset/set\",";
climatePayload += "\"pr_mode_stat_t\":\"" + String(_nodeId) + "/preset/state\",";
climatePayload += "\"pr_modes\":[\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"],";

// Availability (abbreviated)
climatePayload += "\"avty_t\":\"" + _availabilityTopic + "\",";
climatePayload += "\"pl_avail\":\"online\",";
climatePayload += "\"pl_not_avail\":\"offline\",";

// Action (abbreviated)
climatePayload += "\"act_t\":\"" + String(_nodeId) + "/action\",";
climatePayload += "\"qos\":0,";
climatePayload += "\"ret\":true";
climatePayload += "}";

// Publish to discovery topic
_mqttClient.publish(climateTopic.c_str(), climatePayload.c_str(), true);
```

**Payload Size:** ~1024 bytes (well within 1536 byte MQTT buffer)

### State Topics

After discovery, the device publishes state to these topics:

```
esp32_thermostat/temperature          → Current temperature
esp32_thermostat/temperature/setpoint → Target temperature
esp32_thermostat/mode/state           → Current mode (off/heat)
esp32_thermostat/preset/state         → Current preset (eco/comfort/away/sleep/boost)
esp32_thermostat/action               → Current action (idle/heating)
esp32_thermostat/status               → Availability (online/offline)
```

Home Assistant subscribes to these topics and updates the climate entity in real-time.

### MQTT Buffer Size

**IMPORTANT:** ESP32 MQTT client has limited buffer size.

```cpp
// In src/mqtt_manager.cpp
_mqttClient.setBufferSize(1536);  // Increased from 1024 for discovery messages
```

**Payload Size Guidelines:**
- Keep discovery payloads under 1500 bytes
- Use abbreviated keys to reduce size
- Test payload size before deploying:
  ```cpp
  Serial.print("Discovery payload size: ");
  Serial.println(climatePayload.length());
  ```

## Troubleshooting

### Discovery Not Working

**1. Check MQTT Broker Connection**
```bash
mosquitto_sub -h 192.168.178.32 -t "homeassistant/#" -v
```

**2. Enable Home Assistant MQTT Debug Logging**

Add to `configuration.yaml`:
```yaml
logger:
  logs:
    homeassistant.components.mqtt: debug
```

Restart Home Assistant and check logs for validation errors.

**3. Verify Discovery Topic**

Discovery messages should appear on:
```
homeassistant/climate/<node_id>/config
```

**4. Check Payload Format**

Use MQTT Explorer or mosquitto_sub to view published messages:
```bash
mosquitto_sub -h 192.168.178.32 -t "homeassistant/climate/+/config" -v
```

**5. Common Validation Errors**

**Error:** `'some but not all values in the same group of inclusion 'preset_modes'`
- **Cause:** Mixing abbreviated and full keys for preset fields
- **Fix:** Use ALL abbreviated or ALL full keys for preset group

**Error:** `'preset_modes must not include preset mode 'none''`
- **Cause:** Including 'none' in preset_modes list
- **Fix:** Remove 'none' from preset_modes array

**Error:** `Invalid discovery payload`
- **Cause:** Malformed JSON
- **Fix:** Validate JSON syntax, check for missing quotes/commas

### Entity Not Appearing in HA

1. **Check device is online** - Look at availability topic
2. **Verify unique_id** - Must be unique across all entities
3. **Check MQTT integration** - Settings → Devices & Services → MQTT
4. **Restart Home Assistant** - Sometimes needed for new entities
5. **Clear retained messages** - Old discovery messages may conflict:
   ```bash
   mosquitto_pub -h 192.168.178.32 -t "homeassistant/climate/old_id/config" -n -r
   ```

### Preset Modes Not Showing

1. **Verify preset keys are consistent** - All abbreviated or all full
2. **Check preset state topic** - Must publish initial state
3. **Validate preset names** - Use lowercase, valid preset names
4. **Check HA logs** - Look for validation errors

## Testing and Validation

### Python Test Script

The project includes a comprehensive test script for MQTT discovery:

```bash
python3 test/test_mqtt_discovery_current.py
```

This script tests multiple variations:
1. **test1**: No presets (baseline)
2. **test2**: Abbreviated preset keys ✅ WORKING
3. **test3**: Full preset keys ✅ WORKING
4. **test4**: Mixed keys ❌ VALIDATION ERROR

### Manual Testing Checklist

Before deploying discovery changes:

- [ ] Validate JSON syntax
- [ ] Check payload size (must be < 1536 bytes)
- [ ] Test with MQTT broker
- [ ] Enable HA debug logging
- [ ] Verify entity creation in HA
- [ ] Test all preset modes
- [ ] Verify state updates
- [ ] Check availability handling
- [ ] Test after HA restart

### Payload Size Calculator

```python
import json

payload = {
    "name": "Thermostat",
    "uniq_id": "esp32_thermostat_climate",
    # ... full payload
}

payload_json = json.dumps(payload)
print(f"Payload size: {len(payload_json)} bytes")

if len(payload_json) > 1536:
    print("⚠️  WARNING: Payload exceeds MQTT buffer size!")
```

## Best Practices

1. **Use Abbreviated Keys** - Reduces payload size by ~10-15%
2. **Retain Discovery Messages** - Essential for HA restart recovery
3. **Publish States Before Discovery** - Ensures immediate entity availability
4. **Use Unique IDs** - Prevents entity conflicts
5. **Group Entities by Device** - Better organization in HA
6. **Test Thoroughly** - Use test scripts before production
7. **Enable Debug Logging** - During development and troubleshooting
8. **Document Your Topics** - Maintain topic structure documentation
9. **Version Your Discovery** - Track changes via sw_version field
10. **Validate Payloads** - Test JSON syntax before publishing

## References

- [Home Assistant MQTT Discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery)
- [MQTT Climate Discovery](https://www.home-assistant.io/integrations/climate.mqtt/)
- [MQTT Sensor Discovery](https://www.home-assistant.io/integrations/sensor.mqtt/)
- [Abbreviated Keys Reference](https://www.home-assistant.io/integrations/mqtt/#discovery-topic)

## Version History

- **v1.5** (2025-11-14): Added preset mode support with abbreviated keys
- **v1.4** (2025-11-14): Initial MQTT discovery implementation without presets
- **v1.3** (2025-11-13): Pre-discovery version (manual YAML only)

## Notes for Future Projects

### Quick Start Template

```cpp
// 1. Set MQTT buffer size
_mqttClient.setBufferSize(1536);

// 2. Create device info (abbreviated)
String deviceInfo = "{\"ids\":[\"my_device\"],\"name\":\"My Device\",\"mf\":\"DIY\",\"mdl\":\"ESP32-Device\",\"sw\":\"1.0\"}";

// 3. Build discovery payload (abbreviated keys)
String payload = "{";
payload += "\"name\":\"My Sensor\",";
payload += "\"uniq_id\":\"my_device_sensor\",";
payload += "\"dev\":" + deviceInfo + ",";
payload += "\"stat_t\":\"my_device/sensor/state\",";
payload += "\"avty_t\":\"my_device/status\",";
payload += "\"pl_avail\":\"online\",";
payload += "\"pl_not_avail\":\"offline\",";
payload += "\"ret\":true";
payload += "}";

// 4. Publish discovery (retained)
String topic = "homeassistant/sensor/my_device/sensor/config";
mqttClient.publish(topic.c_str(), payload.c_str(), true);

// 5. Publish state
mqttClient.publish("my_device/sensor/state", "23.5", true);
mqttClient.publish("my_device/status", "online", true);
```

### Key Takeaways

1. **Consistency is critical** - Don't mix abbreviated and full keys within groups
2. **Test before deploy** - Use Python scripts to validate discovery messages
3. **Buffer size matters** - Increase MQTT buffer for complex entities
4. **Retained messages** - Essential for HA restart reliability
5. **Debug logging** - Always enable during development

---

**Document maintained by:** Claude Code
**Last updated:** 2025-11-14
**Project:** ESP32-KNX-Thermostat
