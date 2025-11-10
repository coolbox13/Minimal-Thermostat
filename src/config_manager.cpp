#include "config_manager.h"
#include "logger.h"

// Static tag for logging
static const char* TAG = "CONFIG";

// Initialize static instance
ConfigManager* ConfigManager::_instance = nullptr;

ConfigManager* ConfigManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new ConfigManager();
    }
    return _instance;
}

ConfigManager::ConfigManager() {
    // Private constructor
}

float ConfigManager::roundToPrecision(float value, int decimals) {
    float multiplier = pow(10.0f, decimals);
    return roundf(value * multiplier) / multiplier;
}

bool ConfigManager::begin() {
    LOG_I(TAG, "Initializing configuration storage");
    if (!_preferences.begin("thermostat", false)) {
        LOG_E(TAG, "Failed to open preferences namespace");
        return false;
    }

    // Check if this is first run and initialize defaults
    if (!_preferences.isKey("initialized")) {
        // Set defaults for first run
        setWifiSSID("");
        setWifiPassword("");
        setMqttServer("192.168.178.32");
        setMqttPort(1883);
        setPidKp(DEFAULT_KP);
        setPidKi(DEFAULT_KI);
        setPidKd(DEFAULT_KD);
        setSetpoint(DEFAULT_SETPOINT);
        
        // Mark as initialized
        _preferences.putBool("initialized", true);
        LOG_I(TAG, "First run - initialized defaults");
    }

    return true;
}

void ConfigManager::end() {
    _preferences.end();
}

// Network settings
String ConfigManager::getWifiSSID() {
    return _preferences.getString("wifi_ssid", "");
}

void ConfigManager::setWifiSSID(const String& ssid) {
    _preferences.putString("wifi_ssid", ssid);
}

String ConfigManager::getWifiPassword() {
    return _preferences.getString("wifi_pass", "");
}

void ConfigManager::setWifiPassword(const String& password) {
    _preferences.putString("wifi_pass", password);
}

// MQTT settings
String ConfigManager::getMqttServer() {
    return _preferences.getString("mqtt_srv", "192.168.178.32");
}

void ConfigManager::setMqttServer(const String& server) {
    _preferences.putString("mqtt_srv", server);
}

uint16_t ConfigManager::getMqttPort() {
    return _preferences.getUShort("mqtt_port", DEFAULT_MQTT_PORT);
}

void ConfigManager::setMqttPort(uint16_t port) {
    _preferences.putUShort("mqtt_port", port);
}

// KNX settings
uint8_t ConfigManager::getKnxArea() {
    return _preferences.getUChar("knx_area", DEFAULT_KNX_AREA);
}

void ConfigManager::setKnxArea(uint8_t area) {
    _preferences.putUChar("knx_area", area);
}

uint8_t ConfigManager::getKnxLine() {
    return _preferences.getUChar("knx_line", DEFAULT_KNX_LINE);
}

void ConfigManager::setKnxLine(uint8_t line) {
    _preferences.putUChar("knx_line", line);
}

uint8_t ConfigManager::getKnxMember() {
    return _preferences.getUChar("knx_member", DEFAULT_KNX_MEMBER);
}

void ConfigManager::setKnxMember(uint8_t member) {
    _preferences.putUChar("knx_member", member);
}

bool ConfigManager::getUseTestAddresses() {
    return _preferences.getBool("knx_test", true);
}

void ConfigManager::setUseTestAddresses(bool useTest) {
    _preferences.putBool("knx_test", useTest);
}

// PID Controller settings
float ConfigManager::getPidKp() {
    return roundToPrecision(_preferences.getFloat("pid_kp", DEFAULT_KP), 2);
}
void ConfigManager::setPidKp(float kp) {
    _preferences.putFloat("pid_kp", roundToPrecision(kp, 2));
}
void ConfigManager::setPidKi(float ki) {
    _preferences.putFloat("pid_ki", roundToPrecision(ki, 3));
}
void ConfigManager::setPidKd(float kd) {
    _preferences.putFloat("pid_kd", roundToPrecision(kd, 3));
}
void ConfigManager::setSetpoint(float setpoint) {
    _preferences.putFloat("setpoint", roundToPrecision(setpoint, 1));
}
float ConfigManager::getPidKi() {
    return roundToPrecision(_preferences.getFloat("pid_ki", DEFAULT_KI), 3);
}
float ConfigManager::getPidKd() {
    return roundToPrecision(_preferences.getFloat("pid_kd", DEFAULT_KD), 3);
}
float ConfigManager::getSetpoint() {
    return roundToPrecision(_preferences.getFloat("setpoint", DEFAULT_SETPOINT), 1);
}

