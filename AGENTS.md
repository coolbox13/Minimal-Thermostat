# AGENTS.md

This file provides guidance to Codex (Codex.ai/code) when working with code in this repository.

## Project Overview

ESP32-KNX-Thermostat is an embedded C++ project for ESP32 that implements a smart thermostat with KNX building automation integration, MQTT support, and a web interface. The system uses an adaptive PID controller for climate control and BME280 sensor for environmental monitoring.

**Platform**: PlatformIO with Arduino framework for ESP32 (NodeMCU-32S board)

## Build and Development Commands

### Building and Uploading
```bash
# Build the project
platformio run

# Build and upload firmware to ESP32
platformio run --target upload

# Upload filesystem (web interface files from data/)
platformio run --target uploadfs

# Clean build files
platformio run --target clean

# Monitor serial output
platformio device monitor
```

### Configuration
- Serial port is configured in `platformio.ini` as `/dev/cu.usbserial-0001`
- Upload/monitor speed: 115200 baud
- If upload fails, check the correct port with `platformio device list`

## Code Architecture

### Core Design Principles

The codebase follows a **modular layered architecture** with distinct separation between:

1. **Device Layer**: Hardware abstraction (BME280 sensor, valve control, WiFi)
2. **Communication Layer**: Protocol handlers (KNX, MQTT, HTTP)
3. **Application Layer**: Business logic (PID controller, configuration management, OTA updates)
4. **Presentation Layer**: User interfaces (web dashboard, Home Assistant integration)

### Key Architectural Patterns

**Singleton Pattern**: Used extensively for manager classes (ConfigManager, WiFiConnectionManager, WebServerManager)
- Always access via `getInstance()` methods
- Thread-safe initialization in most cases

**Thread Safety**:
- KNXManager uses mutex-protected message queues for cross-thread communication
- Use `std::mutex` and `std::lock_guard` patterns when adding concurrent features

**Event-Driven Communication**:
- WiFiConnectionManager uses callback registration system for state changes
- KNX messages processed via callback functions
- Manager classes are cross-referenced (KNXManager ↔ MQTTManager) for protocol bridging

### Critical System Components

#### 1. Main Control Loop (`src/main.cpp`)
The main loop orchestrates:
- Watchdog timer resets (45-minute timeout)
- WiFi connectivity monitoring (60-second interval)
- Sensor reading updates (30-second interval via BME280)
- PID controller updates (10-second interval)
- KNX and MQTT message processing

**Important**: The main loop must call `watchdogManager.update()` regularly to prevent system resets.

#### 2. Adaptive PID Controller (`src/adaptive_pid_controller.cpp`)
Central to temperature regulation:
- Global state variables: `g_pid_input`, `g_pid_output`
- Temperature history buffer (300 samples = 5 minutes at 1s intervals)
- Self-tuning using Ziegler-Nichols method
- Deadband support (±0.2°C default) to prevent oscillation

**Key functions**:
- `initializePIDController()`: Load parameters from ConfigManager
- `updatePIDController(temp, valve_pos)`: Main update called every 10s
- `getPIDOutput()`: Returns calculated valve position (0-100%)
- Setters: `setTemperatureSetpoint()`, `setPidKp/Ki/Kd()`

#### 3. Configuration Management (`src/config_manager.cpp`)
Singleton managing persistent storage using ESP32 Preferences API:
- Network settings (WiFi credentials, MQTT server/port)
- KNX settings (physical address, test/production mode toggle)
- PID parameters (Kp, Ki, Kd, setpoint)
- Watchdog and reboot tracking

**Critical**: Always call `configManager->begin()` in setup before accessing settings.

#### 4. Watchdog System (`src/watchdog_manager.cpp`)
Two-tier watchdog protection:
- **System Watchdog**: 45-minute timeout (ESP32 hardware watchdog)
- **WiFi Watchdog**: 30-minute timeout (software implementation)

Reboot tracking with safe mode after 3 consecutive watchdog resets:
- Enum `RebootReason` defines all reboot causes
- `registerRebootReason()` stores reason before reboot
- Safe mode disables aggressive reconnection attempts

**Usage**: Call `resetSystemWatchdog()` and `resetWiFiWatchdog()` in main loop.

#### 5. WiFi Connection Manager (`src/wifi_connection.cpp`)
Robust WiFi handling with:
- Automatic reconnection (up to 10 attempts)
- Configuration portal fallback (AP: "ESP32-Thermostat-AP")
- Signal strength monitoring and quality metrics
- Event callback system for state changes

**Important**: Integration with WatchdogManager prevents reboot loops during network issues.

