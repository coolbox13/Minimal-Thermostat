#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <vector>
#include "config_manager.h"
#include "../config/wifi_manager_fix.h"

class WebAuthManager {
public:
    WebAuthManager(AsyncWebServer& server, ConfigManager& configManager);
    
    // Authentication methods
    bool authenticate(AsyncWebServerRequest* request);
    void requestAuthentication(AsyncWebServerRequest* request);
    bool validateSession(const String& sessionId);
    String createSession();
    void removeSession(const String& sessionId);
    
    // Security methods
    bool validateCSRFToken(AsyncWebServerRequest *request, const String& token);
    String generateCSRFToken(AsyncWebServerRequest *request);
    bool checkRateLimit(const String& ip);
    void addSecurityHeaders(AsyncWebServerResponse *response);
    
    void setCredentials(const char* username, const char* password);

private:
    AsyncWebServer& server;
    ConfigManager& configManager;
    char username[32];
    char password[32];
    
    // Session management
    struct Session {
        String ip;
        unsigned long created;
        unsigned long lastAccess;
        String csrfToken;
    };
    std::map<String, Session> sessions;
    void cleanupSessions();
    
    // Rate limiting
    struct RateLimit {
        unsigned long firstAttempt;
        int attempts;
    };
    std::map<String, RateLimit> rateLimits;
    void cleanupRateLimits();
    
    // Constants
    static const unsigned long SESSION_TIMEOUT = 3600000; // 1 hour
    static const unsigned long RATE_LIMIT_WINDOW = 300000; // 5 minutes
    static const int MAX_ATTEMPTS = 5;
    
    // Helper methods
    String generateRandomString(size_t length);
    void purgeOldSessions();
}; 