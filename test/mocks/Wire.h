#ifndef MOCK_WIRE_H
#define MOCK_WIRE_H

#include <stdint.h>

/**
 * Mock implementation of Arduino Wire (I2C) library for testing
 */
class TwoWire {
public:
    void begin() {}
    void begin(int sda, int sdl) {}
    void setClock(uint32_t frequency) {}

    void beginTransmission(uint8_t address) {}
    uint8_t endTransmission() { return 0; }

    uint8_t requestFrom(uint8_t address, uint8_t quantity) { return quantity; }

    size_t write(uint8_t data) { return 1; }
    size_t write(const uint8_t *data, size_t quantity) { return quantity; }

    int available() { return 0; }
    int read() { return 0; }
    int peek() { return -1; }
};

extern TwoWire Wire;

#endif // MOCK_WIRE_H
