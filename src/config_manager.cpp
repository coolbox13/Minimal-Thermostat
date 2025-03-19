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
    return _preferences.getFloat("pid_kp", DEFAULT_KP);
}

// 1. Update the setter methods to limit precision directly

void ConfigManager::setPidKp(float kp) {
    // Round to 2 decimal places
    kp = roundf(kp * 100) / 100.0f;
    _preferences.putFloat("pid_kp", kp);
}

void ConfigManager::setPidKi(float ki) {
    // Round to 3 decimal places
    ki = roundf(ki * 1000) / 1000.0f;
    _preferences.putFloat("pid_ki", ki);
}

void ConfigManager::setPidKd(float kd) {
    // Round to 3 decimal places
    kd = roundf(kd * 1000) / 1000.0f;
    _preferences.putFloat("pid_kd", kd);
}

void ConfigManager::setSetpoint(float setpoint) {
    // Round to 1 decimal place
    setpoint = roundf(setpoint * 10) / 10.0f;
    _preferences.putFloat("setpoint", setpoint);
}

float ConfigManager::getPidKi() {
    return _preferences.getFloat("pid_ki", DEFAULT_KI);
}

float ConfigManager::getPidKd() {
    return _preferences.getFloat("pid_kd", DEFAULT_KD);
}

// Export all settings as JSON
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
    
    // Update PID values with limited precision
    doc["pid"]["kp"] = roundf(getPidKp() * 100) / 100.0f;
    doc["pid"]["ki"] = roundf(getPidKi() * 1000) / 1000.0f;
    doc["pid"]["kd"] = roundf(getPidKd() * 1000) / 1000.0f;
    doc["pid"]["setpoint"] = roundf(getSetpoint() * 10) / 10.0f;
    
    LOG_D(TAG, "Created JSON configuration");
}

// Import settings from JSON
bool ConfigManager::setFromJson(const JsonDocument& doc) {
    String errorMessage; // Unused in this version
    return setFromJson(doc, errorMessage);
}

// Enhanced setFromJson with error reporting
bool ConfigManager::setFromJson(const JsonDocument& doc, String& errorMessage) {
    LOG_I(TAG, "Importing configuration from JSON");
    
    // Network settings
    if (doc.containsKey("network")) {
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
            if (pass.length() > 0 && pass != "**********") {  // Only update if changed
                if (pass.length() > 64) {
                    errorMessage = "WiFi password too long (max 64 characters)";
                    LOG_W(TAG, "%s", errorMessage.c_str());
                    return false;
                }
                setWifiPassword(pass);
            }
        }
    }
    
    // MQTT settings
    if (doc.containsKey("mqtt")) {
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
    }
    
    // KNX settings
    if (doc.containsKey("knx")) {
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
    }
    
    // BME280 settings - just validate but don't store yet
    if (doc.containsKey("bme280")) {
        if (doc["bme280"].containsKey("address")) {
            String address = doc["bme280"]["address"].as<String>();
            if (address != "0x76" && address != "0x77") {
                errorMessage = "BME280 address must be 0x76 or 0x77";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            // BME280 address stored in config.h for now
        }
        
        if (doc["bme280"].containsKey("sda_pin")) {
            uint8_t pin = doc["bme280"]["sda_pin"].as<uint8_t>();
            if (pin > 39) {
                errorMessage = "BME280 SDA pin must be 0-39";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            // BME280 SDA pin stored in config.h for now
        }
        
        if (doc["bme280"].containsKey("scl_pin")) {
            uint8_t pin = doc["bme280"]["scl_pin"].as<uint8_t>();
            if (pin > 39) {
                errorMessage = "BME280 SCL pin must be 0-39";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            // BME280 SCL pin stored in config.h for now
        }
        
        if (doc["bme280"].containsKey("interval")) {
            uint16_t interval = doc["bme280"]["interval"].as<uint16_t>();
            if (interval < 1 || interval > 3600) {
                errorMessage = "BME280 interval must be 1-3600 seconds";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            // BME280 interval stored in config.h for now
        }
    }
    
    // PID settings
    if (doc.containsKey("pid")) {
        if (doc["pid"].containsKey("kp")) {
            float kp = doc["pid"]["kp"].as<float>();
            // Round to 2 decimal places to avoid excessive precision
            kp = roundf(kp * 100) / 100.0f;
            if (kp < 0) {
                errorMessage = "PID Kp must be >= 0";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            setPidKp(kp);
        }
        
        if (doc["pid"].containsKey("ki")) {
            float ki = doc["pid"]["ki"].as<float>();
            // Round to 3 decimal places for integral gain
            ki = roundf(ki * 1000) / 1000.0f;
            if (ki < 0) {
                errorMessage = "PID Ki must be >= 0";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            setPidKi(ki);
        }
        
        if (doc["pid"].containsKey("kd")) {
            float kd = doc["pid"]["kd"].as<float>();
            // Round to 3 decimal places for derivative gain
            kd = roundf(kd * 1000) / 1000.0f;
            if (kd < 0) {
                errorMessage = "PID Kd must be >= 0";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            setPidKd(kd);
        }
        
        if (doc["pid"].containsKey("setpoint")) {
            float setpoint = doc["pid"]["setpoint"].as<float>();
            // Round to 1 decimal place for setpoint
            setpoint = roundf(setpoint * 10) / 10.0f;
            if (setpoint < 5 || setpoint > 30) {
                errorMessage = "Temperature setpoint must be between 5°C and 30°C";
                LOG_W(TAG, "%s", errorMessage.c_str());
                return false;
            }
            setSetpoint(setpoint);
        }
    }
    
    LOG_I(TAG, "Configuration imported successfully");
    return true;
}

// Add this method to your ConfigManager class implementation

float ConfigManager::getSetpoint() {
    return _preferences.getFloat("setpoint", DEFAULT_SETPOINT);
}