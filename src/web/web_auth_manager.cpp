#include "web_auth_manager.h"
#include "config_manager.h"

#ifdef ESP32
  #include <WebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WebServer.h>
#endif

WebAuthManager::WebAuthManager(WebServerClass& server, ConfigManager& config)
    : server(server), config(config) {
}

bool WebAuthManager::isAuthenticated() {
    // First check rate limiting
    String ip = server.client().remoteIP().toString();
    if (!checkRateLimit(ip)) {
        return false;
    }
    
    // Check if session cookie exists
    String sessionId = server.header("Cookie");
    if (sessionId.length() > 0) {
        // Extract session ID from cookie
        int start = sessionId.indexOf("session=");
        if (start >= 0) {
            start += 8; // Length of "session="
            int end = sessionId.indexOf(";", start);
            if (end < 0) end = sessionId.length();
            sessionId = sessionId.substring(start, end);
            
            // Validate session
            if (validateSession(sessionId)) {
                return true;
            }
        }
    }
    
    // If no valid session, check basic auth
    if (config.getWebUsername()[0] != '\0') {
        if (server.authenticate(config.getWebUsername(), config.getWebPassword())) {
            // Create new session
            String newSession = createSession();
            server.sendHeader("Set-Cookie", "session=" + newSession + "; Path=/; Max-Age=3600; HttpOnly; SameSite=Strict");
            return true;
        }
        return false;
    }
    
    return true; // No authentication required if no credentials set
}

void WebAuthManager::requestAuthentication() {
    server.requestAuthentication();
}

bool WebAuthManager::validateSession(const String& sessionId) {
    auto it = sessions.find(sessionId);
    if (it != sessions.end()) {
        Session& session = it->second;
        unsigned long now = millis();
        
        // Check if session has expired
        if (now - session.created > SESSION_TIMEOUT) {
            sessions.erase(it);
            return false;
        }
        
        // Update last access time
        session.lastAccess = now;
        return true;
    }
    return false;
}

String WebAuthManager::createSession() {
    cleanupSessions();
    
    String sessionId = generateRandomString(32);
    Session session;
    session.ip = server.client().remoteIP().toString();
    session.created = millis();
    session.lastAccess = session.created;
    
    sessions[sessionId] = session;
    return sessionId;
}

void WebAuthManager::removeSession(const String& sessionId) {
    sessions.erase(sessionId);
}

bool WebAuthManager::validateCSRFToken(const String& token) {
    // Get the CSRF token from the session cookie
    String sessionId = server.header("Cookie");
    if (sessionId.length() > 0) {
        int start = sessionId.indexOf("session=");
        if (start >= 0) {
            start += 8; // Length of "session="
            int end = sessionId.indexOf(";", start);
            if (end < 0) end = sessionId.length();
            sessionId = sessionId.substring(start, end);
            
            // Check if session exists and is valid
            auto it = sessions.find(sessionId);
            if (it != sessions.end()) {
                // Compare the provided token with the session's CSRF token
                return token == it->second.csrfToken;
            }
        }
    }
    return false;
}

String WebAuthManager::generateCSRFToken() {
    // Get the current session ID
    String sessionId = server.header("Cookie");
    if (sessionId.length() > 0) {
        int start = sessionId.indexOf("session=");
        if (start >= 0) {
            start += 8; // Length of "session="
            int end = sessionId.indexOf(";", start);
            if (end < 0) end = sessionId.length();
            sessionId = sessionId.substring(start, end);
            
            // Check if session exists and is valid
            auto it = sessions.find(sessionId);
            if (it != sessions.end()) {
                // Generate a new CSRF token for this session
                String token = generateRandomString(32);
                it->second.csrfToken = token;
                return token;
            }
        }
    }
    
    // If no valid session, generate a temporary token
    return generateRandomString(32);
}

bool WebAuthManager::checkRateLimit(const String& ip) {
    cleanupRateLimits();
    
    auto it = rateLimits.find(ip);
    if (it != rateLimits.end()) {
        RateLimit& limit = it->second;
        unsigned long now = millis();
        
        // Reset if window has expired
        if (now - limit.firstAttempt > RATE_LIMIT_WINDOW) {
            limit.firstAttempt = now;
            limit.attempts = 1;
            return true;
        }
        
        // Check if too many attempts
        if (limit.attempts >= MAX_ATTEMPTS) {
            return false;
        }
        
        limit.attempts++;
        return true;
    }
    
    // First attempt for this IP
    RateLimit limit;
    limit.firstAttempt = millis();
    limit.attempts = 1;
    rateLimits[ip] = limit;
    return true;
}

void WebAuthManager::addSecurityHeaders() {
    server.sendHeader("X-Content-Type-Options", "nosniff");
    server.sendHeader("X-Frame-Options", "DENY");
    server.sendHeader("X-XSS-Protection", "1; mode=block");
    server.sendHeader("Strict-Transport-Security", "max-age=31536000; includeSubDomains");
    server.sendHeader("Content-Security-Policy", "default-src 'self'");
    server.sendHeader("Referrer-Policy", "same-origin");
}

void WebAuthManager::cleanupSessions() {
    unsigned long now = millis();
    for (auto it = sessions.begin(); it != sessions.end();) {
        if (now - it->second.created > SESSION_TIMEOUT) {
            it = sessions.erase(it);
        } else {
            ++it;
        }
    }
}

void WebAuthManager::cleanupRateLimits() {
    unsigned long now = millis();
    for (auto it = rateLimits.begin(); it != rateLimits.end();) {
        if (now - it->second.firstAttempt > RATE_LIMIT_WINDOW) {
            it = rateLimits.erase(it);
        } else {
            ++it;
        }
    }
}

String WebAuthManager::generateRandomString(size_t length) {
    const char charset[] = "0123456789"
                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz";
    String result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; i++) {
        int index = random(0, sizeof(charset) - 1);
        result += charset[index];
    }
    
    return result;
} 