#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <Preferences.h>

/**
 * @brief Configuration manager for persistent storage of thermostat settings
 *
 * PREFERENCES NAMESPACE STRUCTURE:
 * This class uses multiple ESP32 Preferences namespaces to organize data:
 *
 * 1. "thermostat" - Main application configuration (persistent across reboots)
 *    - Network settings (WiFi credentials)
 *    - MQTT configuration
 *    - KNX addressing
 *    - PID controller parameters
 *    - Timing intervals and timeouts
 *    All settings in this namespace persist across reboots and are user-configurable.
 *
 * 2. "config" - System diagnostic data (runtime tracking)
 *    - Reboot reasons and counts
 *    - Watchdog reboot tracking
 *    - Last connected timestamp
 *    This namespace stores diagnostic/troubleshooting information.
 *
 * 3. "watchdog" - Watchdog state (managed by WatchdogManager)
 *    - See watchdog_manager.cpp for details
 *    - Consecutive reset tracking
 *    - Safe mode state
 *
 * RATIONALE FOR SEPARATION:
 * - Separating configuration from diagnostics allows independent backup/restore
 * - Different namespaces prevent accidental overwriting of critical data
 * - Diagnostic data can be cleared without affecting configuration
 *
 * NOTE: Future improvement could consolidate "config" and "watchdog" into
 * a single "diagnostics" namespace. Current structure maintained for
 * backward compatibility with existing deployments.
 */
class ConfigManager {
public:
    /**
     * @brief Get the singleton instance of ConfigManager
     * @return Pointer to the ConfigManager instance
     */
    static ConfigManager* getInstance();

    /**
     * @brief Initialize the configuration manager and load stored settings
     * @return true if initialization successful, false otherwise
     */
    bool begin();

    /**
     * @brief Close the preferences storage
     */
    void end();

    // Network settings
    /**
     * @brief Get the configured WiFi SSID
     * @return WiFi network name
     */
    String getWifiSSID();

    /**
     * @brief Set the WiFi SSID for network connection
     * @param ssid Network name (max 32 characters)
     */
    void setWifiSSID(const String& ssid);

    /**
     * @brief Get the configured WiFi password
     * @return WiFi password
     */
    String getWifiPassword();

    /**
     * @brief Set the WiFi password for network connection
     * @param password Network password (max 64 characters)
     */
    void setWifiPassword(const String& password);

    // NTP settings
    /**
     * @brief Get the NTP server hostname
     * @return NTP server hostname or IP address
     */
    String getNtpServer();

    /**
     * @brief Set the NTP server hostname
     * @param server NTP server hostname or IP address
     */
    void setNtpServer(const String& server);

    /**
     * @brief Get the timezone offset in seconds from UTC
     * @return Timezone offset in seconds
     */
    int getNtpTimezoneOffset();

    /**
     * @brief Set the timezone offset in seconds from UTC
     * @param offset Timezone offset in seconds (e.g., UTC+1 = 3600, UTC+2 = 7200)
     */
    void setNtpTimezoneOffset(int offset);

    /**
     * @brief Get the daylight saving time offset in seconds
     * @return Daylight saving offset in seconds
     */
    int getNtpDaylightOffset();

    /**
     * @brief Set the daylight saving time offset in seconds
     * @param offset Daylight saving offset in seconds (typically 3600 for 1 hour)
     */
    void setNtpDaylightOffset(int offset);

    // MQTT settings
    /**
     * @brief Get the MQTT broker server address
     * @return Server hostname or IP address
     */
    String getMqttServer();

    /**
     * @brief Set the MQTT broker server address
     * @param server Hostname or IP address of MQTT broker
     */
    void setMqttServer(const String& server);

    /**
     * @brief Get the MQTT broker port number
     * @return Port number (default: 1883)
     */
    uint16_t getMqttPort();

    /**
     * @brief Set the MQTT broker port number
     * @param port Port number (1-65535)
     */
    void setMqttPort(uint16_t port);

    // KNX settings
    /**
     * @brief Get the KNX area address component
     * @return Area number (0-15)
     */
    uint8_t getKnxArea();

    /**
     * @brief Set the KNX area address component
     * @param area Area number (0-15)
     */
    void setKnxArea(uint8_t area);

    /**
     * @brief Get the KNX line address component
     * @return Line number (0-15)
     */
    uint8_t getKnxLine();

    /**
     * @brief Set the KNX line address component
     * @param line Line number (0-15)
     */
    void setKnxLine(uint8_t line);

    /**
     * @brief Get the KNX member address component
     * @return Member number (0-255)
     */
    uint8_t getKnxMember();

    /**
     * @brief Set the KNX member address component
     * @param member Member number (0-255)
     */
    void setKnxMember(uint8_t member);