#### 6. KNX Manager (`src/knx_manager.cpp`)
Thread-safe KNX communication:
- Message queue pattern for asynchronous operations
- Bidirectional communication with MQTT via cross-manager references
- Address toggling between test/production modes via ConfigManager

**Group Addresses**:
- Sensor data: 0/0/4 (temp), 0/0/5 (humidity), 0/0/6 (pressure)
- Valve control: 1/1/1 (production), 10/2/2 (test mode)

**Usage**: Call `loop()` in main loop to process queued messages.

#### 7. Web Server (`src/web_server.cpp`)
AsyncWebServer providing:
- Dashboard (`data/index.html`) for real-time monitoring
- Configuration interface (`data/config.html`) for settings
- REST API endpoints for JSON data exchange
- OTA firmware update handling

Files served from SPIFFS filesystem (uploaded via `uploadfs` target).

### Manager Cross-Communication

```
WiFiConnectionManager → ConfigManager (save credentials)
                     → WatchdogManager (reset timers)

KNXManager ↔ MQTTManager (protocol bridging)
          → ConfigManager (load addresses)

WebServerManager → ConfigManager (save/load settings)
                → KNXManager (reload addresses after config change)
                → OTAManager (firmware updates)

PIDController → ConfigManager (load/save parameters)
             → KNXManager (send valve commands)
```

## Important Configuration Details

### Hardcoded vs. Runtime Configuration

**Runtime Configurable** (via web interface or ConfigManager):
- WiFi credentials
- MQTT server/port
- KNX physical address (area/line/member)
- KNX test/production address toggle
- PID parameters (Kp, Ki, Kd)
- Temperature setpoint

**Hardcoded** (in `include/config.h`):
- BME280 I²C address (0x76) and pins (SDA=21, SCL=22)
- MQTT topic structure
- Watchdog timeouts (45min system, 30min WiFi)
- Update intervals (10s PID, 30s sensor, 60s WiFi check)
- PID deadband (0.2°C)
- WiFi reconnection attempts (10)

**When adding new configurable parameters**:
1. Add getter/setter to `ConfigManager`
2. Update web interface (`data/config.html`, `data/config.js`)
3. Add to JSON export/import in `ConfigManager::getJson()` and `setFromJson()`

### KNX Address Management

The system supports toggling between test and production KNX addresses:
- Controlled by `ConfigManager::getUseTestAddresses()`
- When changed via web interface, call `knxManager.reloadAddresses()`
- Physical address stored as area/line/member components
- Group addresses defined in `config.h` with _MAIN/_MID/_SUB components

## Coding Style and Conventions

This project follows strict C++ coding guidelines (see `docs/Arduino C Cursorrules.md`):

### Naming Conventions
- **PascalCase**: Classes and structures (`ConfigManager`, `BME280Sensor`)
- **camelCase**: Functions, methods, variables (`getSignalStrength`, `currentTemp`)
- **ALL_CAPS**: Constants and macros (`PID_UPDATE_INTERVAL`, `KNX_AREA`)
- **snake_case**: File names (`adaptive_pid_controller.cpp`)

### Function Design
- Short functions (<20 lines when possible)
- Single responsibility principle
- Use early returns to reduce nesting
- Verb-based naming (`initializePIDController`, `updateSensorReadings`)
- Boolean functions: `isX`, `hasX`, `canX` prefix

### Class Design
- Prefer composition over inheritance
- Use singleton pattern for managers
- Private member variables with public getters/setters
- Doxygen-style comments for public methods
- Follow Rule of Five for resource management

### Memory Management
- Prefer smart pointers in new code (though Arduino framework uses raw pointers)
- Use RAII principles for resource cleanup
- Avoid manual `new`/`delete` when possible

### Error Handling
- Use Logger system (LOG_E, LOG_W, LOG_I, LOG_D) instead of Serial.print
- Store critical errors using `storeLogToFlash()` callback
- Validate inputs in manager methods
- Return bool for success/failure, use output parameters for data

### Thread Safety
- Use `std::mutex` for shared state
- Lock scope should be minimal (`std::lock_guard` preferred)
- Document thread-safety requirements in comments
- Queue-based patterns for cross-thread communication (see KNXManager)

## Common Development Patterns

### Adding a New Sensor
1. Create header in `include/` with sensor interface
2. Implement in `src/` with initialization and reading methods
3. Add instance to global variables in `main.cpp`
4. Initialize in `setup()` after Wire.begin()
5. Update reading in main loop (consider timing intervals)
6. Add KNX/MQTT publishing if needed
7. Update web interface to display data

