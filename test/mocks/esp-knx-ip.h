#ifndef MOCK_ESP_KNX_IP_H
#define MOCK_ESP_KNX_IP_H

#include "Arduino.h"
#include <functional>
#include <map>
#include <string>

/**
 * Mock KNX address class
 */
class address_t {
private:
    uint8_t _ga[3];

public:
    address_t() {
        _ga[0] = 0;
        _ga[1] = 0;
        _ga[2] = 0;
    }

    address_t(uint8_t area, uint8_t line, uint8_t member) {
        _ga[0] = area;
        _ga[1] = line;
        _ga[2] = member;
    }

    String toString() const {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d/%d/%d", _ga[0], _ga[1], _ga[2]);
        return String(buf);
    }

    bool operator==(const address_t& other) const {
        return _ga[0] == other._ga[0] &&
               _ga[1] == other._ga[1] &&
               _ga[2] == other._ga[2];
    }

    bool operator<(const address_t& other) const {
        if (_ga[0] != other._ga[0]) return _ga[0] < other._ga[0];
        if (_ga[1] != other._ga[1]) return _ga[1] < other._ga[1];
        return _ga[2] < other._ga[2];
    }
};

/**
 * Mock callback registration structure
 */
typedef std::function<void(uint8_t*, uint8_t)> knx_callback_t;

/**
 * Mock ESP-KNX-IP class for testing
 * Simulates KNX bus communication without actual hardware
 */
class ESPKNXIP {
private:
    address_t _physicalAddress;
    bool _started;
    std::map<address_t, knx_callback_t> _callbacks;
    std::map<address_t, uint8_t> _groupAddressValues;

public:
    ESPKNXIP()
        : _started(false) {}

    /**
     * Start KNX communication
     */
    void start() {
        _started = true;
    }

    /**
     * Stop KNX communication
     */
    void stop() {
        _started = false;
    }

    /**
     * Set physical address
     */
    void physical_address_set(address_t addr) {
        _physicalAddress = addr;
    }

    /**
     * Register callback for group address
     */
    void callback_register(const char* name, address_t addr, knx_callback_t callback) {
        _callbacks[addr] = callback;
    }

    /**
     * Send 1-byte value to group address
     */
    void write_1byte_int(address_t addr, uint8_t value) {
        if (!_started) return;
        _groupAddressValues[addr] = value;
    }

    /**
     * Send 2-byte float to group address (DPT 9.xxx)
     */
    void write_2byte_float(address_t addr, float value) {
        if (!_started) return;
        // Store as scaled integer for simplicity in mock
        _groupAddressValues[addr] = (uint8_t)(value * 10.0f);
    }

    /**
     * Send 2-byte int to group address
     */
    void write_2byte_int(address_t addr, int16_t value) {
        if (!_started) return;
        _groupAddressValues[addr] = (uint8_t)value;
    }

    /**
     * Send 4-byte float to group address
     */
    void write_4byte_float(address_t addr, float value) {
        if (!_started) return;
        _groupAddressValues[addr] = (uint8_t)value;
    }

    /**
     * Process KNX messages (call in loop)
     */
    void loop() {
        // Mock does nothing in loop
    }

    // ===== Test Control Methods =====

    /**
     * Simulate receiving a KNX telegram
     */
    void simulateTelegram(address_t addr, uint8_t* data, uint8_t len) {
        if (_callbacks.count(addr)) {
            _callbacks[addr](data, len);
        }
    }

    /**
     * Get value sent to a group address
     */
    uint8_t getMockGroupAddressValue(address_t addr) {
        if (_groupAddressValues.count(addr)) {
            return _groupAddressValues[addr];
        }
        return 0;
    }

    /**
     * Check if group address was written to
     */
    bool wasMockGroupAddressWritten(address_t addr) {
        return _groupAddressValues.count(addr) > 0;
    }

    /**
     * Check if callback is registered
     */
    bool isMockCallbackRegistered(address_t addr) {
        return _callbacks.count(addr) > 0;
    }

    /**
     * Check if started
     */
    bool isStarted() {
        return _started;
    }

    /**
     * Reset mock state
     */
    void resetMock() {
        _started = false;
        _callbacks.clear();
        _groupAddressValues.clear();
    }
};

// Global instance
extern ESPKNXIP knx;

#endif // MOCK_ESP_KNX_IP_H
