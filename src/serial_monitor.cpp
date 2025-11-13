#include "serial_monitor.h"

// Helper function for logger integration
void sendToSerialMonitor(const char* line) {
    SerialMonitor::getInstance().println(String(line));
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

    Serial.println("[SERIAL_MON] Serial monitor WebSocket initialized");
}

void SerialMonitor::println(const String& line) {
    // Add to buffer
    _buffer.push_back(line);

    // Maintain buffer size
    if (_buffer.size() > MAX_BUFFER_SIZE) {
        _buffer.erase(_buffer.begin());
    }

    // Broadcast to all connected clients
    if (_ws && _ws->count() > 0) {
        _ws->textAll(line);
    }
}

void SerialMonitor::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("[SERIAL_MON] WebSocket client #%u connected from %s\n",
                         client->id(), client->remoteIP().toString().c_str());

            // Send buffer history to new client
            sendBufferToClient(client);

            // Add to client list
            _clients.push_back(client);
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("[SERIAL_MON] WebSocket client #%u disconnected\n", client->id());

            // Remove from client list
            _clients.erase(
                std::remove(_clients.begin(), _clients.end(), client),
                _clients.end()
            );
            break;

        case WS_EVT_ERROR:
            Serial.printf("[SERIAL_MON] WebSocket error from client #%u\n", client->id());
            break;

        case WS_EVT_DATA:
            // Optional: Handle commands from client (future feature)
            break;

        case WS_EVT_PONG:
            // Pong received
            break;
    }
}

void SerialMonitor::sendBufferToClient(AsyncWebSocketClient* client) {
    // Send all buffered lines to new client
    for (const String& line : _buffer) {
        client->text(line);
        delay(1);  // Small delay to prevent overwhelming client
    }
}
