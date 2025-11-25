#include "home_assistant.h"
#include "config.h"
#include "config_manager.h"
#include "serial_monitor.h"
#include "serial_redirect.h"

// Redirect Serial to CapturedSerial for web monitor
#define Serial CapturedSerial

// Define discovery topic prefix according to Home Assistant standard
#define HA_DISCOVERY_PREFIX "homeassistant"

// Constructor
HomeAssistant::HomeAssistant(PubSubClient& mqttClient, const char* nodeId) 
    : _mqttClient(mqttClient), _nodeId(nodeId) {
    
    // Set up availability topic
    _availabilityTopic = String("esp32_thermostat/status");
}

// Initialize Home Assistant auto discovery
void HomeAssistant::begin() {
    Serial.println("Initializing Home Assistant auto discovery...");
    
    // First ensure we're connected
    if (!_mqttClient.connected()) {
        Serial.println("MQTT client not connected, cannot initialize Home Assistant discovery");
        return;
    }
    
    // Register auto-discovery information
    registerEntities();
    
    // Set initial availability
    updateAvailability(true);
    
    Serial.println("Home Assistant auto discovery initialized");
}

// Register all entities for auto discovery
void HomeAssistant::registerEntities() {
    // Get current timestamp for debugging
    unsigned long now = millis();
    String timestamp = String(now);

    Serial.println("Registering entities with Home Assistant at time: " + timestamp);

    // Create origin info (for sensors - full keys)
    String originInfo = "{\"name\":\"ESP32-KNX-Thermostat\",\"sw_version\":\"" + String(FIRMWARE_VERSION) + "\",\"support_url\":\"https://github.com/yourusername/ESP32-KNX-Thermostat\"}";

    // Create device info JSON string using ABBREVIATED keys for climate entity
    // Using abbreviated keys: ids, mf, mdl, sw (instead of identifiers, manufacturer, model, sw_version)
    String deviceInfo = "{\"ids\":[\"" + String(_nodeId) + "\"],\"name\":\"ESP32 KNX Thermostat\",\"mf\":\"DIY\",\"mdl\":\"ESP32-KNX-Thermostat\",\"sw\":\"" + String(FIRMWARE_VERSION) + "\"}";
    
    // Temperature sensor with enhanced attributes
    String tempTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/temperature/config";
    String tempPayload = "{";
    tempPayload += "\"name\":\"Temperature\",";
    tempPayload += "\"unique_id\":\"" + String(_nodeId) + "_temperature\",";
    tempPayload += "\"device_class\":\"temperature\",";
    tempPayload += "\"state_topic\":\"esp32_thermostat/temperature\",";
    tempPayload += "\"unit_of_measurement\":\"°C\",";
    tempPayload += "\"value_template\":\"{{ value }}\",";
    tempPayload += "\"availability_topic\":\"esp32_thermostat/status\",";
    tempPayload += "\"state_class\":\"measurement\",";  // Add state_class for proper history
    tempPayload += "\"suggested_display_precision\":1,"; // Precision for display
    tempPayload += "\"origin\":" + originInfo + ",";
    tempPayload += "\"device\":" + deviceInfo;
    tempPayload += "}";
    
    bool tempSuccess = _mqttClient.publish(tempTopic.c_str(), tempPayload.c_str(), true);
    Serial.print("Published temperature config: ");
    Serial.println(tempSuccess ? "Success" : "FAILED");
    
    // Humidity sensor with enhanced attributes
    String humTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/humidity/config";
    String humPayload = "{";
    humPayload += "\"name\":\"Humidity\",";
    humPayload += "\"unique_id\":\"" + String(_nodeId) + "_humidity\",";
    humPayload += "\"device_class\":\"humidity\",";
    humPayload += "\"state_topic\":\"esp32_thermostat/humidity\",";
    humPayload += "\"unit_of_measurement\":\"%\",";
    humPayload += "\"value_template\":\"{{ value }}\",";
    humPayload += "\"availability_topic\":\"esp32_thermostat/status\",";
    humPayload += "\"state_class\":\"measurement\",";  // Add state_class for proper history
    humPayload += "\"suggested_display_precision\":1,"; // Precision for display
    humPayload += "\"origin\":" + originInfo + ",";
    humPayload += "\"device\":" + deviceInfo;
    humPayload += "}";
    
    bool humSuccess = _mqttClient.publish(humTopic.c_str(), humPayload.c_str(), true);
    Serial.print("Published humidity config: ");
    Serial.println(humSuccess ? "Success" : "FAILED");
    
    // Pressure sensor with enhanced attributes
    String presTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pressure/config";
    String presPayload = "{";
    presPayload += "\"name\":\"Pressure\",";
    presPayload += "\"unique_id\":\"" + _nodeId + "_pressure\",";
    presPayload += "\"device_class\":\"pressure\",";
    presPayload += "\"state_topic\":\"esp32_thermostat/pressure\",";
    presPayload += "\"unit_of_measurement\":\"hPa\",";
    presPayload += "\"value_template\":\"{{ value }}\",";
    presPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    presPayload += "\"state_class\":\"measurement\",";  // Add state_class for proper history
    presPayload += "\"suggested_display_precision\":1,"; // Precision for display
    presPayload += "\"origin\":" + originInfo + ",";
    presPayload += "\"device\":" + deviceInfo;
    presPayload += "}";
    
    bool presSuccess = _mqttClient.publish(presTopic.c_str(), presPayload.c_str(), true);
    Serial.print("Published pressure config: ");
    Serial.println(presSuccess ? "Success" : "FAILED");
    
    // Valve position sensor with enhanced attributes
    String valvePosTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/valve_position/config";
    String valuePosPayload = "{";
    valuePosPayload += "\"name\":\"Valve Position\",";
    valuePosPayload += "\"unique_id\":\"" + _nodeId + "_valve_position\",";
    valuePosPayload += "\"state_topic\":\"esp32_thermostat/valve/position\",";
    valuePosPayload += "\"unit_of_measurement\":\"%\",";
    valuePosPayload += "\"value_template\":\"{{ value }}\",";
    valuePosPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    valuePosPayload += "\"state_class\":\"measurement\",";  // Add state_class for proper history
    valuePosPayload += "\"icon\":\"mdi:valve\",";           // Add an appropriate icon
    valuePosPayload += "\"origin\":" + originInfo + ",";
    valuePosPayload += "\"device\":" + deviceInfo;
    valuePosPayload += "}";
    
    bool valvePosSuccess = _mqttClient.publish(valvePosTopic.c_str(), valuePosPayload.c_str(), true);
    Serial.print("Published valve position config: ");
    Serial.println(valvePosSuccess ? "Success" : "FAILED");

    // ========================================================================
    // CRITICAL: Publish ALL climate state topics BEFORE discovery config
    // This ensures Home Assistant finds retained messages when it subscribes
    // ========================================================================

    Serial.println("\n=== Publishing Climate State Topics (BEFORE Discovery) ===");

    // 1. Publish mode state
    bool modeSuccess = _mqttClient.publish("esp32_thermostat/mode/state", "heat", true);
    Serial.print("  [1/4] Mode state: heat - ");
    Serial.println(modeSuccess ? "OK" : "FAILED");

    // 2. Publish initial setpoint from PID config
    char setpointStr[8];
    dtostrf(PID_SETPOINT, 1, 1, setpointStr);
    bool setpointSuccess = _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
    Serial.print("  [2/4] Setpoint: ");
    Serial.print(setpointStr);
    Serial.print("°C - ");
    Serial.println(setpointSuccess ? "OK" : "FAILED");

    // 3. Publish initial preset state with validation
    extern ConfigManager* configManager;
    String currentPreset = "none";  // Default fallback

    if (configManager) {
        currentPreset = configManager->getCurrentPreset();
        Serial.print("  [3/4] Preset from ConfigManager: '");
        Serial.print(currentPreset);
        Serial.println("'");

        // Validate preset is in the allowed list
        const char* validPresets[] = {"none", "eco", "comfort", "away", "sleep", "boost"};
        bool isValid = false;
        for (int i = 0; i < 6; i++) {
            if (currentPreset.equals(validPresets[i])) {
                isValid = true;
                break;
            }
        }

        if (!isValid) {
            Serial.print("        WARNING: Invalid preset '");
            Serial.print(currentPreset);
            Serial.println("' - resetting to 'none'");
            currentPreset = "none";
            configManager->setCurrentPreset("none");
        }
    } else {
        Serial.println("  [3/4] WARNING: ConfigManager is NULL - using default 'none'");
    }

    bool presetSuccess = _mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
    Serial.print("        Preset state: '");
    Serial.print(currentPreset);
    Serial.print("' - ");
    Serial.println(presetSuccess ? "OK" : "FAILED");

    // 4. Publish initial action state (idle at startup)
    bool actionSuccess = _mqttClient.publish("esp32_thermostat/action", "idle", true);
    Serial.print("  [4/4] Action state: idle - ");
    Serial.println(actionSuccess ? "OK" : "FAILED");

    // Note: No delay needed - MQTT broker will process retained messages asynchronously
    Serial.println("  Waiting for broker to process retained messages...");

    Serial.println("=== All State Topics Published Successfully ===\n");

    // ========================================================================
    // NOW publish the climate discovery config
    // Home Assistant will immediately subscribe and find the retained states
    // ========================================================================

    Serial.println("=== Publishing Climate Discovery Config ===");

    String climateTopic = String(HA_DISCOVERY_PREFIX) + "/climate/" + _nodeId + "/config";
    String climatePayload = "{";
    climatePayload += "\"name\":\"Thermostat\",";
    climatePayload += "\"uniq_id\":\"" + String(_nodeId) + "_climate\",";
    climatePayload += "\"dev\":" + deviceInfo + ",";
    // Mode control (abbreviated)
    climatePayload += "\"mode_cmd_t\":\"" + String(_nodeId) + "/mode/set\",";
    climatePayload += "\"mode_stat_t\":\"" + String(_nodeId) + "/mode/state\",";
    climatePayload += "\"modes\":[\"off\",\"heat\"],";
    // Temperature control (abbreviated)
    climatePayload += "\"temp_cmd_t\":\"" + String(_nodeId) + "/temperature/set\",";
    climatePayload += "\"temp_stat_t\":\"" + String(_nodeId) + "/temperature/setpoint\",";
    climatePayload += "\"curr_temp_t\":\"" + String(_nodeId) + "/temperature\",";
    climatePayload += "\"min_temp\":15,";
    climatePayload += "\"max_temp\":30,";
    climatePayload += "\"temp_step\":0.5,";
    climatePayload += "\"temp_unit\":\"C\",";
    // Preset modes (abbreviated) - verified working with HA
    climatePayload += "\"pr_mode_cmd_t\":\"" + String(_nodeId) + "/preset/set\",";
    climatePayload += "\"pr_mode_stat_t\":\"" + String(_nodeId) + "/preset/state\",";
    // HA FIX #3: Include "none" in preset modes so users can clear preset via HA
    climatePayload += "\"pr_modes\":[\"none\",\"eco\",\"comfort\",\"away\",\"sleep\",\"boost\"],";
    // Availability (abbreviated)
    climatePayload += "\"avty_t\":\"" + _availabilityTopic + "\",";
    climatePayload += "\"pl_avail\":\"online\",";
    climatePayload += "\"pl_not_avail\":\"offline\",";
    // Action (abbreviated)
    climatePayload += "\"act_t\":\"" + String(_nodeId) + "/action\",";
    climatePayload += "\"qos\":0,";
    climatePayload += "\"ret\":true";
    climatePayload += "}";

    Serial.print("  Discovery payload size: ");
    Serial.print(climatePayload.length());
    Serial.println(" bytes");

    bool climateSuccess = _mqttClient.publish(climateTopic.c_str(), climatePayload.c_str(), true);
    Serial.print("  Published to: ");
    Serial.println(climateTopic);
    Serial.print("  Result: ");
    Serial.println(climateSuccess ? "SUCCESS" : "FAILED");

    if (!climateSuccess) {
        Serial.println("  ERROR: Climate discovery publish failed!");
        Serial.println("  Check MQTT buffer size and broker connection");
    }

    Serial.println("=== Climate Discovery Complete ===\n");

    // Subscribe to the thermostat control topics
    _mqttClient.subscribe("esp32_thermostat/mode/set");
    _mqttClient.subscribe("esp32_thermostat/temperature/set");
    _mqttClient.subscribe("esp32_thermostat/preset/set");
    Serial.println("Subscribed to thermostat control topics");

    // Re-publish states one more time to ensure HA picks them up
    // (Some sources suggest this helps with initialization)
    // Note: No delay needed - publishing happens immediately
    Serial.println("\n=== Re-publishing States After Discovery ===");
    _mqttClient.publish("esp32_thermostat/mode/state", "heat", true);
    _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
    _mqttClient.publish("esp32_thermostat/preset/state", currentPreset.c_str(), true);
    _mqttClient.publish("esp32_thermostat/action", "idle", true);
    Serial.println("  All states re-published");
    Serial.println("=== Climate Entity Initialization Complete ===\n");

    // Add restart command option
    _mqttClient.subscribe("esp32_thermostat/restart");

    // Binary sensor for heating status
    String heatingTopic = String(HA_DISCOVERY_PREFIX) + "/binary_sensor/" + _nodeId + "/heating/config";
    String heatingPayload = "{";
    heatingPayload += "\"name\":\"Heating Status\",";
    heatingPayload += "\"unique_id\":\"" + String(_nodeId) + "_heating_status\",";
    heatingPayload += "\"device_class\":\"running\",";
    heatingPayload += "\"state_topic\":\"esp32_thermostat/heating/state\",";
    heatingPayload += "\"payload_on\":\"ON\",";
    heatingPayload += "\"payload_off\":\"OFF\",";
    heatingPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    heatingPayload += "\"icon\":\"mdi:radiator\",";
    heatingPayload += "\"origin\":" + originInfo + ",";
    heatingPayload += "\"device\":" + deviceInfo;
    heatingPayload += "}";
    bool heatingSuccess = _mqttClient.publish(heatingTopic.c_str(), heatingPayload.c_str(), true);
    Serial.print("Published heating status config: ");
    Serial.println(heatingSuccess ? "Success" : "FAILED");

    // PID Kp parameter sensor
    String kpTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pid_kp/config";
    String kpPayload = "{";
    kpPayload += "\"name\":\"PID Kp\",";
    kpPayload += "\"unique_id\":\"" + String(_nodeId) + "_pid_kp\",";
    kpPayload += "\"state_topic\":\"esp32_thermostat/pid/kp\",";
    kpPayload += "\"value_template\":\"{{ value }}\",";
    kpPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    kpPayload += "\"icon\":\"mdi:chart-bell-curve\",";
    kpPayload += "\"origin\":" + originInfo + ",";
    kpPayload += "\"device\":" + deviceInfo;
    kpPayload += "}";
    bool kpSuccess = _mqttClient.publish(kpTopic.c_str(), kpPayload.c_str(), true);
    Serial.print("Published PID Kp config: ");
    Serial.println(kpSuccess ? "Success" : "FAILED");

    // PID Ki parameter sensor
    String kiTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pid_ki/config";
    String kiPayload = "{";
    kiPayload += "\"name\":\"PID Ki\",";
    kiPayload += "\"unique_id\":\"" + String(_nodeId) + "_pid_ki\",";
    kiPayload += "\"state_topic\":\"esp32_thermostat/pid/ki\",";
    kiPayload += "\"value_template\":\"{{ value }}\",";
    kiPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    kiPayload += "\"icon\":\"mdi:chart-bell-curve\",";
    kiPayload += "\"origin\":" + originInfo + ",";
    kiPayload += "\"device\":" + deviceInfo;
    kiPayload += "}";
    bool kiSuccess = _mqttClient.publish(kiTopic.c_str(), kiPayload.c_str(), true);
    Serial.print("Published PID Ki config: ");
    Serial.println(kiSuccess ? "Success" : "FAILED");

    // PID Kd parameter sensor
    String kdTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/pid_kd/config";
    String kdPayload = "{";
    kdPayload += "\"name\":\"PID Kd\",";
    kdPayload += "\"unique_id\":\"" + String(_nodeId) + "_pid_kd\",";
    kdPayload += "\"state_topic\":\"esp32_thermostat/pid/kd\",";
    kdPayload += "\"value_template\":\"{{ value }}\",";
    kdPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    kdPayload += "\"icon\":\"mdi:chart-bell-curve\",";
    kdPayload += "\"origin\":" + originInfo + ",";
    kdPayload += "\"device\":" + deviceInfo;
    kdPayload += "}";
    bool kdSuccess = _mqttClient.publish(kdTopic.c_str(), kdPayload.c_str(), true);
    Serial.print("Published PID Kd config: ");
    Serial.println(kdSuccess ? "Success" : "FAILED");

    // WiFi signal strength sensor
    String wifiTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/wifi_signal/config";
    String wifiPayload = "{";
    wifiPayload += "\"name\":\"WiFi Signal\",";
    wifiPayload += "\"unique_id\":\"" + String(_nodeId) + "_wifi_signal\",";
    wifiPayload += "\"device_class\":\"signal_strength\",";
    wifiPayload += "\"state_topic\":\"esp32_thermostat/wifi/rssi\",";
    wifiPayload += "\"unit_of_measurement\":\"dBm\",";
    wifiPayload += "\"value_template\":\"{{ value }}\",";
    wifiPayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    wifiPayload += "\"state_class\":\"measurement\",";
    wifiPayload += "\"icon\":\"mdi:wifi\",";
    wifiPayload += "\"origin\":" + originInfo + ",";
    wifiPayload += "\"device\":" + deviceInfo;
    wifiPayload += "}";
    bool wifiSuccess = _mqttClient.publish(wifiTopic.c_str(), wifiPayload.c_str(), true);
    Serial.print("Published WiFi signal config: ");
    Serial.println(wifiSuccess ? "Success" : "FAILED");

    // Uptime sensor
    String uptimeTopic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + _nodeId + "/uptime/config";
    String uptimePayload = "{";
    uptimePayload += "\"name\":\"Uptime\",";
    uptimePayload += "\"unique_id\":\"" + String(_nodeId) + "_uptime\",";
    uptimePayload += "\"device_class\":\"duration\",";
    uptimePayload += "\"state_topic\":\"esp32_thermostat/uptime\",";
    uptimePayload += "\"unit_of_measurement\":\"s\",";
    uptimePayload += "\"value_template\":\"{{ value }}\",";
    uptimePayload += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    uptimePayload += "\"state_class\":\"total_increasing\",";
    uptimePayload += "\"icon\":\"mdi:clock-outline\",";
    uptimePayload += "\"origin\":" + originInfo + ",";
    uptimePayload += "\"device\":" + deviceInfo;
    uptimePayload += "}";
    bool uptimeSuccess = _mqttClient.publish(uptimeTopic.c_str(), uptimePayload.c_str(), true);
    Serial.print("Published uptime config: ");
    Serial.println(uptimeSuccess ? "Success" : "FAILED");
}

