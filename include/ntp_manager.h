#ifndef NTP_MANAGER_H
#define NTP_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include "logger.h"
#include "config.h"

/**
 * @file ntp_manager.h
 * @brief Manages NTP time synchronization for the ESP32
 * 
 * Synchronizes system time with NTP servers after WiFi connection.
 * Provides functions to get current time and formatted time strings.
 */
class NTPManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton instance
     */
    static NTPManager& getInstance();

    /**
     * @brief Initialize NTP manager
     * @param ntpServer Primary NTP server (default: "pool.ntp.org")
     * @param timezoneOffset Timezone offset in seconds from UTC (default: 0 = UTC)
     * @param daylightOffset Daylight saving offset in seconds (default: 0 = no DST)
     * @return True if initialized successfully
     */
    bool begin(const char* ntpServer = "pool.ntp.org", 
               int timezoneOffset = 0, 
               int daylightOffset = 0);

    /**
     * @brief Sync time with NTP server
     * @param timeout Timeout in milliseconds (default: 10000 = 10 seconds)
     * @return True if sync was successful
     */
    bool syncTime(unsigned long timeout = 10000);

    /**
     * @brief Check if time has been synchronized
     * @return True if time is synchronized
     */
    bool isTimeSet() const;

    /**
     * @brief Get current time as time_t (Unix timestamp)
     * @return Current time, or 0 if not set
     */
    time_t getCurrentTime() const;

    /**
     * @brief Get formatted time string
     * @param format Time format string (default: "%Y-%m-%d %H:%M:%S")
     * @return Formatted time string
     */
    String getFormattedTime(const char* format = "%Y-%m-%d %H:%M:%S") const;

    /**
     * @brief Get formatted time string with milliseconds since boot
     * @return Formatted string like "2025-11-11 13:30:45 (uptime: 12345s)"
     */
    String getFormattedTimeWithUptime() const;

    /**
     * @brief Set timezone offset
     * @param offset Timezone offset in seconds from UTC
     */
    void setTimezoneOffset(int offset);

    /**
     * @brief Set daylight saving offset
     * @param offset Daylight saving offset in seconds
     */
    void setDaylightOffset(int offset);

    /**
     * @brief Set NTP server
     * @param server NTP server hostname or IP
     */
    void setNTPServer(const char* server);

    /**
     * @brief Get timezone offset
     * @return Timezone offset in seconds
     */
    int getTimezoneOffset() const { return _timezoneOffset; }

    /**
     * @brief Get daylight saving offset
     * @return Daylight saving offset in seconds
     */
    int getDaylightOffset() const { return _daylightOffset; }

    /**
     * @brief Get NTP server
     * @return NTP server hostname
     */
    const char* getNTPServer() const { return _ntpServer; }

private:
    NTPManager();
    ~NTPManager();

    // Prevent copying
    NTPManager(const NTPManager&) = delete;
    NTPManager& operator=(const NTPManager&) = delete;

    static NTPManager* _instance;

    char _ntpServer[64];        // NTP server hostname
    int _timezoneOffset;        // Timezone offset in seconds
    int _daylightOffset;         // Daylight saving offset in seconds
    bool _timeSet;               // Whether time has been synchronized
    unsigned long _lastSyncTime; // Last sync attempt time
};

#endif // NTP_MANAGER_H

