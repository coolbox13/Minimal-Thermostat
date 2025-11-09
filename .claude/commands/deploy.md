---
description: Build and deploy firmware to ESP32 via PlatformIO
---

Build and upload the current branch to the ESP32 board connected at /dev/cu.usbserial-0001.

Steps:
1. Check git status to show current branch
2. Verify ESP32 is connected on the USB port
3. Build and upload firmware using PlatformIO
4. Show upload results including flash/RAM usage

If the upload fails, suggest checking:
- ESP32 is connected to USB
- Correct port in platformio.ini
- No other program is using the serial port
