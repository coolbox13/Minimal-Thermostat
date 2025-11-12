#ifndef SERIAL_MONITOR_H
#define SERIAL_MONITOR_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <vector>

/**
 * @brief Serial Monitor for web-based serial console
 *
 * Captures Serial output to a circular buffer and streams it via WebSocket
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
     * @param line Line to add
     */
    void println(const String& line);

    /**
     * @brief Get number of connected clients
     */
    size_t getClientCount() const { return _clients.size(); }

private:
    SerialMonitor() : _server(nullptr), _ws(nullptr) {}
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

    AsyncWebServer* _server;
    AsyncWebSocket* _ws;
    std::vector<String> _buffer;
    std::vector<AsyncWebSocketClient*> _clients;

    static const size_t MAX_BUFFER_SIZE = 200;  // Last 200 lines
};

/**
 * @brief Custom Print class that captures to SerialMonitor
 */
class SerialCapture : public Print {
public:
    SerialCapture() : _lineBuffer("") {}

    size_t write(uint8_t c) override {
        // Write to real Serial
        Serial.write(c);

        // Capture for web monitor
        if (c == '\n') {
            // Line complete, send to monitor
            SerialMonitor::getInstance().println(_lineBuffer);
            _lineBuffer = "";
        } else if (c != '\r') {  // Ignore carriage returns
            _lineBuffer += (char)c;

            // Prevent buffer overflow
            if (_lineBuffer.length() > 512) {
                SerialMonitor::getInstance().println(_lineBuffer);
                _lineBuffer = "";
            }
        }

        return 1;
    }

    size_t write(const uint8_t* buffer, size_t size) override {
        for (size_t i = 0; i < size; i++) {
            write(buffer[i]);
        }
        return size;
    }

private:
    String _lineBuffer;
};

#endif // SERIAL_MONITOR_H
