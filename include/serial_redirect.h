#ifndef SERIAL_REDIRECT_H
#define SERIAL_REDIRECT_H

/**
 * @file serial_redirect.h
 * @brief Redirects Serial to CapturedSerial for web monitor capture
 *
 * This header MUST be included early in every .cpp file that uses Serial output
 * It redefines Serial to use CapturedSerial, which duplicates output to both
 * hardware serial and the web monitor
 */

#include "serial_capture_config.h"  // Gets real Serial pointer before redefinition

// Forward declarations
class TeeSerial;
extern TeeSerial CapturedSerial;

void initSerialCapture();

// THE CRITICAL REDEFINITION
// This redirects ALL Serial.print() calls to CapturedSerial
#undef Serial
#define Serial CapturedSerial

#endif // SERIAL_REDIRECT_H
