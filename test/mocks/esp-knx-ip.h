#ifndef MOCK_ESP_KNX_IP_H
#define MOCK_ESP_KNX_IP_H

#include "Arduino.h"

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
typedef void (*knx_callback_t)(uint8_t*, uint8_t);

/**
 * Simple map entry for callbacks and values
 */
struct CallbackEntry {
    address_t addr;
    knx_callback_t callback;
    bool used;
};

struct ValueEntry {
    address_t addr;
    uint8_t value;
    bool used;
};

/**
 * Mock ESP-KNX-IP class for testing
 * Simulates KNX bus communication without actual hardware
 */
class ESPKNXIP {
private:
    static const int MAX_CALLBACKS = 10;
    static const int MAX_VALUES = 20;

    address_t _physicalAddress;
    bool _started;
    CallbackEntry _callbacks[MAX_CALLBACKS];
    ValueEntry _groupAddressValues[MAX_VALUES];

public:
    ESPKNXIP()
        : _started(false) {
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            _callbacks[i].used = false;
        }
        for (int i = 0; i < MAX_VALUES; i++) {
            _groupAddressValues[i].used = false;
        }
    }

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
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            if (!_callbacks[i].used) {
                _callbacks[i].addr = addr;
                _callbacks[i].callback = callback;
                _callbacks[i].used = true;
                return;
            }
        }
    }

    /**
     * Send 1-byte value to group address
     */
    void write_1byte_int(address_t addr, uint8_t value) {
        if (!_started) return;
        setGroupAddressValue(addr, value);
    }

    /**
     * Send 2-byte float to group address (DPT 9.xxx)
     */
    void write_2byte_float(address_t addr, float value) {
        if (!_started) return;
        // Store as scaled integer for simplicity in mock
        setGroupAddressValue(addr, (uint8_t)(value * 10.0f));
    }

    /**
     * Send 2-byte int to group address
     */
    void write_2byte_int(address_t addr, int16_t value) {
        if (!_started) return;
        setGroupAddressValue(addr, (uint8_t)value);
    }

    /**
     * Send 4-byte float to group address
     */
    void write_4byte_float(address_t addr, float value) {
        if (!_started) return;
        setGroupAddressValue(addr, (uint8_t)value);
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
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            if (_callbacks[i].used && _callbacks[i].addr == addr) {
                _callbacks[i].callback(data, len);
                return;
            }
        }
    }

    /**
     * Get value sent to a group address
     */
    uint8_t getMockGroupAddressValue(address_t addr) {
        for (int i = 0; i < MAX_VALUES; i++) {
            if (_groupAddressValues[i].used && _groupAddressValues[i].addr == addr) {
                return _groupAddressValues[i].value;
            }
        }
        return 0;
    }

    /**
     * Check if group address was written to
     */
    bool wasMockGroupAddressWritten(address_t addr) {
        for (int i = 0; i < MAX_VALUES; i++) {
            if (_groupAddressValues[i].used && _groupAddressValues[i].addr == addr) {
                return true;
            }
        }
        return false;
    }

    /**
     * Check if callback is registered
     */
    bool isMockCallbackRegistered(address_t addr) {
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            if (_callbacks[i].used && _callbacks[i].addr == addr) {
                return true;
            }
        }
        return false;
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
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            _callbacks[i].used = false;
        }
        for (int i = 0; i < MAX_VALUES; i++) {
            _groupAddressValues[i].used = false;
        }
    }

private:
    /**
     * Helper to set or update group address value
     */
    void setGroupAddressValue(address_t addr, uint8_t value) {
        // Check if address already exists
        for (int i = 0; i < MAX_VALUES; i++) {
            if (_groupAddressValues[i].used && _groupAddressValues[i].addr == addr) {
                _groupAddressValues[i].value = value;
                return;
            }
        }
        // Add new address
        for (int i = 0; i < MAX_VALUES; i++) {
            if (!_groupAddressValues[i].used) {
                _groupAddressValues[i].addr = addr;
                _groupAddressValues[i].value = value;
                _groupAddressValues[i].used = true;
                return;
            }
        }
    }
};

// Global instance
extern ESPKNXIP knx;

#endif // MOCK_ESP_KNX_IP_H