    /**
     * @brief Check if test KNX addresses should be used
     * @return true if test addresses enabled, false for production
     */
    bool getUseTestAddresses();

    /**
     * @brief Enable or disable test KNX addresses
     * @param useTest true for test addresses, false for production
     */
    void setUseTestAddresses(bool useTest);

    /**
     * @brief Get the KNX valve command address area component (production mode)
     * @return Area number (0-15) for valve command
     */
    uint8_t getKnxValveCommandArea();

    /**
     * @brief Set the KNX valve command address area component (production mode)
     * @param area Area number (0-15) for valve command
     */
    void setKnxValveCommandArea(uint8_t area);

    /**
     * @brief Get the KNX valve command address line component (production mode)
     * @return Line number (0-15) for valve command
     */
    uint8_t getKnxValveCommandLine();

    /**
     * @brief Set the KNX valve command address line component (production mode)
     * @param line Line number (0-15) for valve command
     */
    void setKnxValveCommandLine(uint8_t line);

    /**
     * @brief Get the KNX valve command address member component (production mode)
     * @return Member number (0-255) for valve command
     */
    uint8_t getKnxValveCommandMember();

    /**
     * @brief Set the KNX valve command address member component (production mode)
     * @param member Member number (0-255) for valve command
     */
    void setKnxValveCommandMember(uint8_t member);

    /**
     * @brief Get the KNX valve feedback address area component (production mode)
     * @return Area number (0-15) for valve position feedback
     */
    uint8_t getKnxValveFeedbackArea();

    /**
     * @brief Set the KNX valve feedback address area component (production mode)
     * @param area Area number (0-15) for valve position feedback
     */
    void setKnxValveFeedbackArea(uint8_t area);

    /**
     * @brief Get the KNX valve feedback address line component (production mode)
     * @return Line number (0-15) for valve position feedback
     */
    uint8_t getKnxValveFeedbackLine();

    /**
     * @brief Set the KNX valve feedback address line component (production mode)
     * @param line Line number (0-15) for valve position feedback
     */
    void setKnxValveFeedbackLine(uint8_t line);

    /**
     * @brief Get the KNX valve feedback address member component (production mode)
     * @return Member number (0-255) for valve position feedback
     */
    uint8_t getKnxValveFeedbackMember();

    /**
     * @brief Set the KNX valve feedback address member component (production mode)
     * @param member Member number (0-255) for valve position feedback
     */
    void setKnxValveFeedbackMember(uint8_t member);

    // PID Controller settings
    /**
     * @brief Get the proportional gain (Kp) parameter
     * @return Kp value (rounded to 2 decimal places)
     */
    float getPidKp();

    /**
     * @brief Set the proportional gain (Kp) parameter
     * @param kp Kp value (will be rounded to 2 decimal places)
     */
    void setPidKp(float kp);

    /**
     * @brief Get the integral gain (Ki) parameter
     * @return Ki value (rounded to 3 decimal places)
     */
    float getPidKi();

    /**
     * @brief Set the integral gain (Ki) parameter
     * @param ki Ki value (will be rounded to 3 decimal places)
     */
    void setPidKi(float ki);

    /**
     * @brief Get the derivative gain (Kd) parameter
     * @return Kd value (rounded to 3 decimal places)
     */
    float getPidKd();

    /**
     * @brief Set the derivative gain (Kd) parameter
     * @param kd Kd value (will be rounded to 3 decimal places)
     */
    void setPidKd(float kd);

    /**
     * @brief Get the temperature setpoint
     * @return Setpoint in °C (rounded to 1 decimal place)
     */
    float getSetpoint();

    /**
     * @brief Set the temperature setpoint
     * @param setpoint Target temperature in °C (5-30°C, rounded to 1 decimal)
     */
    void setSetpoint(float setpoint);

    // Timing parameters
    uint32_t getSensorUpdateInterval();
    void setSensorUpdateInterval(uint32_t interval);

    uint32_t getHistoryUpdateInterval();
    void setHistoryUpdateInterval(uint32_t interval);

    uint32_t getPidUpdateInterval();
    void setPidUpdateInterval(uint32_t interval);

    uint32_t getConnectivityCheckInterval();
    void setConnectivityCheckInterval(uint32_t interval);

    uint32_t getPidConfigWriteInterval();
    void setPidConfigWriteInterval(uint32_t interval);

    uint16_t getWifiConnectTimeout();
    void setWifiConnectTimeout(uint16_t timeout);

    uint8_t getMaxReconnectAttempts();
    void setMaxReconnectAttempts(uint8_t attempts);

    uint32_t getSystemWatchdogTimeout();
    void setSystemWatchdogTimeout(uint32_t timeout);

    uint32_t getWifiWatchdogTimeout();
    void setWifiWatchdogTimeout(uint32_t timeout);

