#ifndef SERIAL_CAPTURE_CONFIG_H
#define SERIAL_CAPTURE_CONFIG_H

// This header must be included BEFORE serial_monitor.h
// It saves the REAL Serial pointer before any redefinition happens

#include <Arduino.h>

// Save the REAL Serial pointer BEFORE redefinition
// This needs to be accessible globally
namespace SerialCapture {
    static HardwareSerial* RealSerial = &Serial;
}

#endif // SERIAL_CAPTURE_CONFIG_H
