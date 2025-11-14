# MQTT Discovery Migration Guide

## Breaking Change: Climate Entity ID Changed

### What Changed

The climate entity's MQTT discovery topic has been updated to follow Home Assistant best practices:

**Old (before this update):**
- Topic: `homeassistant/climate/esp32_thermostat/thermostat/config`
- Entity ID: `climate.esp32_thermostat_thermostat`

**New (current):**
- Topic: `homeassistant/climate/esp32_thermostat/climate/config`
- Entity ID: `climate.esp32_thermostat_climate`

### Why This Change Was Made

1. **Standards Compliance**: Added required `origin` field to all MQTT discovery messages
2. **Removed Invalid Fields**: Removed `timestamp` field that was incorrectly included in discovery payloads
3. **Consistent Naming**: Using `climate` as the object_id is more consistent with Home Assistant conventions

### Migration Steps

After updating to this firmware version, follow these steps in Home Assistant:

#### 1. Remove the Old Entity

In Home Assistant:
1. Go to **Settings** → **Devices & Services** → **MQTT**
2. Find the device "ESP32 KNX Thermostat"
3. You will see TWO climate entities:
   - `climate.esp32_thermostat_thermostat` (old, unavailable)
   - `climate.esp32_thermostat_climate` (new, active)
4. Click on the old entity and select **Delete**

#### 2. Update Your Automations

Search your automations for references to the old entity ID:

**Find and replace:**
- Old: `climate.esp32_thermostat_thermostat`
- New: `climate.esp32_thermostat_climate`

#### 3. Update Your Dashboards

If you have the thermostat in any Lovelace dashboards:
1. Edit the dashboard
2. Find cards referencing `climate.esp32_thermostat_thermostat`
3. Update to `climate.esp32_thermostat_climate`

#### 4. Update Scripts and Scenes

Check any scripts or scenes that control the thermostat and update the entity ID.

### Historical Data

Note: Historical data will remain associated with the old entity ID. If you need to preserve history:

1. The old entity will show as "unavailable" but historical data remains accessible
2. You can keep both entities if you need to reference old data
3. New data will be collected under the new entity ID

### Verification

After migration, verify the new entity is working:

```yaml
# Test in Home Assistant Developer Tools → States
climate.esp32_thermostat_climate
```

You should see:
- Current temperature
- Target temperature
- Mode (heat/off)
- Preset mode (none/eco/comfort/away/sleep/boost)
- Action (idle/heating)

### Additional Changes in This Update

1. **Origin Field**: All entities now include proper origin information
   - Name: ESP32-KNX-Thermostat
   - Software Version: 1.0
   - Support URL: GitHub repository

2. **Performance Improvements**:
   - Removed blocking delays from MQTT discovery
   - Fixed double WiFi connection during startup
   - Faster boot time and responsive web interface

3. **Fully Compliant**: All MQTT discovery messages now follow Home Assistant's official specification

### Rollback (If Needed)

If you need to rollback to the old entity ID:

1. Flash the previous firmware version
2. The old entity will become active again
3. Delete the new `climate.esp32_thermostat_climate` entity

### Questions or Issues?

If you encounter problems during migration, please report them at:
https://github.com/yourusername/ESP32-KNX-Thermostat/issues