    float getPidDeadband();
    void setPidDeadband(float deadband);

    float getPidAdaptationInterval();
    void setPidAdaptationInterval(float interval);

    // Preset mode settings
    /**
     * @brief Get the current active preset mode
     * @return Current preset name (none, eco, comfort, away, sleep, boost)
     */
    String getCurrentPreset();

    /**
     * @brief Set the current active preset mode
     * @param preset Preset name (none, eco, comfort, away, sleep, boost)
     */
    void setCurrentPreset(const String& preset);

    /**
     * @brief Get the temperature setpoint for a specific preset
     * @param preset Preset name (eco, comfort, away, sleep, boost)
     * @return Temperature in °C for the preset
     */
    float getPresetTemperature(const String& preset);

    /**
     * @brief Set the temperature setpoint for a specific preset
     * @param preset Preset name (eco, comfort, away, sleep, boost)
     * @param temperature Temperature in °C (5-30°C, rounded to 1 decimal)
     */
    void setPresetTemperature(const String& preset, float temperature);

    // Manual valve override settings
    /**
     * @brief Check if manual valve override is enabled
     * @return true if manual override is active, false otherwise
     */
    bool getManualOverrideEnabled();

    /**
     * @brief Enable or disable manual valve override
     * @param enabled true to enable manual override, false to disable
     */
    void setManualOverrideEnabled(bool enabled);

    /**
     * @brief Get the manual override valve position
     * @return Manual valve position (0-100%)
     */
    uint8_t getManualOverridePosition();

    /**
     * @brief Set the manual override valve position
     * @param position Valve position 0-100%
     */
    void setManualOverridePosition(uint8_t position);

    /**
     * @brief Get the manual override timeout duration in seconds
     * @return Timeout duration in seconds (0 = no timeout)
     */
    uint32_t getManualOverrideTimeout();

    /**
     * @brief Set the manual override timeout duration
     * @param timeout Timeout in seconds (0 = no auto-disable, default 3600 = 1 hour)
     */
    void setManualOverrideTimeout(uint32_t timeout);

    /**
     * @brief Get when manual override was activated (millis timestamp)
     * @return Activation timestamp in milliseconds since boot
     */
    unsigned long getManualOverrideActivationTime();

    /**
     * @brief Set when manual override was activated
     * @param timestamp Activation time in milliseconds since boot
     */
    void setManualOverrideActivationTime(unsigned long timestamp);

    // Webhook settings
    /**
     * @brief Get the webhook URL for IFTTT/Zapier integration
     * @return Webhook URL
     */
    String getWebhookUrl();

    /**
     * @brief Set the webhook URL for IFTTT/Zapier integration
     * @param url Webhook endpoint URL
     */
    void setWebhookUrl(const String& url);

    /**
     * @brief Check if webhooks are enabled
     * @return true if enabled, false otherwise
     */
    bool getWebhookEnabled();

    /**
     * @brief Enable or disable webhooks
     * @param enabled true to enable, false to disable
     */
    void setWebhookEnabled(bool enabled);

    /**
     * @brief Get low temperature alert threshold
     * @return Temperature in °C below which alert is sent
     */
    float getWebhookTempLowThreshold();

    /**
     * @brief Set low temperature alert threshold
     * @param threshold Temperature in °C below which alert is sent
     */
    void setWebhookTempLowThreshold(float threshold);

    /**
     * @brief Get high temperature alert threshold
     * @return Temperature in °C above which alert is sent
     */
    float getWebhookTempHighThreshold();

    /**
     * @brief Set high temperature alert threshold
     * @param threshold Temperature in °C above which alert is sent
     */
    void setWebhookTempHighThreshold(float threshold);

    /**
     * @brief Export all configuration settings to JSON
     * @param doc JsonDocument to populate with current settings
     */
    void getJson(JsonDocument& doc);

    /**
     * @brief Import configuration settings from JSON
     * @param doc JsonDocument containing settings to import
     * @return true if all settings valid and applied, false on error
     */
    bool setFromJson(const JsonDocument& doc);

    /**
     * @brief Import configuration settings from JSON with error reporting
     * @param doc JsonDocument containing settings to import
     * @param errorMessage Output parameter set to error description on failure
     * @return true if all settings valid and applied, false on error
     */
    bool setFromJson(const JsonDocument& doc, String& errorMessage);

    /**
     * @brief Round a float value to specified decimal places
     * @param value Value to round
     * @param decimals Number of decimal places (0-6)
     * @return Rounded value
     *
     * Used to ensure consistent precision for PID parameters:
     * - Kp: 2 decimals
     * - Ki/Kd: 3 decimals
     * - Setpoint: 1 decimal
     */
    static float roundToPrecision(float value, int decimals);

private:
    ConfigManager();
    static ConfigManager* _instance;
    Preferences _preferences;