// Timing parameters
uint32_t ConfigManager::getSensorUpdateInterval() {
    return _preferences.getUInt("sens_upd_int", DEFAULT_SENSOR_UPDATE_INTERVAL_MS);
}
void ConfigManager::setSensorUpdateInterval(uint32_t interval) {
    _preferences.putUInt("sens_upd_int", interval);
}
uint32_t ConfigManager::getPidUpdateInterval() {
    return _preferences.getUInt("pid_upd_int", DEFAULT_PID_UPDATE_INTERVAL_MS);
}
void ConfigManager::setPidUpdateInterval(uint32_t interval) {
    _preferences.putUInt("pid_upd_int", interval);
}
uint32_t ConfigManager::getConnectivityCheckInterval() {
    return _preferences.getUInt("conn_chk_int", DEFAULT_CONNECTIVITY_CHECK_INTERVAL_MS);
}
void ConfigManager::setConnectivityCheckInterval(uint32_t interval) {
    _preferences.putUInt("conn_chk_int", interval);
}
uint32_t ConfigManager::getPidConfigWriteInterval() {
    return _preferences.getUInt("pid_wr_int", DEFAULT_PID_CONFIG_WRITE_INTERVAL_MS);
}
void ConfigManager::setPidConfigWriteInterval(uint32_t interval) {
    _preferences.putUInt("pid_wr_int", interval);
}
uint16_t ConfigManager::getWifiConnectTimeout() {
    return _preferences.getUShort("wifi_conn_to", DEFAULT_WIFI_CONNECT_TIMEOUT_SEC);
}
void ConfigManager::setWifiConnectTimeout(uint16_t timeout) {
    _preferences.putUShort("wifi_conn_to", timeout);
}
uint8_t ConfigManager::getMaxReconnectAttempts() {
    return _preferences.getUChar("max_reconn", DEFAULT_MAX_RECONNECT_ATTEMPTS);
}
void ConfigManager::setMaxReconnectAttempts(uint8_t attempts) {
    _preferences.putUChar("max_reconn", attempts);
}
uint32_t ConfigManager::getSystemWatchdogTimeout() {
    return _preferences.getUInt("sys_wdt_to", DEFAULT_SYSTEM_WATCHDOG_TIMEOUT_MS);
}
void ConfigManager::setSystemWatchdogTimeout(uint32_t timeout) {
    _preferences.putUInt("sys_wdt_to", timeout);
}
uint32_t ConfigManager::getWifiWatchdogTimeout() {
    return _preferences.getUInt("wifi_wdt_to", DEFAULT_WIFI_WATCHDOG_TIMEOUT_MS);
}
void ConfigManager::setWifiWatchdogTimeout(uint32_t timeout) {
    _preferences.putUInt("wifi_wdt_to", timeout);
}
float ConfigManager::getPidDeadband() {
    return roundToPrecision(_preferences.getFloat("pid_deadb", DEFAULT_PID_DEADBAND), 1);
}
void ConfigManager::setPidDeadband(float deadband) {
    _preferences.putFloat("pid_deadb", roundToPrecision(deadband, 1));
}
float ConfigManager::getPidAdaptationInterval() {
    return roundToPrecision(_preferences.getFloat("pid_adapt", DEFAULT_PID_ADAPTATION_INTERVAL_SEC), 1);
}
void ConfigManager::setPidAdaptationInterval(float interval) {
    _preferences.putFloat("pid_adapt", roundToPrecision(interval, 1));
}

// Webhook settings
String ConfigManager::getWebhookUrl() {
    return _preferences.getString("webhook_url", "");
}

void ConfigManager::setWebhookUrl(const String& url) {
    _preferences.putString("webhook_url", url);
}

bool ConfigManager::getWebhookEnabled() {
    return _preferences.getBool("webhook_en", false);
}

