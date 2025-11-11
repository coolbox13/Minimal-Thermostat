#include "Arduino.h"

// Global mock time variables
unsigned long _mock_millis = 0;
unsigned long _mock_micros = 0;

// Global Serial instance
SerialMock Serial;

// Reset function for tests
void resetArduinoMocks() {
    _mock_millis = 0;
    _mock_micros = 0;
}
