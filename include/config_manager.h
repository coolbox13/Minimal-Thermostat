#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <Preferences.h>

class ConfigManager {
public:
    static ConfigManager* getInstance();
    
    bool begin();
    void end();
    
    // Network settings
    String getWifiSSID();
    void setWifiSSID(const String& ssid);
    
    String getWifiPassword();
    void setWifiPassword(const String& password);
    
    // MQTT settings
    String getMqttServer();
    void setMqttServer(const String& server);
    
    uint16_t getMqttPort();
    void setMqttPort(uint16_t port);
    
    // KNX settings
    uint8_t getKnxArea();
    void setKnxArea(uint8_t area);
    
    uint8_t getKnxLine();
    void setKnxLine(uint8_t line);
    
    uint8_t getKnxMember();
    void setKnxMember(uint8_t member);
    
    bool getUseTestAddresses();
    void setUseTestAddresses(bool useTest);
    
    // PID Controller settings
    float getPidKp();
    void setPidKp(float kp);
    
    float getPidKi();
    void setPidKi(float ki);
    
    float getPidKd();
    void setPidKd(float kd);
    
    float getSetpoint();
    void setSetpoint(float setpoint);
    
    // Export all settings as JSON
    void getJson(JsonDocument& doc);
    
    // Import settings from JSON (original version for backward compatibility)
    bool setFromJson(const JsonDocument& doc);
    
    // Enhanced version with error reporting
    bool setFromJson(const JsonDocument& doc, String& errorMessage);

    // Utility function for consistent rounding
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

    // Validation helper functions
    bool validateAndApplyNetworkSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyMQTTSettings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyKNXSettings(const JsonDocument& doc, String& errorMessage);
    bool validateBME280Settings(const JsonDocument& doc, String& errorMessage);
    bool validateAndApplyPIDSettings(const JsonDocument& doc, String& errorMessage);

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
};

#endif // CONFIG_MANAGER_H