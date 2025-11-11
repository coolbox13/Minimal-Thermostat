#ifndef MOCK_WIFI_H
#define MOCK_WIFI_H

#include "Arduino.h"

// WiFi status codes
typedef enum {
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6
} wl_status_t;

// WiFi modes
typedef enum {
    WIFI_OFF = 0,
    WIFI_STA = 1,
    WIFI_AP = 2,
    WIFI_AP_STA = 3
} wifi_mode_t;

/**
 * Mock IP address class
 */
class IPAddress {
private:
    uint8_t _address[4];

public:
    IPAddress() {
        _address[0] = 0;
        _address[1] = 0;
        _address[2] = 0;
        _address[3] = 0;
    }

    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        _address[0] = a;
        _address[1] = b;
        _address[2] = c;
        _address[3] = d;
    }

    String toString() const {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d.%d.%d.%d",
                 _address[0], _address[1], _address[2], _address[3]);
        return String(buf);
    }

    uint8_t operator[](int index) const {
        return _address[index];
    }

    uint8_t& operator[](int index) {
        return _address[index];
    }

    operator bool() const {
        return _address[0] != 0 || _address[1] != 0 ||
               _address[2] != 0 || _address[3] != 0;
    }
};

/**
 * Mock WiFi client class
 */
class WiFiClient {
public:
    WiFiClient() {}
    virtual ~WiFiClient() {}

    bool connect(const char* host, uint16_t port) { return true; }
    bool connected() { return true; }
    void stop() {}

    size_t write(uint8_t data) { return 1; }
    size_t write(const uint8_t *buf, size_t size) { return size; }

    int available() { return 0; }
    int read() { return -1; }
    int peek() { return -1; }
};

/**
 * Mock WiFi class for testing
 * Simulates ESP32 WiFi functionality without actual hardware
 */
class WiFiClass {
private:
    wl_status_t _status;
    String _ssid;
    String _password;
    IPAddress _localIP;
    IPAddress _gatewayIP;
    IPAddress _subnetMask;
    IPAddress _dnsIP1;
    int _rssi;

public:
    WiFiClass()
        : _status(WL_DISCONNECTED)
        , _ssid("")
        , _password("")
        , _localIP(0, 0, 0, 0)
        , _gatewayIP(0, 0, 0, 0)
        , _subnetMask(0, 0, 0, 0)
        , _dnsIP1(0, 0, 0, 0)
        , _rssi(-70) {}

    /**
     * Start WiFi connection
     */
    void begin(const char* ssid, const char* password) {
        _ssid = ssid;
        _password = password;
        _status = WL_CONNECTED;
        _localIP = IPAddress(192, 168, 1, 100);
        _gatewayIP = IPAddress(192, 168, 1, 1);
        _subnetMask = IPAddress(255, 255, 255, 0);
        _dnsIP1 = IPAddress(8, 8, 8, 8);
    }

    /**
     * Disconnect from WiFi
     */
    void disconnect() {
        _status = WL_DISCONNECTED;
        _localIP = IPAddress(0, 0, 0, 0);
    }

    /**
     * Get connection status
     */
    wl_status_t status() {
        return _status;
    }

    /**
     * Get local IP address
     */
    IPAddress localIP() {
        return _localIP;
    }

    /**
     * Get gateway IP address
     */
    IPAddress gatewayIP() {
        return _gatewayIP;
    }

    /**
     * Get subnet mask
     */
    IPAddress subnetMask() {
        return _subnetMask;
    }

    /**
     * Get DNS IP address
     */
    IPAddress dnsIP(uint8_t index = 0) {
        return _dnsIP1;
    }

    /**
     * Get SSID of connected network
     */
    String SSID() {
        return _ssid;
    }

    /**
     * Get password of connected network
     */
    String psk() {
        return _password;
    }

    /**
     * Get signal strength (RSSI)
     */
    int32_t RSSI() {
        return _rssi;
    }

    /**
     * Set WiFi mode
     */
    bool mode(wifi_mode_t m) {
        return true;
    }

    // ===== Test Control Methods =====

    /**
     * Set mock connection status
     */
    void setMockStatus(wl_status_t status) {
        _status = status;
        if (status != WL_CONNECTED) {
            _localIP = IPAddress(0, 0, 0, 0);
        }
    }

    /**
     * Set mock RSSI value
     */
    void setMockRSSI(int rssi) {
        _rssi = rssi;
    }

    /**
     * Set mock IP address
     */
    void setMockLocalIP(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        _localIP = IPAddress(a, b, c, d);
    }

    /**
     * Reset mock to default state
     */
    void resetMock() {
        _status = WL_DISCONNECTED;
        _ssid = "";
        _password = "";
        _localIP = IPAddress(0, 0, 0, 0);
        _gatewayIP = IPAddress(0, 0, 0, 0);
        _subnetMask = IPAddress(0, 0, 0, 0);
        _dnsIP1 = IPAddress(0, 0, 0, 0);
        _rssi = -70;
    }
};

extern WiFiClass WiFi;

#endif // MOCK_WIFI_H
