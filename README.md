# ESP32-KNX-Thermostat

[![CI/CD Pipeline](https://github.com/coolbox13/Minimal-Thermostat/actions/workflows/ci.yml/badge.svg)](https://github.com/coolbox13/Minimal-Thermostat/actions/workflows/ci.yml)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange.svg)](https://platformio.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Last Commit](https://img.shields.io/github/last-commit/coolbox13/Minimal-Thermostat)](https://github.com/coolbox13/Minimal-Thermostat/commits/main)

A modular smart thermostat system built on ESP32 that integrates with KNX building automation networks, providing advanced climate control with multiple connectivity options.

![KNX Thermostat Dashboard](https://raw.githubusercontent.com/username/ESP32-KNX-Thermostat/main/docs/images/dashboard.png)

## Features

- **Advanced Climate Control**: 
  - Adaptive PID-based temperature regulation with self-tuning capability
  - Manual override functionality with timeout support
  - Sensor validation to prevent invalid readings from affecting control
  - Write coalescing for flash wear reduction (PID parameters saved max once per 5 minutes)
  
- **Multi-Protocol Support**: 
  - Native KNX integration for building automation
  - MQTT connectivity for home automation systems
  - Web interface for direct control and configuration
  - Webhook integration for IFTTT, Zapier, and custom automation
  
- **Sensor Integration**: 
  - BME280 temperature/humidity/pressure monitoring
  - 24-hour historical data storage (circular buffer, 5-minute intervals)
  - Real-time sensor readings with configurable update intervals
  
- **Robust Connectivity**: 
  - Advanced WiFi connection manager with event-driven architecture
  - Automatic reconnection with configurable attempts
  - Internet connectivity testing (ping-based)
  - WiFi signal strength monitoring
  - Configuration portal when connection fails
  - Dual watchdog system:
    - 45-minute system watchdog for automatic recovery
    - 30-minute WiFi watchdog for connection monitoring
  
- **Home Assistant Integration**: 
  - Full climate entity support with auto-discovery via MQTT
  - Automatic device registration
  - Supports on/off and heating modes
  - Temperature setpoint adjustment
  
- **Web Interface (PWA)**: 
  - Progressive Web App with offline support
  - Multiple pages: Dashboard, Status, Configuration, Event Logs
  - Real-time data visualization
  - Historical data graphing (24-hour view)
  - Mobile-responsive design
  
- **Event Logging & Monitoring**: 
  - Persistent event log storage in SPIFFS (100 entries)
  - MQTT log publishing for remote monitoring
  - Filterable logs by level and tag
  - Web-based log viewer
  
- **OTA Updates**: 
  - Remote firmware upgrades via web interface
  - Safe update process with rollback capability
  
- **Persistent Configuration**: 
  - Settings stored in non-volatile memory (Preferences API)
  - Configuration manager with web interface
  - PID parameters persistence
  - WiFi credentials storage

## Architecture

### System Components
The thermostat uses a modular design with specialized components:

```mermaid
graph TB
    classDef core fill:#f9f,stroke:#333,stroke-width:2px
    classDef protocols fill:#bbf,stroke:#333,stroke-width:2px
    classDef peripherals fill:#bfb,stroke:#333,stroke-width:2px
    classDef ui fill:#fbb,stroke:#333,stroke-width:2px
    classDef storage fill:#ffb,stroke:#333,stroke-width:2px
    
    %% Core Components
    Main[Main Controller]
    PID[Adaptive PID Controller]
    Config[Configuration Manager]
    Logger[Logger System]
    
    %% Protocol Handlers
    KNX[KNX Manager]
    MQTT[MQTT Manager]
    HA[Home Assistant]
    WebServer[Web Server Manager]
    
    %% Peripherals
    BME280[BME280 Sensor]
    Valve[Valve Control]
    OTA[OTA Manager]
    
    %% External Systems
    KNXBus((KNX Bus))
    MQTTBroker((MQTT Broker))
    Browser((Web Browser))
    
    %% Core Relationships
    Main --> PID
    Main --> Config
    Main --> Logger
    
    %% Main to Protocols
    Main --> KNX
    Main --> MQTT
    Main --> WebServer
    
    %% Main to Peripherals
    Main --> BME280
    Main --> Valve
    Main --> OTA
    
    %% Protocol Interconnections
    MQTT --> HA
    KNX <--> MQTT
    
    %% External Connections
    KNX --> KNXBus
    MQTT --> MQTTBroker
    WebServer --> Browser
    OTA --> Browser
    
    %% Apply Classes
    class Main,PID,Config,Logger core
    class KNX,MQTT,HA,WebServer protocols
    class BME280,Valve,OTA peripherals
    class KNXBus,MQTTBroker,Browser ui
```

### Layered Architecture
The software is organized in a layered architecture for maintainability:

```mermaid
graph TD
    subgraph "Presentation Layer"
        WebUI[Web Interface]
        HAMI[Home Assistant Integration]
    end
    
    subgraph "Application Layer"
        PID[Adaptive PID Controller]
        Config[Configuration Manager]
        OTA[OTA Update Manager]
    end
    
    subgraph "Communication Layer"
        KNX[KNX Protocol Manager]
        MQTT[MQTT Protocol Manager]
        HTTP[HTTP Web Server]
    end
    
    subgraph "Device Layer"
        BME280[Temperature/Humidity/Pressure Sensor]
        Valve[Heating Valve Control]
        WiFi[WiFi Manager]
    end
    
    subgraph "Hardware Abstraction Layer"
        Arduino[Arduino Framework]
        ESP32[ESP32 SDK]
    end
    
    %% Connections between layers
    WebUI --> HTTP
    WebUI --> PID
    HAMI --> MQTT
    
    PID --> BME280
    PID --> Valve
    PID --> KNX
    Config --> PID
    Config --> KNX
    Config --> MQTT
    OTA --> HTTP
    OTA --> ESP32
    
    KNX --> Arduino
    MQTT --> WiFi
    HTTP --> WiFi
    
    BME280 --> Arduino
    Valve --> Arduino
    WiFi --> ESP32
    
    Arduino --> ESP32
```

### PID Control Flow
The adaptive PID controller is central to temperature regulation:

```mermaid
sequenceDiagram
    participant Main as Main Controller
    participant Sensor as BME280 Sensor
    participant PID as Adaptive PID Controller
    participant History as Temperature History
    participant KNX as KNX Manager
    participant Valve as Valve Actuator
    
    Note over Main,Valve: Initialization Phase
    Main->>PID: initializePIDController()
    PID->>PID: Set default parameters
    
    Note over Main,Valve: Regular Control Cycle
    loop Every PID_UPDATE_INTERVAL
        Main->>Sensor: readTemperature()
        Sensor-->>Main: current_temp
        Main->>PID: updatePIDController(current_temp, valve_position)
        
        activate PID
        PID->>History: Store temperature
        PID->>PID: Calculate error
        
        alt Temperature in deadband
            PID->>PID: Maintain current valve position
        else Temperature outside deadband
            PID->>PID: Calculate integral error
            PID->>PID: Calculate derivative error
            PID->>PID: Compute raw PID output
            PID->>PID: Apply output limits
        end
        
        PID->>PID: Update performance metrics
        
        alt Adaptation is enabled and timer expired
            PID->>PID: Analyze performance
            PID->>PID: Adjust PID parameters
        end
        deactivate PID
        
        PID-->>Main: getPIDOutput()
        Main->>KNX: setValvePosition(pid_output)
        KNX->>Valve: Apply new position
    end
```

## Hardware Requirements

- ESP32 development board (NodeMCU ESP32-S recommended)
- BME280 sensor module
- I²C connection cables
- Power supply (USB or 5V DC)

### Wiring Diagram

```
┌────────────┐           ┌────────────┐
│            │           │            │
│            │ SDA (21)  │            │
│   ESP32    ├───────────┤  BME280    │
│            │ SCL (22)  │            │
│            ├───────────┤            │
│            │ 3.3V      │            │
│            ├───────────┤            │
│            │ GND       │            │
└────────────┘           └────────────┘
```

## Getting Started

### Prerequisites

- PlatformIO IDE (recommended) or Arduino IDE
- Git client

### Installation

1. Clone this repository:
   ```
   git clone https://github.com/coolbox13/ESP32-KNX-Thermostat.git
   ```

2. Open the project in PlatformIO or configure Arduino IDE with required libraries:
   ```
   cd ESP32-KNX-Thermostat
   platformio project init
   ```

3. Review and update configuration settings:
   - Network settings (WiFi credentials)
   - KNX physical/group addresses 
   - MQTT broker details
   - Temperature control parameters

4. Build and upload:
   ```
   platformio run --target upload
   ```

5. Upload file system image for the web interface:
   ```
   platformio run --target uploadfs
   ```

### Initial Configuration

After uploading, the thermostat creates a WiFi access point named "ESP32-Thermostat-AP" if it can't connect to a saved network. Connect to this network to configure:

1. WiFi credentials
2. KNX physical/group addresses (can be toggled between test/production)
3. MQTT server details
4. Temperature control parameters

## Web Interface

The thermostat provides a comprehensive Progressive Web App (PWA) with multiple pages:

### Dashboard (`/`)
- Real-time monitoring of temperature, humidity, pressure, and valve position
- Temperature setpoint adjustment
- Manual valve override control
- PID parameter display and adjustment
- Historical data graph (24-hour view)

### Status Page (`/status`)
- System status overview
- WiFi connection information
- Memory and flash usage statistics
- Sensor health status

### Configuration Page (`/config`)
- WiFi credentials configuration
- KNX physical and group addresses (test/production toggle)
- MQTT broker settings
- PID parameters configuration
- System settings (intervals, timeouts)

### Event Logs Page (`/logs`)
- View persistent event logs
- Filter by log level (INFO, WARNING, ERROR)
- Filter by tag/category
- Export logs functionality
- Real-time log updates

### Features
- **Progressive Web App**: Installable on mobile devices, works offline
- **Responsive Design**: Optimized for desktop, tablet, and mobile
- **Real-time Updates**: WebSocket-like updates for live data
- **Dark Theme**: Modern, easy-on-the-eyes interface

![Web Interface](https://raw.githubusercontent.com/username/ESP32-KNX-Thermostat/main/docs/images/web_interface.png)

## KNX Integration

The thermostat uses standard KNX datapoints:
- Temperature: DPT 9.001 (2-byte float)
- Setpoint: DPT 9.001 (2-byte float)
- Valve Position: DPT 5.001 (1-byte percentage)
- Operating Mode: DPT 20.102 (1-byte HVAC mode)

### KNX Addressing

Both test and production addresses are supported:
- Temperature Sensor: 0/0/4
- Humidity Sensor: 0/0/5
- Pressure Sensor: 0/0/6
- Valve Control: 1/1/1 (production) or 10/2/2 (test)

## Home Assistant Integration

The device automatically registers with Home Assistant via MQTT discovery:
- Creates a climate entity for temperature control
- Provides sensors for temperature, humidity, pressure, and valve position
- Supports on/off and heating modes
- Allows temperature setpoint adjustment

Example Home Assistant configuration:
```yaml
# Configuration is automatic via MQTT discovery
# No manual YAML configuration required!
```

## Project Structure

```
ESP32-KNX-Thermostat/
├── data/                        # Web interface files (SPIFFS)
│   ├── index.html               # Main dashboard
│   ├── status.html              # Status page
│   ├── config.html              # Configuration page
│   ├── logs.html                # Event logs viewer
│   ├── style.css                # Shared styling
│   ├── script.js                # Dashboard functionality
│   ├── status.js                # Status page scripts
│   ├── config.js                # Configuration page scripts
│   └── manifest.json            # PWA manifest
├── include/                     # Header files
│   ├── adaptive_pid_controller.h # PID controller interface
│   ├── bme280_sensor.h          # Temperature sensor interface
│   ├── config.h                 # Configuration constants
│   ├── config_manager.h         # Configuration manager
│   ├── event_log.h              # Persistent event logging
│   ├── history_manager.h        # Historical data storage
│   ├── home_assistant.h         # HA integration
│   ├── knx_manager.h            # KNX protocol manager
│   ├── logger.h                 # Logging system
│   ├── mqtt_manager.h           # MQTT protocol manager
│   ├── ota_manager.h            # OTA update manager
│   ├── persistence_manager.h    # Persistent storage abstraction
│   ├── utils.h                  # Utility functions
│   ├── valve_control.h          # Valve control interface
│   ├── watchdog_manager.h       # Watchdog system
│   ├── web_server.h             # Web server manager
│   ├── webhook_manager.h        # Webhook integration
│   ├── wifi_connection.h        # WiFi connection manager
│   └── wifi_connection_events.h # WiFi event system
├── src/                         # Implementation files
│   ├── adaptive_pid_controller.cpp
│   ├── bme280_sensor.cpp
│   ├── config_manager.cpp
│   ├── event_log.cpp
│   ├── history_manager.cpp
│   ├── home_assistant.cpp
│   ├── knx_manager.cpp
│   ├── logger.cpp
│   ├── main.cpp                 # Main application
│   ├── mqtt_manager.cpp
│   ├── ota_manager.cpp
│   ├── persistence_manager.cpp
│   ├── utils.cpp
│   ├── valve_control.cpp
│   ├── watchdog_manager.cpp
│   ├── web_server.cpp
│   ├── webhook_manager.cpp
│   └── wifi_connection.cpp
├── test/                        # Unit tests
│   └── test_utils/
│       └── test_precision.cpp
├── docs/                        # Documentation
├── partitions_custom.csv        # Custom partition table
├── platformio.ini               # PlatformIO configuration
└── README.md                    # This file
```

## Development

### Adding New Sensors

The project follows an interface-based architecture that facilitates extension. To add a new sensor:

1. Create a new class implementing the sensor interface
2. Add the necessary initialization in `main.cpp`
3. Add data processing for the sensor readings
4. Update the web interface to display the new data

### Extending the Web Interface

The web interface uses standard HTML, CSS and JavaScript:

1. Edit the files in the `data` directory
2. Add new API endpoints in `web_server.cpp`
3. Upload the file system image to update the interface

## Troubleshooting

### WiFi Connection Issues

The system includes several recovery mechanisms:
- **Automatic Reconnection**: Up to 10 reconnection attempts with configurable timeouts
- **Configuration Portal**: Automatically starts when connection fails
- **Watchdog System**: 
  - System watchdog (45 minutes) for automatic reboot on system hang
  - WiFi watchdog (30 minutes) for connection monitoring
- **Internet Connectivity Testing**: Periodic ping tests to verify internet access
- **Signal Strength Monitoring**: Tracks WiFi signal quality over time

Check the serial console for diagnostic messages:
```
[WIFI] WiFi connection lost. Attempting to reconnect...
[WIFI] Reconnection attempt 1 of 10 failed
...
[WIFI] Multiple reconnection attempts failed. Starting config portal...
```

### Event Logging

The system maintains a persistent event log stored in SPIFFS:
- Access logs via web interface at `/logs`
- View logs via MQTT topic `esp32_thermostat/logs`
- Logs are automatically filtered (only WARNING and ERROR stored by default)
- Maximum 100 log entries (circular buffer)

### Sensor Issues

- Invalid sensor readings (NaN, infinity, out of range) are automatically rejected
- PID control skips cycles with invalid readings to prevent system instability
- Check event logs for sensor error messages

### Memory Management

The system logs memory usage at startup:
- RAM usage statistics
- Flash/OTA partition usage
- SPIFFS usage
- Warnings when usage exceeds 90% or 95%

### Historical Data

The system maintains a 24-hour rolling history of sensor readings:
- Stores temperature, humidity, pressure, and valve position
- One data point every 5 minutes (288 points total)
- Accessible via web interface dashboard graph
- API endpoint: `/api/history` (returns JSON)
- Circular buffer automatically overwrites oldest data

## Hardware-Encoded Settings Table

|Setting|Current Change Method|Recommended Change Method|
|---|---|---|
|WiFi credentials|Web interface, WiFiManager|Keep current|
|KNX physical address|KNX web interface|Keep current|
|KNX group addresses|KNX web interface|Add to web config|
|MQTT server & port|Web interface|Keep current|
|PID parameters (Kp, Ki, Kd)|Web interface|Keep current|
|Temperature setpoint|Web, MQTT, KNX|Keep current|
|Valve position|Web, MQTT, KNX|Keep current|
|BME280 I²C address|Hardcoded (0x76)|Add to web config|
|I²C pins (SDA/SCL)|Hardcoded (21/22)|Add to web config|
|Debug level|Hardcoded|Add to web config|
|MQTT topics|Hardcoded|Add to web config|
|KNX test addresses|Hardcoded|Add to web config|
|Watchdog timeout|Hardcoded (45 min)|Add to web config|
|WIFI reconnect attempts|Hardcoded (10)|Add to web config|
|Sensor update interval|Hardcoded (30s)|Add to web config|
|PID update interval|Hardcoded (10s)|Add to web config|

## Bus Communication Table

### MQTT Bus

|Direction|Topic|Purpose|
|---|---|---|
|Input|esp32_thermostat/mode/set|Set thermostat mode (heat/off)|
|Input|esp32_thermostat/temperature/set|Set temperature setpoint|
|Input|esp32_thermostat/valve/set|Set valve position directly|
|Input|esp32_thermostat/restart|Trigger device restart|
|Output|esp32_thermostat/status|Device online status|
|Output|esp32_thermostat/temperature|Current temperature|
|Output|esp32_thermostat/humidity|Current humidity|
|Output|esp32_thermostat/pressure|Current pressure|
|Output|esp32_thermostat/valve/position|Current valve position|
|Output|esp32_thermostat/mode/state|Current thermostat mode|
|Output|esp32_thermostat/temperature/setpoint|Current temperature setpoint|
|Output|esp32_thermostat/action|Current action (heating/idle)|
|Output|esp32_thermostat/logs|Event log entries (JSON format)|

### KNX Bus

|Direction|Group Address|Purpose|
|---|---|---|
|Input|10/2/2 (test)|Valve control input|
|Input|1/1/1 (production)|Valve control input|
|Output|0/0/4|Temperature sensor value|
|Output|0/0/5|Humidity sensor value|
|Output|0/0/6|Pressure sensor value|
|Output|10/2/2|Valve position output (test address)|
|Output|1/1/1|Valve position output (production address)|

## Recommendations

1. **Hardcoded Settings**: Add configuration options in the web interface for all hardcoded settings, particularly:
    
    - I²C address and pins for BME280
    - MQTT topics structure
    - KNX group addresses
    - Timing parameters (watchdog, intervals)
    
2. **Communication Protocol**:
    
    - Implement bidirectional feedback for the valve position in KNX
    - Add proper KNX datapoint types for thermostat mode (DPT 20.102)
    - Enable temperature setpoint control via KNX (currently only implemented in MQTT)
    
3. **Security Improvements**:
    
    - Add MQTT username/password to web configuration
    - Implement secure storage for credentials
    - Add authentication for the web interface
    - Consider TLS for MQTT connections
    
4. **User Experience**:
    
    - Create a settings export/import feature
    - Implement profiles for different operating modes
    - Add factory reset option
    - Improve mobile responsiveness of the web interface
    
5. **Robustness**:
    
    - ✅ Log important events to persistent storage (IMPLEMENTED)
    - ✅ Implement sensor failure detection and fallback (IMPLEMENTED - invalid readings rejected)
    - Add energy-saving mode
    - Consider adding a battery backup option

## Home Assistant Integration Table

|Direction|Entity Type|Entity ID|Purpose|Data Type|
|---|---|---|---|---|
|Input|climate|esp32_thermostat_thermostat|Set mode, temperature|modes, temperature|
|Output|climate|esp32_thermostat_thermostat|Thermostat status|temperature, setpoint, mode, action|
|Output|sensor|esp32_thermostat_temperature|Current temperature|°C|
|Output|sensor|esp32_thermostat_humidity|Current humidity|%|
|Output|sensor|esp32_thermostat_pressure|Current pressure|hPa|
|Output|sensor|esp32_thermostat_valve_position|Valve position|%|
|Output|sensor|availability|Device online status|online/offline|

## Recommendations for Home Assistant Integration

1. **Additional Entities to Implement**:
    
    - **Energy sensor**: Add power consumption tracking for energy dashboards
    - **Thermostat mode sensor**: More detailed operating modes (comfort, away, night)
    - **Diagnostic entities**: WiFi signal strength, uptime, last error
    - **Maintenance sensor**: BME280 sensor health status
    
2. **Entity Customization**:
    
    - Add device class and state class attributes for proper categorization
    - Implement unit_of_measurement consistently
    - Add min/max attributes for numerical sensors
    
3. **Advanced Integration**:
    
    - Create a dashboard template that can be easily imported
    - Add energy management entities for consumption tracking
    - Implement proper device class for climate entity for better automation compatibility
    - Add service calls for advanced functions like PID parameter tuning
    
4. **Additional Features**:
    
    - Weekly schedule programming via Home Assistant
    - Presence detection integration for smarter heating
    - Enable multiple temperature presets (comfort, eco, away)
    - Implement temperature offset calibration capability

## Contributing

No intention of developing further for other use besides my own home. Please fork and go ahead.

Special thanks to Nico Weichbrodt <envy> for building an ESP library for KNX communiocation. I used it to port to ESP32 and make this thermostat possible.

esp-knx-ip library for KNX/IP communication on an ESP32
 * Ported from ESP8266 version
 * Author: Nico Weichbrodt <envy> (Original), Modified for ESP32
 * License: MIT

## License

This project is released under the MIT License.