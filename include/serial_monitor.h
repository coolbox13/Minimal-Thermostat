#ifndef SERIAL_MONITOR_H
#define SERIAL_MONITOR_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <vector>

/**
 * @brief Serial Monitor for web-based serial console
 *
 * Captures ALL Serial output to a circular buffer and streams it via WebSocket
 * Uses hybrid approach: TeeSerial for Arduino + esp_log_set_vprintf for ESP-IDF
 */
class SerialMonitor {
public:
    static SerialMonitor& getInstance() {
        static SerialMonitor instance;
        return instance;
    }

    /**
     * @brief Initialize the serial monitor with WebSocket endpoint
     * @param server AsyncWebServer instance
     */
    void begin(AsyncWebServer* server);

    /**
     * @brief Add a line to the buffer and broadcast to clients
     * @param line Line to add (raw, no formatting)
     */
    void println(const String& line);

    /**
     * @brief Add raw data (for character-by-character capture)
     * @param data Data buffer
     * @param len Length of data
     */
    void write(const char* data, size_t len);

    /**
     * @brief Get number of connected clients
     */
    size_t getClientCount() const { return _ws ? _ws->count() : 0; }

    /**
     * @brief Clean up disconnected WebSocket clients
     * Should be called periodically from loop()
     */
    void cleanupClients() {
        if (_ws) {
            _ws->cleanupClients();
        }
    }

private:
    SerialMonitor() : _server(nullptr), _ws(nullptr), _lineBuffer("") {}
    ~SerialMonitor() = default;

    // Prevent copying
    SerialMonitor(const SerialMonitor&) = delete;
    SerialMonitor& operator=(const SerialMonitor&) = delete;

    /**
     * @brief WebSocket event handler
     */
    void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                          AwsEventType type, void* arg, uint8_t* data, size_t len);

    /**
     * @brief Send buffer to newly connected client
     */
    void sendBufferToClient(AsyncWebSocketClient* client);

    /**
     * @brief Process incoming character and handle line buffering
     */
    void processChar(char c);

    AsyncWebServer* _server;
    AsyncWebSocket* _ws;
    std::vector<String> _buffer;
    String _lineBuffer;

    static const size_t MAX_BUFFER_SIZE = 20;    // Last 20 lines (prevent WebSocket queue overflow)
    static const size_t MAX_LINE_LENGTH = 512;   // Max line length
};

/**
 * @brief TeeSerial - Duplicates all Serial output to both hardware serial and web monitor
 *
 * This class intercepts Serial.print() calls and forwards them to:
 * 1. The real hardware Serial (for USB monitoring)
 * 2. The SerialMonitor (for web interface)
 */
class TeeSerial : public Print {
public:
    TeeSerial() : _hwSerial(nullptr), _lineBuffer("") {}

    /**
     * @brief Initialize with hardware serial reference
     * Must be called AFTER Serial.begin()
     */
    void begin(HardwareSerial* hwSerial) {
        _hwSerial = hwSerial;
    }

    size_t write(uint8_t c) override {
        // Write to hardware serial first
        size_t result = 0;
        if (_hwSerial) {
            result = _hwSerial->write(c);
        }

        // Capture for web monitor - ALWAYS capture, even if hwSerial is null
        if (c == '\n') {
            // Line complete, send to monitor
            if (_lineBuffer.length() > 0) {
                SerialMonitor::getInstance().println(_lineBuffer);
            }
            _lineBuffer = "";
        } else if (c != '\r') {  // Ignore carriage returns
            _lineBuffer += (char)c;

            // Prevent buffer overflow
            if (_lineBuffer.length() > 512) {
                SerialMonitor::getInstance().println(_lineBuffer);
                _lineBuffer = "";
            }
        }

        return result;
    }

    size_t write(const uint8_t* buffer, size_t size) override {
        // Write to hardware serial first
        size_t result = 0;
        if (_hwSerial) {
            result = _hwSerial->write(buffer, size);
        }

        // Capture for web monitor
        for (size_t i = 0; i < size; i++) {
            char c = buffer[i];
            if (c == '\n') {
                SerialMonitor::getInstance().println(_lineBuffer);
                _lineBuffer = "";
            } else if (c != '\r') {
                _lineBuffer += c;
                if (_lineBuffer.length() > 512) {
                    SerialMonitor::getInstance().println(_lineBuffer);
                    _lineBuffer = "";
                }
            }
        }

        return result;
    }

    // Forward other Serial methods
    void begin(unsigned long baud) { if (_hwSerial) _hwSerial->begin(baud); }
    void end() { if (_hwSerial) _hwSerial->end(); }
    int available() { return _hwSerial ? _hwSerial->available() : 0; }
    int read() { return _hwSerial ? _hwSerial->read() : -1; }
    int peek() { return _hwSerial ? _hwSerial->peek() : -1; }
    void flush() { if (_hwSerial) _hwSerial->flush(); }

private:
    HardwareSerial* _hwSerial;
    String _lineBuffer;
};

// Global TeeSerial instance
extern TeeSerial CapturedSerial;

#endif // SERIAL_MONITOR_H
