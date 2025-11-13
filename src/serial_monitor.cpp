// Step 1: Include config to save real Serial
#include "serial_capture_config.h"

// Step 2: Include our headers
#include "serial_monitor.h"
#include "serial_redirect.h"
#include <esp_log.h>

// Now use the saved pointer from SerialCapture namespace
static HardwareSerial* _realSerial = SerialCapture::RealSerial;
HardwareSerial* _realSerialForLogger = SerialCapture::RealSerial;

// Global TeeSerial instance
TeeSerial CapturedSerial;

// Initialize serial capture - called from main.cpp setup()
void initSerialCapture() {
    // Start hardware serial first
    _realSerial->begin(115200);

    // Initialize TeeSerial with hardware serial reference
    CapturedSerial.begin(_realSerial);

    // TEST: Directly call TeeSerial to verify it works
    CapturedSerial.println("*** DIRECT TEST: TeeSerial is working ***");
    // This should appear on both hardware serial AND web monitor (once WebSocket connects)
}

// Custom vprintf for ESP-IDF log redirection
static int custom_vprintf(const char *fmt, va_list args) {
    char buf[256];
    int len = vsnprintf(buf, sizeof(buf), fmt, args);

    // Send to real serial
    _realSerial->write((const uint8_t*)buf, len);

    // Also capture it for web monitor
    SerialMonitor::getInstance().write(buf, len);

    return len;
}

void SerialMonitor::begin(AsyncWebServer* server) {
    _server = server;

    // Create WebSocket endpoint
    _ws = new AsyncWebSocket("/ws/serial");

    // Set event handler
    _ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                        AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onWebSocketEvent(server, client, type, arg, data, len);
    });

    // Add to server
    _server->addHandler(_ws);

    // Redirect ESP-IDF logs to our custom handler
    esp_log_set_vprintf(custom_vprintf);

    // Don't log this message through Serial (would cause recursion)
    // Instead, directly write to hardware serial
    _realSerial->println("[SERIAL_MON] Web serial monitor initialized");
}

void SerialMonitor::println(const String& line) {
    // ALWAYS add to buffer (even if _ws doesn't exist yet)
    _buffer.push_back(line);
    if (_buffer.size() > MAX_BUFFER_SIZE) {
        _buffer.erase(_buffer.begin());
    }

    // Only broadcast if WebSocket exists AND has clients
    if (_ws && _ws->count() > 0) {
        _ws->textAll(line);
    }
    // NO debug output here - causes log spam
}

void SerialMonitor::write(const char* data, size_t len) {
    // Process character by character for line buffering
    for (size_t i = 0; i < len; i++) {
        processChar(data[i]);
    }
}

void SerialMonitor::processChar(char c) {
    if (c == '\n') {
        // Line complete, send to clients
        if (_lineBuffer.length() > 0) {
            println(_lineBuffer);
            _lineBuffer = "";
        }
    } else if (c != '\r') {  // Ignore carriage returns
        _lineBuffer += c;

        // Prevent buffer overflow
        if (_lineBuffer.length() > MAX_LINE_LENGTH) {
            println(_lineBuffer);
            _lineBuffer = "";
        }
    }
}

void SerialMonitor::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            _realSerial->printf("[SERIAL_MON] Client #%u connected from %s\n",
                         client->id(), client->remoteIP().toString().c_str());

            sendBufferToClient(client);  // Send welcome + buffer history
            break;

        case WS_EVT_DISCONNECT:
            _realSerial->printf("[SERIAL_MON] Client #%u disconnected\n", client->id());
            // No need to remove from _clients
            break;

        case WS_EVT_ERROR:
            _realSerial->printf("[SERIAL_MON] WebSocket error from client #%u\n", client->id());
            break;

        case WS_EVT_DATA:
            // Optional: Handle commands from client
            break;

        case WS_EVT_PONG:
            break;
    }
}

void SerialMonitor::sendBufferToClient(AsyncWebSocketClient* client) {
    // Send welcome
    client->text("=== Serial Monitor Connected ===");
    delay(10);

    // Send buffer history with throttling
    for (size_t i = 0; i < _buffer.size(); i++) {
        client->text(_buffer[i]);
        if (i % 10 == 0) delay(5);  // Small delay every 10 messages
    }

    _realSerial->printf("[SERIAL_MON] Sent %d lines to client\n", _buffer.size());
}

// Helper function for Logger to send to web monitor
// This avoids circular dependency between logger.h and serial_monitor.h
void captureLogToWebMonitor(const char* msg) {
    SerialMonitor::getInstance().println(String(msg));
}
