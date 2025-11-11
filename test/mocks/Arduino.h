#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>

// Arduino types
typedef std::string String;
typedef bool boolean;
typedef uint8_t byte;

// Arduino constants
#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

// Math constants
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// Mock time functions
extern unsigned long _mock_millis;
extern unsigned long _mock_micros;

inline unsigned long millis() { return _mock_millis; }
inline unsigned long micros() { return _mock_micros; }
inline void delay(unsigned long ms) { _mock_millis += ms; }
inline void delayMicroseconds(unsigned int us) { _mock_micros += us; }

// Math functions
inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline long constrain(long x, long a, long b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline double constrain(double x, double a, double b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

// Utility functions
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline int digitalRead(uint8_t pin) { return LOW; }

// Serial mock
class SerialMock {
public:
    void begin(unsigned long baud) {}
    void print(const char* str) {}
    void print(int val) {}
    void print(float val) {}
    void println(const char* str) {}
    void println(int val) {}
    void println(float val) {}
    void println() {}
    template<typename T> void print(T val) {}
    template<typename T> void println(T val) {}
};

extern SerialMock Serial;

#endif // MOCK_ARDUINO_H