// Send state updates for each entity
void HomeAssistant::updateStates(float temperature, float humidity, float pressure, int valvePosition) {
    // Convert values to strings and publish
    char tempStr[8];
    dtostrf(temperature, 1, 2, tempStr);
    _mqttClient.publish("esp32_thermostat/temperature", tempStr);
    
    char humStr[8];
    dtostrf(humidity, 1, 2, humStr);
    _mqttClient.publish("esp32_thermostat/humidity", humStr);
    
    char presStr[8];
    dtostrf(pressure, 1, 2, presStr);
    _mqttClient.publish("esp32_thermostat/pressure", presStr);
    
    // For the valve position
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    _mqttClient.publish("esp32_thermostat/valve/position", valveStr);
    
    // HA FIX #2: Update action state based on mode AND valve position
    // When mode is "off", action should be "off", not "idle"
    extern ConfigManager* configManager;
    const char* action;
    if (configManager && !configManager->getThermostatEnabled()) {
        action = "off";
    } else if (valvePosition > 0) {
        action = "heating";
    } else {
        action = "idle";
    }
    _mqttClient.publish("esp32_thermostat/action", action);
    _mqttClient.publish("esp32_thermostat/heating/state", (valvePosition > 0) ? "ON" : "OFF");

    // Also publish a general "online" status message
    _mqttClient.publish(_availabilityTopic.c_str(), "online", true);
}

