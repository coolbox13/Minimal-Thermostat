# ESP32 KNX Thermostat API Documentation

Version: 1.5 | Last Updated: 2025-11-14

This document provides comprehensive API documentation for the ESP32-KNX-Thermostat project, including MQTT topics, REST API endpoints, and integration examples.

## Table of Contents
- [MQTT API](#mqtt-api)
- [REST API Endpoints](#rest-api-endpoints)
- [Home Assistant Integration](#home-assistant-integration)
- [Configuration Management](#configuration-management)
- [Integration Examples](#integration-examples)

---

## MQTT API

The thermostat communicates via MQTT using the base topic prefix `esp32_thermostat/`.

### Climate Control Topics

#### Mode Control
**Subscribe Topics (Commands from HA):**
```
esp32_thermostat/mode/set
```
- **Payload:** `"off"` or `"heat"`
- **Description:** Set the operating mode

**Publish Topics (State to HA):**
```
esp32_thermostat/mode/state
```
- **Payload:** `"off"` or `"heat"`
- **Retained:** Yes
- **Description:** Current operating mode

#### Temperature Control
**Subscribe Topics:**
```
esp32_thermostat/temperature/set
```
- **Payload:** Float value (15.0 - 30.0)
- **Description:** Set temperature setpoint

**Publish Topics:**
```
esp32_thermostat/temperature
```
- **Payload:** Float value (e.g., `"20.5"`)
- **Description:** Current measured temperature

```
esp32_thermostat/temperature/setpoint
```
- **Payload:** Float value (e.g., `"22.0"`)
- **Retained:** Yes
- **Description:** Target temperature setpoint

#### Preset Modes (v1.5+)
**Subscribe Topics:**
```
esp32_thermostat/preset/set
```
- **Payload:** `"eco"`, `"comfort"`, `"away"`, `"sleep"`, or `"boost"`
- **Description:** Set preset mode

**Publish Topics:**
```
esp32_thermostat/preset/state
```
- **Payload:** Current preset name
- **Retained:** Yes
- **Description:** Active preset mode

**Preset Mode Behaviors:**
| Preset | Temperature Offset | Description |
|--------|-------------------|-------------|
| `eco` | -2°C | Energy saving mode |
| `comfort` | 0°C | Normal operation |
| `away` | -4°C | Extended absence |
| `sleep` | -1°C | Nighttime comfort |
| `boost` | +2°C | Quick heating |

### Sensor Data Topics

All sensor topics publish periodically (every 30 seconds) and are not retained.

#### Environmental Sensors
```
esp32_thermostat/temperature
```
- **Payload:** Float (°C, e.g., `"20.5"`)
- **Update Rate:** 30s
- **Source:** BME280 sensor

```
esp32_thermostat/humidity
```
- **Payload:** Float (%, e.g., `"45.2"`)
- **Update Rate:** 30s
- **Source:** BME280 sensor

```
esp32_thermostat/pressure
```
- **Payload:** Float (hPa, e.g., `"1013.25"`)
- **Update Rate:** 30s
- **Source:** BME280 sensor

#### Valve Control
**Subscribe Topics:**
```
esp32_thermostat/valve/set
```
- **Payload:** Integer (0-100)
- **Description:** Manual valve position override

**Publish Topics:**
```
esp32_thermostat/valve/position
```
- **Payload:** Integer (0-100, percentage)
- **Description:** Current valve position

```
esp32_thermostat/valve/status
```
- **Payload:** Status string
- **Description:** Valve operational status

#### System Status
```
esp32_thermostat/action
```
- **Payload:** `"idle"` or `"heating"`
- **Retained:** Yes
- **Description:** Current HVAC action

```
esp32_thermostat/heating/state
```
- **Payload:** `"ON"` or `"OFF"`
- **Description:** Heating element state

```
esp32_thermostat/status
```
- **Payload:** `"online"` or `"offline"`
- **Retained:** Yes (LWT)
- **Description:** Device availability (Last Will Testament)

#### PID Controller Parameters
```
esp32_thermostat/pid/kp
esp32_thermostat/pid/ki
esp32_thermostat/pid/kd
```
- **Payload:** Float values
- **Description:** Current PID tuning parameters

#### System Diagnostics
```
esp32_thermostat/wifi/rssi
```
- **Payload:** Integer (dBm, e.g., `"-67"`)
- **Description:** WiFi signal strength

```
esp32_thermostat/uptime
```
- **Payload:** Integer (seconds)
- **Description:** System uptime

```
esp32_thermostat/logs
```
- **Payload:** JSON log entries
- **Description:** System log messages

#### Manual Override
```
esp32_thermostat/manual_override/enabled
```
- **Payload:** `"ON"` or `"OFF"`
- **Description:** Manual override active state

```
esp32_thermostat/manual_override/position
```
- **Payload:** Integer (0-100)
- **Description:** Manual valve position when override enabled

#### System Control
**Subscribe Topics:**
```
esp32_thermostat/restart
```
- **Payload:** Any value
- **Description:** Trigger device restart

---

## REST API Endpoints

All API endpoints return JSON responses with appropriate HTTP status codes.

### Sensor Data

#### GET /api/sensor-data
Get current sensor readings.

**Response:**
```json
{
  "temperature": 20.5,
  "humidity": 45.2,
  "pressure": 1013.25,
  "valve_position": 42,
  "setpoint": 22.0,
  "pid_output": 42.3
}
```

#### GET /api/history
Get historical sensor data.

**Query Parameters:**
- `limit` (optional): Number of data points (default: 100)

**Response:**
```json
{
  "history": [
    {
      "timestamp": 1699987200,
      "temperature": 20.5,
      "humidity": 45.2,
      "valve_position": 42
    }
  ]
}
```

### System Status

#### GET /api/status
Get comprehensive system status.

**Response:**
```json
{
  "temperature": 20.5,
  "humidity": 45.2,
  "pressure": 1013.25,
  "setpoint": 22.0,
  "valve_position": 42,
  "pid_output": 42.3,
  "mode": "heat",
  "preset": "comfort",
  "action": "heating",
  "wifi": {
    "connected": true,
    "ssid": "MyNetwork",
    "rssi": -67,
    "ip": "192.168.1.100"
  },
  "uptime": 86400,
  "heap_free": 234567
}
```

#### GET /api/sensor-health
Get sensor health monitoring status.

**Response:**
```json
{
  "temperature_health": "healthy",
  "humidity_health": "healthy",
  "pressure_health": "healthy",
  "last_update": 1699987200,
  "consecutive_failures": 0
}
```

#### GET /api/valve-health
Get valve health monitoring status.

**Response:**
```json
{
  "valve_health": "healthy",
  "stuck_detected": false,
  "last_movement": 1699987200,
  "position_changes": 42
}
```

### Configuration

#### GET /api/config
Get current device configuration.

**Response:**
```json
{
  "wifi": {
    "ssid": "MyNetwork",
    "ip": "192.168.1.100"
  },
  "mqtt": {
    "server": "192.168.1.32",
    "port": 1883,
    "enabled": true
  },
  "knx": {
    "area": 1,
    "line": 1,
    "member": 160,
    "use_test_addresses": false
  },
  "pid": {
    "kp": 2.0,
    "ki": 0.5,
    "kd": 1.0,
    "setpoint": 22.0
  },
  "webhook_url": "https://example.com/webhook"
}
```

#### POST /api/config
Update device configuration.

**Request Body:**
```json
{
  "mqtt_server": "192.168.1.32",
  "mqtt_port": 1883,
  "knx_area": 1,
  "knx_line": 1,
  "knx_member": 160,
  "use_test_addresses": false,
  "pid_kp": 2.0,
  "pid_ki": 0.5,
  "pid_kd": 1.0,
  "webhook_url": "https://example.com/webhook"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Configuration updated",
  "restart_required": true
}
```

#### GET /api/config/export
Download configuration as JSON file.

**Response:** JSON file download with `Content-Disposition: attachment`

#### POST /api/config/import
Upload configuration from JSON file.

**Request:** Multipart form data with JSON file
**Response:**
```json
{
  "success": true,
  "message": "Configuration imported"
}
```

### Temperature Control

#### POST /api/setpoint
Set temperature setpoint.

**Request Body:**
```json
{
  "value": 22.0
}
```

**Response:**
```json
{
  "success": true,
  "setpoint": 22.0
}
```

### Manual Valve Override

#### GET /api/manual-override
Get manual override status.

**Response:**
```json
{
  "enabled": false,
  "position": 0
}
```

#### POST /api/manual-override
Enable/disable manual valve override.

**Request Body:**
```json
{
  "enabled": true,
  "position": 50
}
```

**Response:**
```json
{
  "success": true,
  "enabled": true,
  "position": 50
}
```

### Event Logs

#### GET /api/logs
Get system event logs.

**Response:**
```json
{
  "logs": [
    {
      "timestamp": 1699987200,
      "level": "INFO",
      "message": "System started"
    }
  ]
}
```

#### POST /api/logs/clear
Clear all event logs.

**Response:**
```json
{
  "success": true,
  "message": "Logs cleared"
}
```

### Webhooks

#### POST /api/webhook/test
Test webhook configuration.

**Response:**
```json
{
  "success": true,
  "message": "Webhook test sent",
  "response_code": 200
}
```

### System Control

#### POST /api/reboot
Reboot the device.

**Response:**
```json
{
  "success": true,
  "message": "Rebooting..."
}
```

#### POST /api/factory-reset
Reset device to factory defaults.

**Response:**
```json
{
  "success": true,
  "message": "Factory reset initiated"
}
```

---

## Home Assistant Integration

### MQTT Discovery

The thermostat automatically registers with Home Assistant via MQTT discovery.

**Discovery Topic Structure:**
```
homeassistant/<component>/esp32_thermostat/<object_id>/config
```

**Components Created:**
- **1 Climate Entity**: `climate.esp32_thermostat_thermostat`
- **3 Environmental Sensors**: temperature, humidity, pressure
- **1 Valve Position Sensor**: percentage (0-100%)
- **1 Heating State Binary Sensor**: ON/OFF
- **3 PID Parameter Sensors**: Kp, Ki, Kd
- **1 WiFi RSSI Sensor**: signal strength (dBm)
- **1 Uptime Sensor**: system uptime (seconds)

**Total: 11 entities**

### Climate Entity Features

**Supported Modes:**
- `off` - System disabled
- `heat` - Heating enabled

**Preset Modes (v1.5+):**
- `eco` - Energy saving (-2°C)
- `comfort` - Normal operation (0°C)
- `away` - Extended absence (-4°C)
- `sleep` - Nighttime mode (-1°C)
- `boost` - Quick heating (+2°C)

**Temperature Range:**
- Minimum: 15°C
- Maximum: 30°C
- Step: 0.5°C

**Actions:**
- `idle` - No heating required
- `heating` - Actively heating

### Discovery Payload Example

**Climate Entity Discovery** (abbreviated keys for payload size optimization):
```json
{
  "name": "Thermostat",
  "uniq_id": "esp32_thermostat_climate",
  "dev": {
    "ids": ["esp32_thermostat"],
    "name": "ESP32 KNX Thermostat",
    "mf": "DIY",
    "mdl": "ESP32-KNX-Thermostat",
    "sw": "1.5"
  },
  "mode_cmd_t": "esp32_thermostat/mode/set",
  "mode_stat_t": "esp32_thermostat/mode/state",
  "modes": ["off", "heat"],
  "temp_cmd_t": "esp32_thermostat/temperature/set",
  "temp_stat_t": "esp32_thermostat/temperature/setpoint",
  "curr_temp_t": "esp32_thermostat/temperature",
  "min_temp": 15,
  "max_temp": 30,
  "temp_step": 0.5,
  "temp_unit": "C",
  "pr_mode_cmd_t": "esp32_thermostat/preset/set",
  "pr_mode_stat_t": "esp32_thermostat/preset/state",
  "pr_modes": ["eco", "comfort", "away", "sleep", "boost"],
  "avty_t": "esp32_thermostat/status",
  "pl_avail": "online",
  "pl_not_avail": "offline",
  "act_t": "esp32_thermostat/action",
  "qos": 0,
  "ret": true
}
```

**Key Field Abbreviations:**
| Full Key | Abbreviated | Description |
|----------|-------------|-------------|
| `unique_id` | `uniq_id` | Unique entity identifier |
| `device` | `dev` | Device information |
| `mode_command_topic` | `mode_cmd_t` | Mode command topic |
| `mode_state_topic` | `mode_stat_t` | Mode state topic |
| `temperature_command_topic` | `temp_cmd_t` | Temperature command topic |
| `temperature_state_topic` | `temp_stat_t` | Temperature state topic |
| `current_temperature_topic` | `curr_temp_t` | Current temperature topic |
| `temperature_unit` | `temp_unit` | Temperature unit |
| `preset_mode_command_topic` | `pr_mode_cmd_t` | Preset command topic |
| `preset_mode_state_topic` | `pr_mode_stat_t` | Preset state topic |
| `preset_modes` | `pr_modes` | Available preset modes |
| `availability_topic` | `avty_t` | Availability topic |
| `payload_available` | `pl_avail` | Available payload |
| `payload_not_available` | `pl_not_avail` | Not available payload |
| `action_topic` | `act_t` | Current action topic |
| `retain` | `ret` | Message retention flag |

**Important:** When using preset modes, ALL fields in the preset group must use the same key style (all abbreviated OR all full keys). Mixing styles causes HA validation errors.

---

## Configuration Management

### ConfigManager Class

Central configuration storage using ESP32 Preferences API.

#### Key Methods

**Initialization:**
```cpp
ConfigManager* configManager = ConfigManager::getInstance();
configManager->begin();
```

**WiFi Settings:**
```cpp
String ssid = configManager->getWifiSSID();
String password = configManager->getWifiPassword();
configManager->setWifiCredentials(ssid, password);
```

**MQTT Settings:**
```cpp
String server = configManager->getMqttServer();
uint16_t port = configManager->getMqttPort();
configManager->setMqttServer(server);
configManager->setMqttPort(port);
```

**KNX Settings:**
```cpp
uint8_t area = configManager->getKnxArea();
uint8_t line = configManager->getKnxLine();
uint8_t member = configManager->getKnxMember();
bool useTest = configManager->getUseTestAddresses();

configManager->setKnxPhysicalAddress(area, line, member);
configManager->setUseTestAddresses(useTest);
```

**PID Parameters:**
```cpp
float kp = configManager->getPidKp();
float ki = configManager->getPidKi();
float kd = configManager->getPidKd();
float setpoint = configManager->getTemperatureSetpoint();

configManager->setPidKp(kp);
configManager->setPidKi(ki);
configManager->setPidKd(kd);
configManager->setTemperatureSetpoint(setpoint);
```

**Preset Management:**
```cpp
String preset = configManager->getCurrentPreset();
configManager->setCurrentPreset("comfort");

float offset = configManager->getPresetTemperatureOffset(preset);
```

---

## Integration Examples

### Python MQTT Client

```python
import paho.mqtt.client as mqtt
import json

MQTT_BROKER = "192.168.1.32"
MQTT_PORT = 1883

def on_message(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")

# Connect to MQTT broker
client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Subscribe to temperature updates
client.subscribe("esp32_thermostat/temperature")

# Set temperature setpoint
client.publish("esp32_thermostat/temperature/set", "22.5")

# Change preset mode
client.publish("esp32_thermostat/preset/set", "eco")

# Subscribe to all thermostat topics
client.subscribe("esp32_thermostat/#")

client.loop_forever()
```

### REST API Client (curl)

```bash
# Get sensor data
curl http://192.168.1.100/api/sensor-data

# Set temperature setpoint
curl -X POST http://192.168.1.100/api/setpoint \
  -H "Content-Type: application/json" \
  -d '{"value": 22.0}'

# Enable manual valve override
curl -X POST http://192.168.1.100/api/manual-override \
  -H "Content-Type: application/json" \
  -d '{"enabled": true, "position": 50}'

# Get system status
curl http://192.168.1.100/api/status

# Export configuration
curl http://192.168.1.100/api/config/export -o config.json

# Reboot device
curl -X POST http://192.168.1.100/api/reboot
```

### Node-RED Flow

```json
[
  {
    "id": "mqtt_in",
    "type": "mqtt in",
    "topic": "esp32_thermostat/temperature",
    "broker": "mqtt_broker"
  },
  {
    "id": "http_request",
    "type": "http request",
    "method": "GET",
    "url": "http://192.168.1.100/api/sensor-data"
  }
]
```

---

## Error Handling

### HTTP Status Codes

- **200 OK**: Request successful
- **400 Bad Request**: Invalid parameters
- **404 Not Found**: Endpoint not found
- **500 Internal Server Error**: Server error

### MQTT Error Topics

Critical errors are published to:
```
esp32_thermostat/logs
```

Payload format:
```json
{
  "timestamp": 1699987200,
  "level": "ERROR",
  "message": "Sensor read failed"
}
```

---

## Best Practices

1. **MQTT Connection**: Always check `esp32_thermostat/status` for device availability before sending commands
2. **Retained Messages**: Discovery and state topics use retained messages for HA restart reliability
3. **Rate Limiting**: Don't poll REST endpoints faster than 1 request per second
4. **Setpoint Changes**: Wait at least 10 seconds between setpoint adjustments for PID stability
5. **Configuration Updates**: Expect device restart after configuration changes
6. **Manual Override**: Disable manual override before adjusting PID parameters
7. **Webhook Testing**: Always test webhooks with `/api/webhook/test` before relying on them
8. **Buffer Limits**: MQTT payloads limited to 1536 bytes (use abbreviated keys for large messages)

---

## Version History

- **v1.5** (2025-11-14): Added preset mode support with 5 preset modes
- **v1.4** (2025-11-14): Initial MQTT discovery implementation
- **v1.3** (2025-11-13): Pre-discovery version with manual YAML configuration

---

**Document Maintained By:** Claude Code
**Project:** ESP32-KNX-Thermostat
**Last Updated:** 2025-11-14
