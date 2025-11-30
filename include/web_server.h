/**
 * @file web_server.h
 * @brief Async web server manager for ESP32 KNX Thermostat
 *
 * Provides REST API endpoints, static file serving, and real-time WebSocket
 * communication for the thermostat web interface. Uses ESPAsyncWebServer
 * for non-blocking operation.
 *
 * @note Uses AsyncJsonResponse for memory-efficient large JSON responses
 * @see https://github.com/ESP32Async/ESPAsyncWebServer
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <functional>
#include "config.h"
#include "config_manager.h"

// Forward declaration of embedded HTML
extern const char* THERMOSTAT_HTML;

/**
 * @class WebServerManager
 * @brief Singleton manager for the async web server
 *
 * Handles all HTTP requests including:
 * - Static file serving from LittleFS (with gzip support)
 * - REST API endpoints for sensor data, configuration, and control
 * - SPA routing for the React frontend
 * - WebSocket endpoint for serial monitor streaming
 *
 * @par Memory Management
 * The history endpoint uses AsyncJsonResponse with a 16KB buffer to avoid
 * double-buffering issues that can cause heap fragmentation on ESP32.
 *
 * @par Thread Safety
 * Request handlers run in the async TCP task context. Avoid blocking
 * operations and use appropriate synchronization for shared state.
 */
class WebServerManager {
public:
    /** @brief Callback type for KNX address change notifications */
    typedef std::function<void()> KnxAddressChangedCallback;

    /**
     * @brief Get the singleton instance
     * @return Pointer to the WebServerManager instance
     */
    static WebServerManager* getInstance();

    /**
     * @brief Initialize the web server
     * @param server Pointer to the AsyncWebServer instance to manage
     *
     * Mounts LittleFS filesystem and sets up all routes and handlers.
     * Must be called after WiFi is connected.
     */
    void begin(AsyncWebServer* server);

    /**
     * @brief Add a custom endpoint handler
     * @param uri The URI path to handle
     * @param method HTTP method(s) to accept
     * @param handler The request handler function
     */
    void addEndpoint(const char* uri, WebRequestMethodComposite method,
                     ArRequestHandlerFunction handler);

    /**
     * @brief Add a custom endpoint with upload handler
     * @param uri The URI path to handle
     * @param method HTTP method(s) to accept
     * @param onRequest Handler for the request completion
     * @param onUpload Handler for file upload chunks
     */
    void addEndpoint(const char* uri, WebRequestMethodComposite method,
                     ArRequestHandlerFunction onRequest,
                     ArUploadHandlerFunction onUpload);

    /**
     * @brief Set up all default API routes and static file handlers
     *
     * Configures routes for:
     * - / and SPA routes (/config, /status, /logs, /serial)
     * - /api/* REST endpoints
     * - /assets/* and /js/* static files
     */
    void setupDefaultRoutes();

    /**
     * @brief Get the underlying AsyncWebServer
     * @return Pointer to the managed AsyncWebServer
     */
    AsyncWebServer* getServer() { return _server; }

    /**
     * @brief Register callback for KNX address configuration changes
     * @param callback Function to call when KNX addresses change
     *
     * Used to notify KNXManager to reload addresses after config update.
     */
    void setKnxAddressChangedCallback(KnxAddressChangedCallback callback);

private:
    WebServerManager();
    static WebServerManager* _instance;
    AsyncWebServer* _server;

    KnxAddressChangedCallback _knxAddressChangedCallback = nullptr;

    // Default route handlers
    static void handleRoot(AsyncWebServerRequest *request);
    static void handleTest(AsyncWebServerRequest *request);
    static void handlePing(AsyncWebServerRequest *request);

    /**
     * @brief Handle KNX address toggle in config update
     * @param jsonDoc The received configuration JSON
     * @param oldUseTestSetting Previous test/production address setting
     */
    void handleKNXAddressChange(const JsonDocument& jsonDoc, bool oldUseTestSetting);

    /**
     * @brief Apply PID parameter changes from config update
     * @param jsonDoc The received configuration JSON
     */
    void handlePIDParameterUpdates(const JsonDocument& jsonDoc);

    /**
     * @brief Apply NTP settings changes and resync time
     * @param jsonDoc The received configuration JSON
     */
    void handleNTPUpdate(const JsonDocument& jsonDoc);
};

#endif // WEB_SERVER_H