// Update PID parameters
void HomeAssistant::updatePIDParameters(float kp, float ki, float kd) {
    char kpStr[10];
    dtostrf(kp, 1, 2, kpStr);
    _mqttClient.publish("esp32_thermostat/pid/kp", kpStr);

    char kiStr[10];
    dtostrf(ki, 1, 3, kiStr);
    _mqttClient.publish("esp32_thermostat/pid/ki", kiStr);

    char kdStr[10];
    dtostrf(kd, 1, 3, kdStr);
    _mqttClient.publish("esp32_thermostat/pid/kd", kdStr);
}

// Update system diagnostics
void HomeAssistant::updateDiagnostics(int wifiRSSI, unsigned long uptime) {
    char rssiStr[8];
    itoa(wifiRSSI, rssiStr, 10);
    _mqttClient.publish("esp32_thermostat/wifi/rssi", rssiStr);

    char uptimeStr[16];
    ultoa(uptime / 1000, uptimeStr, 10); // Convert ms to seconds
    _mqttClient.publish("esp32_thermostat/uptime", uptimeStr);
}

// Update manual valve override status
void HomeAssistant::updateManualOverride(bool enabled, int position) {
    // Publish override enabled status
    _mqttClient.publish("esp32_thermostat/manual_override/enabled", enabled ? "ON" : "OFF");

    // Publish override position
    char posStr[8];
    itoa(position, posStr, 10);
    _mqttClient.publish("esp32_thermostat/manual_override/position", posStr);
}

