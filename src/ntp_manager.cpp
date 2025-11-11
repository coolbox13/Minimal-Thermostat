#include "ntp_manager.h"
#include <WiFi.h>

static const char* TAG = "NTP";

NTPManager* NTPManager::_instance = nullptr;

NTPManager::NTPManager() 
    : _timezoneOffset(0)
    , _daylightOffset(0)
    , _timeSet(false)
    , _lastSyncTime(0)
{
    strncpy(_ntpServer, "pool.ntp.org", sizeof(_ntpServer) - 1);
    _ntpServer[sizeof(_ntpServer) - 1] = '\0';
}

NTPManager::~NTPManager() {
}

NTPManager& NTPManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new NTPManager();
    }
    return *_instance;
}

bool NTPManager::begin(const char* ntpServer, int timezoneOffset, int daylightOffset) {
    if (ntpServer != nullptr && strlen(ntpServer) > 0) {
        strncpy(_ntpServer, ntpServer, sizeof(_ntpServer) - 1);
        _ntpServer[sizeof(_ntpServer) - 1] = '\0';
    }
    
    _timezoneOffset = timezoneOffset;
    _daylightOffset = daylightOffset;

    LOG_I(TAG, "NTP Manager initialized");
    LOG_I(TAG, "NTP Server: %s", _ntpServer);
    LOG_I(TAG, "Timezone offset: %d seconds", _timezoneOffset);
    LOG_I(TAG, "Daylight offset: %d seconds", _daylightOffset);

    return true;
}

bool NTPManager::syncTime(unsigned long timeout) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_W(TAG, "Cannot sync time: WiFi not connected");
        return false;
    }

    LOG_I(TAG, "Synchronizing time with NTP server: %s", _ntpServer);

    // Configure time with timezone
    configTime(_timezoneOffset, _daylightOffset, _ntpServer);

    // Wait for time to be set (with timeout)
    unsigned long startTime = millis();
    struct tm timeinfo;
    
    while (!getLocalTime(&timeinfo)) {
        if (millis() - startTime > timeout) {
            LOG_E(TAG, "NTP time sync failed: timeout after %lu ms", timeout);
            _timeSet = false;
            return false;
        }
        delay(100);
    }

    _timeSet = true;
    _lastSyncTime = millis();

    // Log synchronized time
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    LOG_I(TAG, "Time synchronized successfully: %s", timeStr);

    return true;
}

bool NTPManager::isTimeSet() const {
    return _timeSet;
}

time_t NTPManager::getCurrentTime() const {
    // Return UTC Unix timestamp (seconds since epoch)
    // time() returns UTC timestamp regardless of timezone settings
    time_t now = time(nullptr);
    if (now < 0) {
        return 0;  // Time not set
    }
    return now;
}

String NTPManager::getFormattedTime(const char* format) const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time not set";
    }

    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), format, &timeinfo);
    return String(timeStr);
}

String NTPManager::getFormattedTimeWithUptime() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time not set";
    }

    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    unsigned long uptime = millis() / 1000;
    char result[128];
    snprintf(result, sizeof(result), "%s (uptime: %lus)", timeStr, uptime);
    
    return String(result);
}

void NTPManager::setTimezoneOffset(int offset) {
    _timezoneOffset = offset;
    if (_timeSet) {
        // Reconfigure time with new offset
        configTime(_timezoneOffset, _daylightOffset, _ntpServer);
    }
    LOG_I(TAG, "Timezone offset set to: %d seconds", offset);
}

void NTPManager::setDaylightOffset(int offset) {
    _daylightOffset = offset;
    if (_timeSet) {
        // Reconfigure time with new offset
        configTime(_timezoneOffset, _daylightOffset, _ntpServer);
    }
    LOG_I(TAG, "Daylight saving offset set to: %d seconds", offset);
}

void NTPManager::setNTPServer(const char* server) {
    if (server != nullptr && strlen(server) > 0) {
        strncpy(_ntpServer, server, sizeof(_ntpServer) - 1);
        _ntpServer[sizeof(_ntpServer) - 1] = '\0';
        LOG_I(TAG, "NTP server set to: %s", _ntpServer);
    }
}

