---
description: Monitor serial output from ESP32 for 30 seconds
---

Capture and display serial output from the ESP32 thermostat for 30 seconds at 115200 baud.

Steps:
1. Verify ESP32 is connected at /dev/cu.usbserial-0001
2. Start serial monitor using PlatformIO
3. Capture output for 30 seconds
4. Display the captured logs
5. Analyze any errors, warnings, or important information in the output

Look for:
- WiFi connection status
- KNX initialization and messages
- MQTT connection and messages
- Sensor readings (temperature, humidity, pressure)
- PID controller updates
- Watchdog resets
- Any error messages or warnings
- Reboot reasons

If monitoring fails, suggest:
- Check ESP32 is connected to USB
- Verify correct port in platformio.ini
- Ensure no other program is using the serial port
- Try unplugging and replugging the USB cable