// Update availability status
void HomeAssistant::updateAvailability(bool isOnline) {
    bool published = _mqttClient.publish(_availabilityTopic.c_str(), isOnline ? "online" : "offline", true);
    Serial.print("Published availability status (");
    Serial.print(isOnline ? "online" : "offline");
    Serial.print(") to ");
    Serial.print(_availabilityTopic);
    Serial.println(published ? " - Success" : " - FAILED");
}

// Publish discovery configuration for a single entity - kept for backward compatibility
void HomeAssistant::publishConfig(const char* component, const char* objectId, const char* name, 
                               const char* deviceClass, const char* stateTopic, const char* unit,
                               const char* commandTopic) {
    // This method is no longer used but kept for compatibility
    Serial.println("Warning: Obsolete publishConfig method called");
}

// NEW: Update the setpoint temperature
void HomeAssistant::updateSetpointTemperature(float setpoint) {
    char setpointStr[8];
    dtostrf(setpoint, 1, 1, setpointStr);
    _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
}

// NEW: Update the thermostat mode
void HomeAssistant::updateMode(const char* mode) {
    _mqttClient.publish("esp32_thermostat/mode/state", mode, true);
}

// NEW: Update the preset mode
void HomeAssistant::updatePresetMode(const char* preset) {
    _mqttClient.publish("esp32_thermostat/preset/state", preset, true);
}

// HA FIX #5: Sync all climate state to HA
// This ensures HA stays in sync even if mode/preset/setpoint changes via web interface
void HomeAssistant::syncClimateState() {
    extern ConfigManager* configManager;
    if (!configManager) return;

    // Sync mode state
    const char* mode = configManager->getThermostatEnabled() ? "heat" : "off";
    _mqttClient.publish("esp32_thermostat/mode/state", mode, true);

    // Sync preset state
    String preset = configManager->getCurrentPreset();
    _mqttClient.publish("esp32_thermostat/preset/state", preset.c_str(), true);

    // Sync setpoint
    char setpointStr[8];
    dtostrf(configManager->getSetpoint(), 1, 1, setpointStr);
    _mqttClient.publish("esp32_thermostat/temperature/setpoint", setpointStr, true);
}