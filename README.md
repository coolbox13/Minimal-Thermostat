# ESP32-KNX-Thermostat

A modular smart thermostat system built on ESP32 that integrates with KNX building automation networks, providing advanced climate control with multiple connectivity options.

## Features

- **Multi-Protocol Support**: Native KNX integration plus MQTT connectivity for home automation systems
- **Advanced Climate Control**: Adaptive PID-based temperature regulation for precise comfort
- **Sensor Integration**: BME280 temperature/humidity/pressure monitoring
- **Robust Connectivity**: Advanced WiFi reconnection system with fallback mechanisms and watchdog protection
- **Modular Architecture**: Interface-based design for easy extension
- **OTA Updates**: Remote firmware upgrades via web interface
- **Home Assistant Integration**: Full climate entity support with setpoint control
- **Additional Features**:
  - 45-minute watchdog timer for automatic recovery
  - Toggle between test/production KNX addresses
  - Remote restart via MQTT command
  - Web Interface: Mobile-responsive configuration dashboard
  - **Flexible Operation Modes**: Comfort, Eco, Away, Boost, and Anti-freeze
  - **Secure Connectivity**: Optional TLS support for MQTT connections

## Hardware Requirements

- ESP32 development board (NodeMCU ESP32-S recommended)
- BME280 sensor module
- I²C connection cables
- Power supply (USB or 5V DC)

## Getting Started

### Prerequisites

- PlatformIO IDE (recommended) or Arduino IDE
- Git client

### Installation

1. Clone this repository:
   ```
   git clone https://github.com/yourusername/ESP32-KNX-Thermostat.git
   ```

2. Open the project in PlatformIO or configure Arduino IDE with required libraries
   
3. Set your configuration in `config.h` before uploading

4. Build and upload to your ESP32 device

### Initial Configuration

After uploading, the thermostat creates a WiFi access point named "ESP32-Thermostat-AP". Connect to this network to configure:

1. WiFi credentials
2. KNX physical/group addresses (can be toggled between test/production in config.h)
3. Optional MQTT server details
4. Temperature control parameters

### OTA Updates

Once configured, OTA updates can be performed by:
1. Navigate to http://[device-ip]/update in a web browser
2. Select the new firmware file (.bin)
3. Click "Update" and wait for the device to reboot

## KNX Integration

The thermostat uses standard KNX datapoints:
- Temperature: DPT 9.001 (2-byte float)
- Setpoint: DPT 9.001 (2-byte float)
- Valve Position: DPT 5.001 (1-byte percentage)
- Operating Mode: DPT 20.102 (1-byte HVAC mode)

## Home Assistant Integration

The device automatically registers with Home Assistant via MQTT discovery:
- Creates a climate entity for temperature control
- Provides sensors for temperature, humidity, pressure, and valve position
- Supports on/off and heating modes
- Allows temperature setpoint adjustment

## Project Structure

```
ESP32-KNX-Thermostat/
├── include/                     # Header files
│   ├── esp-knx-ip/              # KNX/IP library headers
│   ├── adaptive_pid_controller.h # PID controller interface
│   ├── bme280_sensor.h          # Temperature sensor interface
│   ├── config.h                 # Configuration settings
│   ├── home_assistant.h         # HA integration
│   ├── knx_manager.h            # KNX protocol manager
│   ├── mqtt_manager.h           # MQTT protocol manager
│   ├── ota_manager.h            # OTA update manager
│   ├── utils.h                  # Utility functions
│   └── valve_control.h          # Valve control interface
├── src/                         # Implementation files
│   ├── esp-knx-ip/              # KNX/IP library implementation
│   ├── adaptive_pid_controller.cpp
│   ├── bme280_sensor.cpp
│   ├── home_assistant.cpp
│   ├── knx_manager.cpp
│   ├── main.cpp                 # Main application
│   ├── mqtt_manager.cpp
│   ├── ota_manager.cpp
│   ├── utils.cpp
│   └── valve_control.cpp
├── data/                        # Web files and configuration
└── platformio.ini               # PlatformIO configuration
```

## Development

The project follows an interface-based architecture that facilitates extension:
- Add new sensors by implementing the `SensorInterface`
- Add new communication protocols via `ProtocolInterface`
- Customize control algorithms through `ControlInterface`

## License

This project is released under the MIT License.

## Contributing

Contributions are welcome! Please follow the existing architecture and coding style when submitting pull requests.