void ConfigManager::setWebhookEnabled(bool enabled) {
    _preferences.putBool("webhook_en", enabled);
}

float ConfigManager::getWebhookTempLowThreshold() {
    return roundToPrecision(_preferences.getFloat("webhook_low", DEFAULT_WEBHOOK_TEMP_LOW_THRESHOLD), 1);
}

void ConfigManager::setWebhookTempLowThreshold(float threshold) {
    _preferences.putFloat("webhook_low", roundToPrecision(threshold, 1));
}

float ConfigManager::getWebhookTempHighThreshold() {
    return roundToPrecision(_preferences.getFloat("webhook_hi", DEFAULT_WEBHOOK_TEMP_HIGH_THRESHOLD), 1);
}

void ConfigManager::setWebhookTempHighThreshold(float threshold) {
    _preferences.putFloat("webhook_hi", roundToPrecision(threshold, 1));
}

// MODIFIED: Updated getJson to use getters directly (which now apply rounding)
void ConfigManager::getJson(JsonDocument& doc) {
    // Create JSON structure directly
    doc["network"]["wifi_ssid"] = getWifiSSID();
    doc["network"]["wifi_pass"] = "**********"; // Don't expose password in JSON
    
    doc["mqtt"]["server"] = getMqttServer();
    doc["mqtt"]["port"] = getMqttPort();
    
    doc["knx"]["area"] = getKnxArea();
    doc["knx"]["line"] = getKnxLine();
    doc["knx"]["member"] = getKnxMember();
    doc["knx"]["use_test"] = getUseTestAddresses();
    
    // Add BME280 settings as hardcoded defaults for now
    doc["bme280"]["address"] = "0x76";  // Default BME280 address
    doc["bme280"]["sda_pin"] = 21;      // Default SDA pin
    doc["bme280"]["scl_pin"] = 22;      // Default SCL pin 
    doc["bme280"]["interval"] = 30;     // Default 30 second interval
    
    // Update PID values - no need for additional rounding since getters now apply it
    doc["pid"]["kp"] = getPidKp();
    doc["pid"]["ki"] = getPidKi();
    doc["pid"]["kd"] = getPidKd();
    doc["pid"]["setpoint"] = getSetpoint();
    doc["pid"]["deadband"] = getPidDeadband();
    doc["pid"]["adaptation_interval"] = getPidAdaptationInterval();

    // Add timing parameters
    doc["timing"]["sensor_update_interval"] = getSensorUpdateInterval();
    doc["timing"]["pid_update_interval"] = getPidUpdateInterval();
    doc["timing"]["connectivity_check_interval"] = getConnectivityCheckInterval();
    doc["timing"]["pid_config_write_interval"] = getPidConfigWriteInterval();
    doc["timing"]["wifi_connect_timeout"] = getWifiConnectTimeout();
    doc["timing"]["max_reconnect_attempts"] = getMaxReconnectAttempts();
    doc["timing"]["system_watchdog_timeout"] = getSystemWatchdogTimeout();
    doc["timing"]["wifi_watchdog_timeout"] = getWifiWatchdogTimeout();

    // Add webhook parameters
    doc["webhook"]["enabled"] = getWebhookEnabled();
    doc["webhook"]["url"] = getWebhookUrl();
    doc["webhook"]["temp_low_threshold"] = getWebhookTempLowThreshold();
    doc["webhook"]["temp_high_threshold"] = getWebhookTempHighThreshold();

    LOG_D(TAG, "Created JSON configuration");
}

