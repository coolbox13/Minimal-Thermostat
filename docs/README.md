# ESP32 KNX Gateway with AsyncWebServer

This project implements a KNX gateway using an ESP32 microcontroller, featuring an asynchronous web server interface and real-time temperature monitoring capabilities.

## Features

- **Asynchronous Web Server**: Built using ESPAsyncWebServer for efficient handling of web requests
- **KNX Integration**: Direct communication with KNX networks for sending temperature data
- **Real-time Monitoring**: Periodic temperature updates to KNX network
- **Web Interface**: Simple web interface for monitoring and control
- **Debug Capabilities**: Comprehensive logging and debugging features

## Hardware Requirements

- ESP32 Development Board (tested on NodeMCU-32S)
- KNX Network Interface
- Power Supply

## Software Dependencies

- ESPAsyncWebServer
- AsyncTCP
- esp-knx-ip
- WiFi

## Installation

1. Clone this repository
2. Install PlatformIO (recommended) or use Arduino IDE
3. Install required libraries:
   ```
   pio lib install "me-no-dev/ESPAsyncWebServer"
   pio lib install "me-no-dev/AsyncTCP"
   ```

## Configuration

1. Set your WiFi credentials in `main.cpp`:
   ```cpp
   WiFi.begin("your_ssid", "your_password");
   ```

2. Configure KNX settings:
   ```cpp
   knx.physical_address_set(knx.PA_to_address(1, 1, 160));  // Set your physical address
   tempAddress = knx.GA_to_address(10, 6, 5);  // Set your group address
   ```

## Usage

1. Upload the code to your ESP32
2. Monitor the serial output for connection details
3. Access the web interface at `http://<esp32-ip-address>`

### API Endpoints

#### Core Endpoints

- **GET `/`**
  - Description: Main dashboard interface
  - Response: HTML page with thermostat dashboard

- **GET `/test`**
  - Description: Test endpoint for server verification
  - Response: Plain text "Test endpoint working"

- **GET `/ping`**
  - Description: Server health check endpoint
  - Response: Plain text "pong"

- **GET `/servertest`**
  - Description: Server functionality verification with JSON response
  - Response: JSON `{"status": "ok", "message": "Server is functioning correctly"}`

#### Firmware Update Endpoints

- **GET `/update`**
  - Description: Firmware update interface
  - Response: HTML page with firmware upload form

- **POST `/doUpdate`**
  - Description: Handles firmware update process
  - Request: Multipart form data with firmware file
  - Response: Update status message

#### Static File Serving

- **GET `/*`**
  - Description: Serves static files from SPIFFS filesystem
  - Response: Requested file if exists

#### KNX Interface

- **GET `/knx`**
  - Description: KNX network interface and configuration
  - Response: KNX status and configuration interface

## Features in Detail

### Temperature Monitoring
- Sends temperature readings (default: 24°C) to KNX address 10/6/5
- Configurable update interval (default: 60 seconds)

### Web Interface
- Real-time temperature display
- Request counter
- Navigation links to all available endpoints

### Debugging
- Verbose logging for AsyncTCP and WebServer
- KNX communication debugging
- IP validation and connection status monitoring

## Current Status

The system is currently operational with:
- Asynchronous web server running on port 80
- KNX temperature transmission working correctly
- Web interface accessible and functional
- Enhanced debugging and monitoring capabilities

## Known Issues

- Web server accessibility might require explicit port forwarding or firewall rules
- Temperature value is currently hardcoded (24°C)

## Future Improvements

- Add temperature sensor integration
- Implement KNX value reception
- Add configuration interface
- Enhance web UI with real-time updates
- Add secure communication options

## Contributing

Contributions are welcome! Please read the API documentation in `API.md` for detailed information about the available functions and their usage.