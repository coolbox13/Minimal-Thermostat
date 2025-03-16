# ESP32-KNX-Thermostat

A modular smart thermostat system built on ESP32 that integrates with KNX building automation networks, providing advanced climate control with multiple connectivity options.

![KNX Thermostat Dashboard](https://raw.githubusercontent.com/username/ESP32-KNX-Thermostat/main/docs/images/dashboard.png)

## Features

- **Advanced Climate Control**: Adaptive PID-based temperature regulation with self-tuning capability
- **Multi-Protocol Support**: 
  - Native KNX integration for building automation
  - MQTT connectivity for home automation systems
  - Web interface for direct control and configuration
- **Sensor Integration**: BME280 temperature/humidity/pressure monitoring
- **Robust Connectivity**: 
  - WiFi with advanced reconnection system
  - Fallback mechanisms and configuration portal
  - 45-minute watchdog timer for automatic recovery
- **Home Assistant Integration**: Full climate entity support with auto-discovery
- **OTA Updates**: Remote firmware upgrades via web interface
- **Persistent Configuration**: Settings stored in non-volatile memory
- **Flexible Operation Modes**: on or off...

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

The thermostat provides a responsive web interface for:
- Monitoring current temperature, humidity, pressure, and valve position
- Adjusting temperature setpoint
- Configuring system settings
- Updating firmware

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
├── data/                        # Web interface files
│   ├── index.html               # Main dashboard
│   ├── style.css                # Styling 
│   └── script.js                # Dashboard functionality
├── include/                     # Header files
│   ├── adaptive_pid_controller.h # PID controller interface
│   ├── bme280_sensor.h          # Temperature sensor interface
│   ├── config.h                 # Configuration settings
│   ├── config_manager.h         # Configuration manager
│   ├── home_assistant.h         # HA integration
│   ├── knx_manager.h            # KNX protocol manager
│   ├── logger.h                 # Logging system
│   ├── mqtt_manager.h           # MQTT protocol manager
│   ├── ota_manager.h            # OTA update manager
│   ├── utils.h                  # Utility functions
│   └── valve_control.h          # Valve control interface
├── src/                         # Implementation files
│   ├── adaptive_pid_controller.cpp
│   ├── bme280_sensor.cpp
│   ├── config_manager.cpp
│   ├── home_assistant.cpp
│   ├── knx_manager.cpp
│   ├── logger.cpp
│   ├── main.cpp                 # Main application
│   ├── mqtt_manager.cpp
│   ├── ota_manager.cpp
│   ├── utils.cpp
│   ├── valve_control.cpp
│   └── web_server.cpp
└── platformio.ini               # PlatformIO configuration
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
- Automatic reconnection attempts
- Configuration portal when connection fails
- Watchdog timer for automatic reboot

Check the serial console for diagnostic messages:
```
[WIFI] WiFi connection lost. Attempting to reconnect...
[WIFI] Reconnection attempt 1 of 10 failed
...
[WIFI] Multiple reconnection attempts failed. Starting config portal...
```

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
    
    - Log important events to persistent storage
    - Implement sensor failure detection and fallback
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