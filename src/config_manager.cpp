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
    return _preferences.begin("thermostat", false);
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

void ConfigManager::setPidKp(float kp) {
    _preferences.putFloat("pid_kp", kp);
}

float ConfigManager::getPidKi() {
    return _preferences.getFloat("pid_ki", DEFAULT_KI);
}

void ConfigManager::setPidKi(float ki) {
    _preferences.putFloat("pid_ki", ki);
}

float ConfigManager::getPidKd() {
    return _preferences.getFloat("pid_kd", DEFAULT_KD);
}

void ConfigManager::setPidKd(float kd) {
    _preferences.putFloat("pid_kd", kd);
}

float ConfigManager::getSetpoint() {
    return _preferences.getFloat("setpoint", DEFAULT_SETPOINT);
}

void ConfigManager::setSetpoint(float setpoint) {
    _preferences.putFloat("setpoint", setpoint);
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
    
    doc["pid"]["kp"] = getPidKp();
    doc["pid"]["ki"] = getPidKi();
    doc["pid"]["kd"] = getPidKd();
    doc["pid"]["setpoint"] = getSetpoint();
    
    LOG_D(TAG, "Created JSON configuration");
}

// Import settings from JSON
bool ConfigManager::setFromJson(const JsonDocument& doc) {
    bool success = true;
    LOG_I(TAG, "Importing configuration from JSON");
    
    // Network settings
    if (doc.containsKey("network")) {
        if (doc["network"].containsKey("wifi_ssid")) {
            setWifiSSID(doc["network"]["wifi_ssid"].as<String>());
        }
        if (doc["network"].containsKey("wifi_pass")) {
            setWifiPassword(doc["network"]["wifi_pass"].as<String>());
        }
    }
    
    // MQTT settings
    if (doc.containsKey("mqtt")) {
        if (doc["mqtt"].containsKey("server")) {
            setMqttServer(doc["mqtt"]["server"].as<String>());
        }
        if (doc["mqtt"].containsKey("port")) {
            setMqttPort(doc["mqtt"]["port"].as<uint16_t>());
        }
    }
    
    // KNX settings
    if (doc.containsKey("knx")) {
        if (doc["knx"].containsKey("area")) {
            setKnxArea(doc["knx"]["area"].as<uint8_t>());
        }
        if (doc["knx"].containsKey("line")) {
            setKnxLine(doc["knx"]["line"].as<uint8_t>());
        }
        if (doc["knx"].containsKey("member")) {
            setKnxMember(doc["knx"]["member"].as<uint8_t>());
        }
        if (doc["knx"].containsKey("use_test")) {
            setUseTestAddresses(doc["knx"]["use_test"].as<bool>());
        }
    }
    
    // PID settings
    if (doc.containsKey("pid")) {
        if (doc["pid"].containsKey("kp")) {
            setPidKp(doc["pid"]["kp"].as<float>());
        }
        if (doc["pid"].containsKey("ki")) {
            setPidKi(doc["pid"]["ki"].as<float>());
        }
        if (doc["pid"].containsKey("kd")) {
            setPidKd(doc["pid"]["kd"].as<float>());
        }
        if (doc["pid"].containsKey("setpoint")) {
            setSetpoint(doc["pid"]["setpoint"].as<float>());
        }
    }
    
    LOG_I(TAG, "Configuration imported successfully");
    return success;
}