// Validation helper functions
bool ConfigManager::validateAndApplyNetworkSettings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("network")) {
        return true;
    }
    if (doc["network"].containsKey("wifi_ssid")) {
        String ssid = doc["network"]["wifi_ssid"].as<String>();
        if (ssid.length() > 32) {
            errorMessage = "WiFi SSID too long (max 32 characters)";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setWifiSSID(ssid);
    }
    if (doc["network"].containsKey("wifi_pass")) {
        String pass = doc["network"]["wifi_pass"].as<String>();
        if (pass.length() > 0 && pass != "**********") {
            if (pass.length() > 64) {
                errorMessage = "WiFi password too long (max 64 characters)";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            setWifiPassword(pass);
        }
    }
    return true;
}
bool ConfigManager::validateAndApplyMQTTSettings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("mqtt")) {
        return true;
    }
    if (doc["mqtt"].containsKey("server")) {
        setMqttServer(doc["mqtt"]["server"].as<String>());
    }
    if (doc["mqtt"].containsKey("port")) {
        uint16_t port = doc["mqtt"]["port"].as<uint16_t>();
        if (port == 0) {
            errorMessage = "Invalid MQTT port";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setMqttPort(port);
    }
    return true;
}
bool ConfigManager::validateAndApplyKNXSettings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("knx")) {
        return true;
    }
    if (doc["knx"].containsKey("area")) {
        uint8_t area = doc["knx"]["area"].as<uint8_t>();
        if (area > 15) {
            errorMessage = "KNX area must be 0-15";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setKnxArea(area);
    }
    if (doc["knx"].containsKey("line")) {
        uint8_t line = doc["knx"]["line"].as<uint8_t>();
        if (line > 15) {
            errorMessage = "KNX line must be 0-15";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setKnxLine(line);
    }
    if (doc["knx"].containsKey("member")) {
        uint8_t member = doc["knx"]["member"].as<uint8_t>();
        if (member > 255) {
            errorMessage = "KNX member must be 0-255";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setKnxMember(member);
    }
    if (doc["knx"].containsKey("use_test")) {
        setUseTestAddresses(doc["knx"]["use_test"].as<bool>());
    }
    return true;
}
bool ConfigManager::validateBME280Settings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("bme280")) {
        return true;
    }
    if (doc["bme280"].containsKey("address")) {
        String address = doc["bme280"]["address"].as<String>();
        if (address != "0x76" && address != "0x77") {
            errorMessage = "BME280 address must be 0x76 or 0x77";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
    }
    if (doc["bme280"].containsKey("sda_pin")) {
        uint8_t pin = doc["bme280"]["sda_pin"].as<uint8_t>();
        if (pin > 39) {
            errorMessage = "BME280 SDA pin must be 0-39";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
    }
    if (doc["bme280"].containsKey("scl_pin")) {
        uint8_t pin = doc["bme280"]["scl_pin"].as<uint8_t>();
        if (pin > 39) {
            errorMessage = "BME280 SCL pin must be 0-39";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
    }
    if (doc["bme280"].containsKey("interval")) {
        uint16_t interval = doc["bme280"]["interval"].as<uint16_t>();
        if (interval < 1 || interval > 3600) {
            errorMessage = "BME280 interval must be 1-3600 seconds";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
    }
    return true;
}
bool ConfigManager::validateAndApplyPIDSettings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("pid")) {
        return true;
    }
    if (doc["pid"].containsKey("kp")) {
        float kp = roundToPrecision(doc["pid"]["kp"].as<float>(), 2);
        LOG_D(TAG, "Parsed Kp: %.2f (rounded)", kp);
        if (kp < 0) {
            errorMessage = "PID Kp must be >= 0";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidKp(kp);
    }
    if (doc["pid"].containsKey("ki")) {
        float ki = roundToPrecision(doc["pid"]["ki"].as<float>(), 3);
        LOG_D(TAG, "Parsed Ki: %.3f (rounded)", ki);
        if (ki < 0) {
            errorMessage = "PID Ki must be >= 0";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidKi(ki);
    }
    if (doc["pid"].containsKey("kd")) {
        float kd = roundToPrecision(doc["pid"]["kd"].as<float>(), 3);
        LOG_D(TAG, "Parsed Kd: %.3f (rounded)", kd);
        if (kd < 0) {
            errorMessage = "PID Kd must be >= 0";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidKd(kd);
    }
    if (doc["pid"].containsKey("setpoint")) {
        float setpoint = roundToPrecision(doc["pid"]["setpoint"].as<float>(), 1);
        LOG_D(TAG, "Parsed setpoint: %.1f (rounded)", setpoint);
        if (setpoint < 5 || setpoint > 30) {
            errorMessage = "Temperature setpoint must be between 5°C and 30°C";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setSetpoint(setpoint);
    }
    if (doc["pid"].containsKey("deadband")) {
        float deadband = roundToPrecision(doc["pid"]["deadband"].as<float>(), 1);
        if (deadband < 0 || deadband > 5) {
            errorMessage = "PID deadband must be between 0°C and 5°C";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidDeadband(deadband);
    }
    if (doc["pid"].containsKey("adaptation_interval")) {
        float interval = roundToPrecision(doc["pid"]["adaptation_interval"].as<float>(), 1);
        if (interval < 10 || interval > 600) {
            errorMessage = "PID adaptation interval must be between 10 and 600 seconds";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidAdaptationInterval(interval);
    }
    return true;
}
bool ConfigManager::validateAndApplyTimingSettings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("timing")) {
        return true;
    }
    if (doc["timing"].containsKey("sensor_update_interval")) {
        uint32_t interval = doc["timing"]["sensor_update_interval"].as<uint32_t>();
        if (interval < 1000 || interval > 300000) {
            errorMessage = "Sensor update interval must be between 1000ms and 300000ms";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setSensorUpdateInterval(interval);
    }
    if (doc["timing"].containsKey("pid_update_interval")) {
        uint32_t interval = doc["timing"]["pid_update_interval"].as<uint32_t>();
        if (interval < 1000 || interval > 60000) {
            errorMessage = "PID update interval must be between 1000ms and 60000ms";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidUpdateInterval(interval);
    }
    if (doc["timing"].containsKey("connectivity_check_interval")) {
        uint32_t interval = doc["timing"]["connectivity_check_interval"].as<uint32_t>();
        if (interval < 60000 || interval > 3600000) {
            errorMessage = "Connectivity check interval must be between 60000ms and 3600000ms";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setConnectivityCheckInterval(interval);
    }
    if (doc["timing"].containsKey("pid_config_write_interval")) {
        uint32_t interval = doc["timing"]["pid_config_write_interval"].as<uint32_t>();
        if (interval < 60000 || interval > 3600000) {
            errorMessage = "PID config write interval must be between 60000ms and 3600000ms";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setPidConfigWriteInterval(interval);
    }
    if (doc["timing"].containsKey("wifi_connect_timeout")) {
        uint16_t timeout = doc["timing"]["wifi_connect_timeout"].as<uint16_t>();
        if (timeout < 10 || timeout > 600) {
            errorMessage = "WiFi connect timeout must be between 10s and 600s";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setWifiConnectTimeout(timeout);
    }
    if (doc["timing"].containsKey("max_reconnect_attempts")) {
        uint8_t attempts = doc["timing"]["max_reconnect_attempts"].as<uint8_t>();
        if (attempts < 1 || attempts > 100) {
            errorMessage = "Max reconnect attempts must be between 1 and 100";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setMaxReconnectAttempts(attempts);
    }
    if (doc["timing"].containsKey("system_watchdog_timeout")) {
        uint32_t timeout = doc["timing"]["system_watchdog_timeout"].as<uint32_t>();
        if (timeout < 60000 || timeout > 7200000) {
            errorMessage = "System watchdog timeout must be between 60000ms and 7200000ms";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setSystemWatchdogTimeout(timeout);
    }
    if (doc["timing"].containsKey("wifi_watchdog_timeout")) {
        uint32_t timeout = doc["timing"]["wifi_watchdog_timeout"].as<uint32_t>();
        if (timeout < 60000 || timeout > 7200000) {
            errorMessage = "WiFi watchdog timeout must be between 60000ms and 7200000ms";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setWifiWatchdogTimeout(timeout);
    }
    return true;
}

bool ConfigManager::validateAndApplyWebhookSettings(const JsonDocument& doc, String& errorMessage) {
    if (!doc.containsKey("webhook")) {
        return true;
    }

    if (doc["webhook"].containsKey("enabled")) {
        bool enabled = doc["webhook"]["enabled"].as<bool>();
        setWebhookEnabled(enabled);
    }

    if (doc["webhook"].containsKey("url")) {
        String url = doc["webhook"]["url"].as<String>();
        if (url.length() > 512) {
            errorMessage = "Webhook URL must be less than 512 characters";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setWebhookUrl(url);
    }

    if (doc["webhook"].containsKey("temp_low_threshold")) {
        float threshold = doc["webhook"]["temp_low_threshold"].as<float>();
        if (threshold < -20.0f || threshold > 50.0f) {
            errorMessage = "Webhook low temperature threshold must be between -20°C and 50°C";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setWebhookTempLowThreshold(threshold);
    }

    if (doc["webhook"].containsKey("temp_high_threshold")) {
        float threshold = doc["webhook"]["temp_high_threshold"].as<float>();
        if (threshold < -20.0f || threshold > 50.0f) {
            errorMessage = "Webhook high temperature threshold must be between -20°C and 50°C";
            LOG_W(TAG, "%s", errorMessage.c_str());
            return false;
        }
        setWebhookTempHighThreshold(threshold);
    }

    return true;
}

// Import settings from JSON
bool ConfigManager::setFromJson(const JsonDocument& doc) {
    String errorMessage;
    return setFromJson(doc, errorMessage);
}

// Enhanced setFromJson with error reporting
bool ConfigManager::setFromJson(const JsonDocument& doc, String& errorMessage) {
    LOG_I(TAG, "Importing configuration from JSON");
    if (!validateAndApplyNetworkSettings(doc, errorMessage)) return false;
    if (!validateAndApplyMQTTSettings(doc, errorMessage)) return false;
    if (!validateAndApplyKNXSettings(doc, errorMessage)) return false;
    if (!validateBME280Settings(doc, errorMessage)) return false;
    if (!validateAndApplyPIDSettings(doc, errorMessage)) return false;
    if (!validateAndApplyTimingSettings(doc, errorMessage)) return false;
    if (!validateAndApplyWebhookSettings(doc, errorMessage)) return false;
    LOG_I(TAG, "Configuration imported successfully");
    return true;
}


void ConfigManager::setLastRebootReason(const String& reason) {
    _preferences.begin("config", false);
    _preferences.putString("reboot_reason", reason);
    _preferences.end();
}

String ConfigManager::getLastRebootReason() {
    _preferences.begin("config", true);
    String reason = _preferences.getString("reboot_reason", "Unknown");
    _preferences.end();
    return reason;
}

void ConfigManager::setRebootCount(int count) {
    _preferences.begin("config", false);
    _preferences.putInt("reboot_count", count);
    _preferences.end();
}

int ConfigManager::getRebootCount() {
    _preferences.begin("config", true);
    int count = _preferences.getInt("reboot_count", 0);
    _preferences.end();
    return count;
}

void ConfigManager::setConsecutiveWatchdogReboots(int count) {
    _preferences.begin("config", false);
    _preferences.putInt("wdt_reboots", count);
    _preferences.end();
}

int ConfigManager::getConsecutiveWatchdogReboots() {
    _preferences.begin("config", true);
    int count = _preferences.getInt("wdt_reboots", 0);
    _preferences.end();
    return count;
}

void ConfigManager::setLastConnectedTime(unsigned long timestamp) {
    _preferences.begin("config", false);
    _preferences.putULong("last_conn_time", timestamp);
    _preferences.end();
}

bool ConfigManager::factoryReset() {
    LOG_I(TAG, "Performing factory reset - clearing all preferences");

    Preferences prefs;

    // Clear "thermostat" namespace (user configuration)
    if (prefs.begin("thermostat", false)) {
        prefs.clear();
        prefs.end();
        LOG_I(TAG, "Cleared 'thermostat' namespace");
    } else {
        LOG_E(TAG, "Failed to open 'thermostat' namespace for clearing");
        return false;
    }

    // Clear "config" namespace (diagnostic data)
    if (prefs.begin("config", false)) {
        prefs.clear();
        prefs.end();
        LOG_I(TAG, "Cleared 'config' namespace");
    } else {
        LOG_W(TAG, "Failed to open 'config' namespace for clearing");
    }

    // Clear "watchdog" namespace (recovery state)
    if (prefs.begin("watchdog", false)) {
        prefs.clear();
        prefs.end();
        LOG_I(TAG, "Cleared 'watchdog' namespace");
    } else {
        LOG_W(TAG, "Failed to open 'watchdog' namespace for clearing");
    }

    // Reinitialize with defaults
    LOG_I(TAG, "Reinitializing with default values");
    if (!begin()) {
        LOG_E(TAG, "Failed to reinitialize after factory reset");
        return false;
    }

    LOG_I(TAG, "Factory reset completed successfully");
    return true;
}