#ifndef MOCK_WEBSERVER_H
#define MOCK_WEBSERVER_H

#include "Arduino.h"
#include <functional>
#include <vector>
#include <map>

// HTTP methods
typedef enum {
    HTTP_GET     = 0,
    HTTP_POST    = 1,
    HTTP_PUT     = 2,
    HTTP_PATCH   = 3,
    HTTP_DELETE  = 4,
    HTTP_OPTIONS = 5
} HTTPMethod;

/**
 * Mock WebServer class for testing
 * Simulates AsyncWebServer functionality
 */
class AsyncWebServer {
private:
    uint16_t _port;
    bool _started;

    struct Route {
        String path;
        HTTPMethod method;
        std::function<void()> handler;
    };

    std::vector<Route> _routes;
    std::map<String, String> _headers;
    std::map<String, String> _args;
    String _responseBody;
    int _responseCode;

public:
    AsyncWebServer(uint16_t port)
        : _port(port)
        , _started(false)
        , _responseCode(200) {}

    /**
     * Start the web server
     */
    void begin() {
        _started = true;
    }

    /**
     * Register GET handler
     */
    void on(const char* uri, std::function<void()> handler) {
        Route route;
        route.path = uri;
        route.method = HTTP_GET;
        route.handler = handler;
        _routes.push_back(route);
    }

    /**
     * Register handler with specific method
     */
    void on(const char* uri, HTTPMethod method, std::function<void()> handler) {
        Route route;
        route.path = uri;
        route.method = method;
        route.handler = handler;
        _routes.push_back(route);
    }

    /**
     * Send response
     */
    void send(int code, const char* content_type, const String& content) {
        _responseCode = code;
        _responseBody = content;
    }

    void send(int code, const char* content_type, const char* content) {
        _responseCode = code;
        _responseBody = String(content);
    }

    void send(int code) {
        _responseCode = code;
        _responseBody = "";
    }

    /**
     * Get request argument
     */
    String arg(const char* name) {
        if (_args.count(name)) {
            return _args[name];
        }
        return "";
    }

    /**
     * Check if argument exists
     */
    bool hasArg(const char* name) {
        return _args.count(name) > 0;
    }

    /**
     * Get number of arguments
     */
    int args() {
        return _args.size();
    }

    /**
     * Get header value
     */
    String header(const char* name) {
        if (_headers.count(name)) {
            return _headers[name];
        }
        return "";
    }

    /**
     * Check if header exists
     */
    bool hasHeader(const char* name) {
        return _headers.count(name) > 0;
    }

    // ===== Test Control Methods =====

    /**
     * Simulate a request to trigger handler
     */
    void simulateRequest(const char* uri, HTTPMethod method = HTTP_GET) {
        for (const auto& route : _routes) {
            if (route.path == uri && route.method == method) {
                if (route.handler) {
                    route.handler();
                }
                break;
            }
        }
    }

    /**
     * Set mock request arguments
     */
    void setMockArg(const char* name, const char* value) {
        _args[name] = value;
    }

    /**
     * Set mock request header
     */
    void setMockHeader(const char* name, const char* value) {
        _headers[name] = value;
    }

    /**
     * Get response body for testing
     */
    String getMockResponseBody() {
        return _responseBody;
    }

    /**
     * Get response code for testing
     */
    int getMockResponseCode() {
        return _responseCode;
    }

    /**
     * Check if server is started
     */
    bool isStarted() {
        return _started;
    }

    /**
     * Reset mock state
     */
    void resetMock() {
        _started = false;
        _routes.clear();
        _headers.clear();
        _args.clear();
        _responseBody = "";
        _responseCode = 200;
    }
};

// Alias for compatibility
typedef AsyncWebServer WebServer;

#endif // MOCK_WEBSERVER_H