    // Default values
    static constexpr float DEFAULT_KP = 2.0f;
    static constexpr float DEFAULT_KI = 0.1f;
    static constexpr float DEFAULT_KD = 0.5f;
    static constexpr float DEFAULT_SETPOINT = 22.0f;
    static constexpr uint16_t DEFAULT_MQTT_PORT = 1883;
    static constexpr uint8_t DEFAULT_KNX_AREA = 1;
    static constexpr uint8_t DEFAULT_KNX_LINE = 1;
    static constexpr uint8_t DEFAULT_KNX_MEMBER = 159;

    // Default timing values (matching config.h constants)
    static constexpr uint32_t DEFAULT_SENSOR_UPDATE_INTERVAL_MS = 30000;
    static constexpr uint32_t DEFAULT_HISTORY_UPDATE_INTERVAL_MS = 300000;  // 5 minutes for 24-hour history
    static constexpr uint32_t DEFAULT_PID_UPDATE_INTERVAL_MS = 10000;
    static constexpr uint32_t DEFAULT_CONNECTIVITY_CHECK_INTERVAL_MS = 300000;
    static constexpr uint32_t DEFAULT_PID_CONFIG_WRITE_INTERVAL_MS = 300000;
    static constexpr uint16_t DEFAULT_WIFI_CONNECT_TIMEOUT_SEC = 180;
    static constexpr uint8_t DEFAULT_MAX_RECONNECT_ATTEMPTS = 10;
    static constexpr uint32_t DEFAULT_SYSTEM_WATCHDOG_TIMEOUT_MS = 2700000;
    static constexpr uint32_t DEFAULT_WIFI_WATCHDOG_TIMEOUT_MS = 1800000;
    static constexpr float DEFAULT_PID_DEADBAND = 0.2f;
    static constexpr float DEFAULT_PID_ADAPTATION_INTERVAL_SEC = 60.0f;
    static constexpr uint8_t DEFAULT_MANUAL_OVERRIDE_POSITION = 0;
    static constexpr uint32_t DEFAULT_MANUAL_OVERRIDE_TIMEOUT_SEC = 3600; // 1 hour
    static constexpr float DEFAULT_WEBHOOK_TEMP_LOW_THRESHOLD = 15.0f;
    static constexpr float DEFAULT_WEBHOOK_TEMP_HIGH_THRESHOLD = 30.0f;

    // Default preset temperatures
    static constexpr float DEFAULT_PRESET_ECO = 18.0f;
    static constexpr float DEFAULT_PRESET_COMFORT = 22.0f;
    static constexpr float DEFAULT_PRESET_AWAY = 16.0f;
    static constexpr float DEFAULT_PRESET_SLEEP = 19.0f;
    static constexpr float DEFAULT_PRESET_BOOST = 24.0f;

    // Validation helper functions
    bool validateAndApplyNetworkSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyMQTTSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyKNXSettings(const JsonDocument& doc, String& errorMessage);
    bool validateBME280Settings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyPIDSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyManualOverrideSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyTimingSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyWebhookSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyPresetSettings(const JsonDocument& doc, String& errorMessage);

public:
        /**
         * @brief Set the last reboot reason
         * @param reason Reason for the last reboot
         */
        void setLastRebootReason(const String& reason);
        
        /**
         * @brief Get the last reboot reason
         * @return Last reboot reason
         */
        String getLastRebootReason();
        
        /**
         * @brief Set the reboot count
         * @param count Number of reboots
         */
        void setRebootCount(int count);
        
        /**
         * @brief Get the reboot count
         * @return Number of reboots
         */
        int getRebootCount();
        
        /**
         * @brief Set the consecutive watchdog reboot count
         * @param count Number of consecutive watchdog reboots
         */
        void setConsecutiveWatchdogReboots(int count);
        
        /**
         * @brief Get the consecutive watchdog reboot count
         * @return Number of consecutive watchdog reboots
         */
        int getConsecutiveWatchdogReboots();
        
        /**
         * @brief Set the timestamp of the last successful WiFi connection
         * @param timestamp Timestamp in milliseconds
         */
        void setLastConnectedTime(unsigned long timestamp);

        /**
         * @brief Perform a factory reset by clearing all stored preferences
         *
         * This method clears all three preference namespaces:
         * - "thermostat": User configuration (WiFi, MQTT, KNX, PID, timing)
         * - "config": Diagnostic data (reboot reasons, timestamps)
         * - "watchdog": Recovery state (consecutive resets, safe mode)
         *
         * After clearing, it reinitializes the "thermostat" namespace with
         * default values. The caller is responsible for rebooting the device
         * after calling this method.
         *
         * @return true if factory reset was successful, false otherwise
         */
        bool factoryReset();
};

#endif // CONFIG_MANAGER_H