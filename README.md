# ESP32-KNX-Thermostat

A comprehensive thermostat solution for ESP32/ESP8266 with KNX integration, providing direct control of KNX heating valves.

## Features

- **KNX Integration**: Direct communication with KNX heating valves and other KNX devices
- **Temperature, Humidity and Pressure Sensing**: Uses BME280 sensor for accurate environmental measurements
- **PID Controller**: Advanced temperature control with configurable PID parameters
- **Web Configuration Interface**: Easy setup through a browser-based interface
- **WiFi Manager**: Simple WiFi configuration through captive portal
- **MQTT Support**: Optional integration with Home Assistant and other MQTT systems
- **Multiple Operating Modes**: Comfort, Economy, Away, and Anti-freeze modes
- **Persistent Configuration**: All settings stored in flash memory
- **Multi-Platform**: Works on both ESP32 and ESP8266

## Hardware Requirements

- ESP32 or ESP8266 development board (NodeMCU ESP32-S recommended)
- BME280 temperature/humidity/pressure sensor
- Power supply (5V or 3.3V depending on your board)
- Optional: Status LED

## KNX Integration

The thermostat uses standard KNX datapoints:
- Temperature: DPT 9.001 (2-byte float)
- Setpoint: DPT 9.001 (2-byte float)
- Valve Position: DPT 5.001 (1-byte percentage)
- Operating Mode: DPT 5.xxx (1-byte unsigned value)

## Installation

### Using PlatformIO (Recommended)

1. Clone this repository
2. Open the project in PlatformIO
3. Configure platformio.ini if needed (board type, etc.)
4. Build and upload to your device

### Using Arduino IDE

1. Install all required libraries through the Arduino Library Manager
2. Copy the files into your sketch folder
3. Open the main sketch in Arduino IDE
4. Select your board and upload

## Initial Setup

1. After uploading the code, the device will create a WiFi access point named "KNX-Thermostat"
2. Connect to this network using your smartphone or computer
3. Follow the captive portal instructions to connect the device to your WiFi network
4. Once connected, access the web interface at http://knx-thermostat.local or the IP address shown in the serial monitor
5. Configure your KNX settings according to your KNX installation

## Configuration Options

Through the web interface, you can configure:

- **MQTT Settings**: Server, port, credentials
- **KNX Settings**: Physical address and group addresses
- **PID Controller**: Proportional, Integral, and Derivative parameters
- **Setpoint**: Target temperature
- **Timing**: Update intervals for sensor readings and PID calculations

## Library Dependencies

- ArduinoJson
- PubSubClient
- WiFiManager
- Adafruit BME280 Library
- Adafruit Unified Sensor
- ArduinoThread
- esp-knx-ip

## Development

This project follows a modular architecture with the following components:

- **Thermostat State**: Central state management with event callbacks
- **Sensor Interface**: Handles reading from BME280
- **KNX Interface**: Manages KNX communication
- **MQTT Interface**: Handles MQTT communication
- **Web Interface**: Provides configuration UI
- **PID Controller**: Implements the control algorithm

## License

This project is released under the MIT License.