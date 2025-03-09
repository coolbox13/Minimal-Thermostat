#ifndef WEB_AUTH_MANAGER_H
#define WEB_AUTH_MANAGER_H

#include <Arduino.h>
#include <map>
#include <vector>

class ConfigManager;
class WebServerClass;

class WebAuthManager {
public:
    WebAuthManager(WebServerClass& server, ConfigManager& config);
    
    // Authentication methods
    bool isAuthenticated();
    void requestAuthentication();
    bool validateSession(const String& sessionId);
    String createSession();
    void removeSession(const String& sessionId);
    
    // Security methods
    bool validateCSRFToken(const String& token);
    String generateCSRFToken();
    bool checkRateLimit(const String& ip);
    void addSecurityHeaders();
    
private:
    WebServerClass& server;
    ConfigManager& config;
    
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

#endif // WEB_AUTH_MANAGER_H 