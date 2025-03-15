# ESP32-KNX-Thermostat

A modular smart thermostat system built on ESP32 that integrates with KNX building automation networks, providing advanced climate control with multiple connectivity options.

## Features

- **Multi-Protocol Support**: Native KNX integration plus MQTT connectivity for home automation systems
- **Advanced Climate Control**: Adaptive PID-based temperature regulation for precise comfort
- **Sensor Integration**: BME280 temperature/humidity/pressure monitoring
- **Robust Connectivity**: Advanced WiFi reconnection system with fallback mechanisms and watchdog protection
- **Modular Architecture**: Interface-based design for easy extension
- **Planned Features**:
  - OTA Updates: Remote firmware upgrades (coming soon)
  - Web Interface: Mobile-responsive configuration dashboard (in development)
  - **Flexible Operation Modes**: Comfort, Eco, Away, Boost, and Anti-freeze
  - Secure Connectivity**: Optional TLS support for MQTT connections

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
   
3. Set your configuration in `data/config.json` or use the web interface after installation

4. Build and upload to your ESP32 device

### Initial Configuration

After uploading, the thermostat creates a WiFi access point named "Thermostat-Setup". Connect to this network to configure:

1. WiFi credentials
2. KNX physical/group addresses
3. Optional MQTT server details
4. Temperature control parameters

## KNX Integration

The thermostat uses standard KNX datapoints:
- Temperature: DPT 9.001 (2-byte float)
- Setpoint: DPT 9.001 (2-byte float)
- Valve Position: DPT 5.001 (1-byte percentage)
- Operating Mode: DPT 20.102 (1-byte HVAC mode)

## Project Structure

```
ESP32-KNX-Thermostat/
├── include/                 # Header files
│   ├── interfaces/          # Abstract interfaces
│   ├── communication/       # Protocol implementations
│   ├── sensors/             # Sensor implementations
│   └── web/                 # Web interface
├── src/                     # Implementation files
├── data/                    # Web files and configuration
└── platformio.ini           # PlatformIO configuration
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