### Adding a New Web API Endpoint
1. Add handler function to `WebServerManager` class
2. Register in `setupDefaultRoutes()` or via `addEndpoint()`
3. Use ArduinoJson for JSON responses
4. Add corresponding JavaScript in `data/script.js` or `data/config.js`
5. Update HTML interface in `data/index.html` or `data/config.html`

### Adding a New Configuration Parameter
1. Add private member and getter/setter to `ConfigManager`
2. Add Preferences key constant (keep keys <15 chars for ESP32)
3. Implement in `getJson()` and `setFromJson()` for web interface
4. Add to `data/config.html` form
5. Add JavaScript handling in `data/config.js`
6. Update dependent components to use new parameter

### Modifying PID Behavior
- PID state is global (`g_pid_input`, `g_pid_output`)
- Update interval controlled by `PID_UPDATE_INTERVAL` (10s)
- Modification should preserve temperature history for auto-tuning
- Test with both deadband active and inactive scenarios
- Verify MQTT and KNX output after changes

## Debugging and Monitoring

### Serial Logging
The Logger system provides tagged output:
```cpp
LOG_E("TAG", "Error message");   // Error
LOG_W("TAG", "Warning message"); // Warning
LOG_I("TAG", "Info message");    // Info
LOG_D("TAG", "Debug message");   // Debug
```

Common tags: `MAIN`, `WIFI`, `KNX`, `MQTT`, `PID`, `SENSOR`, `WATCHDOG`

Set log level in `main.cpp`: `Logger::getInstance().setLogLevel(LOG_INFO)`

### Watchdog Debugging
If experiencing unexpected reboots:
1. Check `ConfigManager::getLastRebootReason()`
2. Review `getConsecutiveWatchdogReboots()` count
3. Increase watchdog timeout temporarily for debugging
4. Add `watchdogManager.pauseWatchdogs(ms)` before long operations

### WiFi Connection Issues
Use WiFiConnectionManager diagnostic methods:
- `getState()` - current connection state
- `getSignalStrength()` - RSSI value
- `getReconnectAttempts()` - failed reconnection count
- `testInternetConnectivity()` - ping test (8.8.8.8)
- `getConnectionDetailsJson()` - full diagnostic output

### Web Interface Access
1. Connect to same network as ESP32
2. Find IP via serial monitor or router DHCP list
3. Access: `http://<ESP32-IP>/`
4. Config portal (if WiFi fails): Connect to "ESP32-Thermostat-AP" AP

## Testing and Validation

When making changes:
1. Verify compilation: `platformio run`
2. Check for warnings (project uses `-D CORE_DEBUG_LEVEL=1`)
3. Test watchdog doesn't trigger during normal operation
4. Verify WiFi reconnection after router reboot
5. Test both KNX test and production address modes
6. Validate configuration persistence across reboots
7. Check MQTT and KNX message publishing
8. Test web interface on mobile and desktop browsers

## Known Constraints and Considerations

1. **ESP32 Memory**: Monitor heap usage, especially with large JSON documents
2. **Filesystem**: SPIFFS has limited space for web interface files
3. **WiFi Stability**: Long WiFi operations can trigger watchdog; use `pauseWatchdogs()`
4. **KNX Timing**: KNX message processing must not block main loop
5. **PID Tuning**: Auto-tuning requires stable setpoint for 5+ minutes
6. **Safe Mode**: After 3 consecutive watchdog reboots, system enters safe mode with limited functionality

## Libraries and Dependencies

Core libraries (auto-installed by PlatformIO):
- **Adafruit BME280**: Environmental sensor (I²C)
- **PubSubClient**: MQTT client
- **WiFiManager**: Captive portal for WiFi setup
- **ESPAsyncWebServer**: Async HTTP server
- **AsyncTCP**: Async networking
- **ArduinoJson**: JSON parsing/generation
- **ESP32Ping**: Network connectivity testing

Custom/embedded library:
- **esp-knx-ip**: KNX/IP protocol stack (ported to ESP32, in `src/esp-knx-ip/`)

## Home Assistant Integration

MQTT discovery automatically creates:
- **Climate entity**: `esp32_thermostat_thermostat` (mode control, temperature setpoint)
- **Sensors**: temperature, humidity, pressure, valve position
- **Topics**: All under `esp32_thermostat/` prefix

Default MQTT server: `192.168.178.32:1883` (change via web interface)

## Additional Resources

- API documentation: `docs/API.md`
- PID analysis: `docs/PID Analysis.md`
- Project audit: `docs/AUDIT_REPORT.md`
- Implementation plans: `docs/IMPLEMENTATION_PLAN.md`
