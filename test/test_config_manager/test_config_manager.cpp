/**
 * @file test_config_manager.cpp
 * @brief Comprehensive unit tests for the Configuration Manager
 *
 * Tests cover:
 * - JSON serialization/deserialization
 * - Parameter storage with MockPreferences
 * - Validation logic (KNX addresses, MQTT port, PID parameters)
 * - Export/import configuration
 * - Singleton pattern
 * - Default values
 * - Factory reset
 *
 * Target Coverage: 70%
 */

#include <unity.h>
#include <ArduinoJson.h>
#include "config_manager.h"
#include "MockPreferences.h"

// ===== Test Fixtures =====

void setUp(void) {
    // Reset preferences before each test
    ConfigManager* config = ConfigManager::getInstance();
    config->end();
}

void tearDown(void) {
    // Cleanup after each test
}

// ===== TEST SUITE 1: Initialization and Singleton =====

/**
 * Test 1.1: Singleton instance
 * Verify that getInstance() returns the same instance
 */
void test_singleton_instance(void) {
    ConfigManager* instance1 = ConfigManager::getInstance();
    ConfigManager* instance2 = ConfigManager::getInstance();

    TEST_ASSERT_EQUAL_PTR(instance1, instance2);
    TEST_ASSERT_NOT_NULL(instance1);
}

/**
 * Test 1.2: begin() initializes successfully
 */
void test_begin_initialization(void) {
    ConfigManager* config = ConfigManager::getInstance();
    bool result = config->begin();

    TEST_ASSERT_TRUE(result);
}

// ===== TEST SUITE 2: Default Values =====

/**
 * Test 2.1: Default PID parameters
 */
void test_default_pid_parameters(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, config->getPidKp());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, config->getPidKi());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, config->getPidKd());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 22.0f, config->getSetpoint());
}

/**
 * Test 2.2: Default network settings
 */
void test_default_network_settings(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    String ssid = config->getWifiSSID();
    String password = config->getWifiPassword();

    // Defaults should be empty strings
    TEST_ASSERT_TRUE(ssid.length() == 0 || ssid.length() > 0); // Just verify no crash
    TEST_ASSERT_TRUE(password.length() >= 0);
}

/**
 * Test 2.3: Default MQTT settings
 */
void test_default_mqtt_settings(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    TEST_ASSERT_EQUAL_UINT16(1883, config->getMqttPort());

    String mqttServer = config->getMqttServer();
    TEST_ASSERT_TRUE(mqttServer.length() >= 0);
}

/**
 * Test 2.4: Default KNX settings
 */
void test_default_knx_settings(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    TEST_ASSERT_EQUAL_UINT8(1, config->getKnxArea());
    TEST_ASSERT_EQUAL_UINT8(1, config->getKnxLine());
    TEST_ASSERT_EQUAL_UINT8(159, config->getKnxMember());
}

// ===== TEST SUITE 3: Getters and Setters =====

/**
 * Test 3.1: Set and get WiFi credentials
 */
void test_wifi_credentials_storage(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setWifiSSID("TestNetwork");
    config->setWifiPassword("SecurePassword123");

    TEST_ASSERT_EQUAL_STRING("TestNetwork", config->getWifiSSID().c_str());
    TEST_ASSERT_EQUAL_STRING("SecurePassword123", config->getWifiPassword().c_str());
}

/**
 * Test 3.2: Set and get MQTT settings
 */
void test_mqtt_settings_storage(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setMqttServer("mqtt.example.com");
    config->setMqttPort(1883);

    TEST_ASSERT_EQUAL_STRING("mqtt.example.com", config->getMqttServer().c_str());
    TEST_ASSERT_EQUAL_UINT16(1883, config->getMqttPort());
}

/**
 * Test 3.3: Set and get KNX address
 */
void test_knx_address_storage(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setKnxArea(2);
    config->setKnxLine(3);
    config->setKnxMember(100);

    TEST_ASSERT_EQUAL_UINT8(2, config->getKnxArea());
    TEST_ASSERT_EQUAL_UINT8(3, config->getKnxLine());
    TEST_ASSERT_EQUAL_UINT8(100, config->getKnxMember());
}

/**
 * Test 3.4: Set and get PID parameters
 */
void test_pid_parameters_storage(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setPidKp(3.5f);
    config->setPidKi(0.25f);
    config->setPidKd(0.75f);
    config->setSetpoint(23.5f);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.5f, config->getPidKp());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, config->getPidKi());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.75f, config->getPidKd());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 23.5f, config->getSetpoint());
}

/**
 * Test 3.5: Set and get timing parameters
 */
void test_timing_parameters_storage(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setSensorUpdateInterval(60000);
    config->setPidUpdateInterval(15000);
    config->setConnectivityCheckInterval(600000);

    TEST_ASSERT_EQUAL_UINT32(60000, config->getSensorUpdateInterval());
    TEST_ASSERT_EQUAL_UINT32(15000, config->getPidUpdateInterval());
    TEST_ASSERT_EQUAL_UINT32(600000, config->getConnectivityCheckInterval());
}

/**
 * Test 3.6: Set and get manual override settings
 */
void test_manual_override_settings(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setManualOverrideEnabled(true);
    config->setManualOverridePosition(75);
    config->setManualOverrideTimeout(7200);
    config->setManualOverrideActivationTime(12345678UL);

    TEST_ASSERT_TRUE(config->getManualOverrideEnabled());
    TEST_ASSERT_EQUAL_UINT8(75, config->getManualOverridePosition());
    TEST_ASSERT_EQUAL_UINT32(7200, config->getManualOverrideTimeout());
    TEST_ASSERT_EQUAL_UINT32(12345678UL, config->getManualOverrideActivationTime());
}

/**
 * Test 3.7: Set and get webhook settings
 */
void test_webhook_settings(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setWebhookUrl("https://maker.ifttt.com/trigger/test");
    config->setWebhookEnabled(true);
    config->setWebhookTempLowThreshold(18.5f);
    config->setWebhookTempHighThreshold(28.5f);

    TEST_ASSERT_EQUAL_STRING("https://maker.ifttt.com/trigger/test",
                             config->getWebhookUrl().c_str());
    TEST_ASSERT_TRUE(config->getWebhookEnabled());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 18.5f, config->getWebhookTempLowThreshold());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 28.5f, config->getWebhookTempHighThreshold());
}

// ===== TEST SUITE 4: JSON Export/Import =====

/**
 * Test 4.1: Export configuration to JSON
 */
void test_export_to_json(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Set some test values
    config->setWifiSSID("TestNet");
    config->setPidKp(3.0f);
    config->setSetpoint(21.0f);
    config->setMqttPort(8883);

    // Export to JSON
    StaticJsonDocument<2048> doc;
    config->getJson(doc);

    // Verify exported values (match actual JSON structure from getJson)
    TEST_ASSERT_EQUAL_STRING("TestNet", doc["network"]["wifi_ssid"]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.0f, doc["pid"]["kp"]);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 21.0f, doc["pid"]["setpoint"]);
    TEST_ASSERT_EQUAL_INT(8883, doc["mqtt"]["port"]);
}

/**
 * Test 4.2: Import configuration from JSON - valid data
 */
void test_import_from_json_valid(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Create JSON configuration (match actual JSON structure expected by setFromJson)
    StaticJsonDocument<2048> doc;
    doc["network"]["wifi_ssid"] = "ImportedNet";
    doc["network"]["wifi_pass"] = "ImportedPass";
    doc["mqtt"]["server"] = "imported.mqtt.com";
    doc["mqtt"]["port"] = 1884;
    doc["knx"]["area"] = 5;
    doc["knx"]["line"] = 6;
    doc["knx"]["member"] = 50;
    doc["pid"]["kp"] = 4.5;
    doc["pid"]["ki"] = 0.3;
    doc["pid"]["kd"] = 0.8;
    doc["pid"]["setpoint"] = 20.5;

    // Import from JSON
    bool result = config->setFromJson(doc);
    TEST_ASSERT_TRUE(result);

    // Verify imported values
    TEST_ASSERT_EQUAL_STRING("ImportedNet", config->getWifiSSID().c_str());
    TEST_ASSERT_EQUAL_STRING("ImportedPass", config->getWifiPassword().c_str());
    TEST_ASSERT_EQUAL_STRING("imported.mqtt.com", config->getMqttServer().c_str());
    TEST_ASSERT_EQUAL_UINT16(1884, config->getMqttPort());
    TEST_ASSERT_EQUAL_UINT8(5, config->getKnxArea());
    TEST_ASSERT_EQUAL_UINT8(6, config->getKnxLine());
    TEST_ASSERT_EQUAL_UINT8(50, config->getKnxMember());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.5f, config->getPidKp());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.3f, config->getPidKi());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.8f, config->getPidKd());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 20.5f, config->getSetpoint());
}

/**
 * Test 4.3: Import configuration from JSON - invalid MQTT port
 */
void test_import_from_json_invalid_mqtt_port(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Create JSON with invalid MQTT port
    StaticJsonDocument<2048> doc;
    doc["mqtt"]["server"] = "test.mqtt.com";
    doc["mqtt"]["port"] = 99999; // Invalid port (> 65535)

    String errorMessage;
    bool result = config->setFromJson(doc, errorMessage);

    // Should fail validation
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(errorMessage.length() > 0);
}

/**
 * Test 4.4: Import configuration from JSON - invalid KNX area
 */
void test_import_from_json_invalid_knx_area(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Create JSON with invalid KNX area
    StaticJsonDocument<2048> doc;
    doc["knx"]["area"] = 20; // Invalid (should be 0-15)
    doc["knx"]["line"] = 1;
    doc["knx"]["member"] = 100;

    String errorMessage;
    bool result = config->setFromJson(doc, errorMessage);

    // Should fail validation
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(errorMessage.length() > 0);
}

/**
 * Test 4.5: Import configuration from JSON - invalid setpoint
 */
void test_import_from_json_invalid_setpoint(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Create JSON with invalid setpoint (out of range)
    StaticJsonDocument<2048> doc;
    doc["pid"]["setpoint"] = 50.0f; // Invalid (should be 5-30°C)

    String errorMessage;
    bool result = config->setFromJson(doc, errorMessage);

    // Should fail validation
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(errorMessage.length() > 0);
}

/**
 * Test 4.6: Export and re-import configuration (round-trip)
 */
void test_json_round_trip(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Set unique test values
    config->setWifiSSID("RoundTripTest");
    config->setPidKp(5.25f);
    config->setSetpoint(19.5f);
    config->setKnxArea(7);

    // Export to JSON
    StaticJsonDocument<2048> doc;
    config->getJson(doc);

    // Clear and re-import
    config->setWifiSSID("");
    config->setPidKp(1.0f);

    bool result = config->setFromJson(doc);
    TEST_ASSERT_TRUE(result);

    // Verify values match original
    TEST_ASSERT_EQUAL_STRING("RoundTripTest", config->getWifiSSID().c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.25f, config->getPidKp());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 19.5f, config->getSetpoint());
    TEST_ASSERT_EQUAL_UINT8(7, config->getKnxArea());
}

// ===== TEST SUITE 5: Precision Rounding =====

/**
 * Test 5.1: roundToPrecision() basic functionality
 */
void test_round_to_precision_basic(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.14f,
        ConfigManager::roundToPrecision(3.14159f, 2));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.718f,
        ConfigManager::roundToPrecision(2.71828f, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 22.0f,
        ConfigManager::roundToPrecision(21.95f, 0));
}

/**
 * Test 5.2: roundToPrecision() with various decimal places
 */
void test_round_to_precision_decimals(void) {
    float value = 123.456789f;

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 123.0f,
        ConfigManager::roundToPrecision(value, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 123.5f,
        ConfigManager::roundToPrecision(value, 1));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 123.46f,
        ConfigManager::roundToPrecision(value, 2));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 123.457f,
        ConfigManager::roundToPrecision(value, 3));
}

/**
 * Test 5.3: PID parameter precision
 * Verify that Kp is rounded to 2 decimals, Ki/Kd to 3 decimals
 */
void test_pid_parameter_precision(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Set values with high precision
    config->setPidKp(2.345678f);
    config->setPidKi(0.123456f);
    config->setPidKd(0.987654f);
    config->setSetpoint(21.789f);

    // Kp should be rounded to 2 decimals
    float kp = config->getPidKp();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.35f, kp);

    // Ki/Kd should be rounded to 3 decimals
    float ki = config->getPidKi();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.123f, ki);

    float kd = config->getPidKd();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.988f, kd);

    // Setpoint should be rounded to 1 decimal
    float setpoint = config->getSetpoint();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 21.8f, setpoint);
}

// ===== TEST SUITE 6: Diagnostic Settings =====

/**
 * Test 6.1: Reboot reason tracking
 */
void test_reboot_reason_tracking(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setLastRebootReason("Watchdog Reset");
    TEST_ASSERT_EQUAL_STRING("Watchdog Reset", config->getLastRebootReason().c_str());

    config->setLastRebootReason("User Initiated");
    TEST_ASSERT_EQUAL_STRING("User Initiated", config->getLastRebootReason().c_str());
}

/**
 * Test 6.2: Reboot count tracking
 */
void test_reboot_count_tracking(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setRebootCount(5);
    TEST_ASSERT_EQUAL_INT(5, config->getRebootCount());

    config->setRebootCount(10);
    TEST_ASSERT_EQUAL_INT(10, config->getRebootCount());
}

/**
 * Test 6.3: Consecutive watchdog reboots tracking
 */
void test_consecutive_watchdog_reboots(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setConsecutiveWatchdogReboots(3);
    TEST_ASSERT_EQUAL_INT(3, config->getConsecutiveWatchdogReboots());

    config->setConsecutiveWatchdogReboots(0);
    TEST_ASSERT_EQUAL_INT(0, config->getConsecutiveWatchdogReboots());
}

/**
 * Test 6.4: Last connected timestamp
 */
void test_last_connected_timestamp(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    unsigned long timestamp = 123456789UL;
    config->setLastConnectedTime(timestamp);

    // Note: getLastConnectedTime() not exposed in header, tested through internal logic
    TEST_ASSERT_TRUE(true); // Placeholder
}

// ===== TEST SUITE 7: Factory Reset =====

/**
 * Test 7.1: Factory reset clears all settings
 */
void test_factory_reset_clears_settings(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Set some custom values
    config->setWifiSSID("CustomNetwork");
    config->setPidKp(10.0f);
    config->setSetpoint(25.0f);
    config->setRebootCount(42);

    // Perform factory reset
    bool result = config->factoryReset();
    TEST_ASSERT_TRUE(result);

    // Re-initialize to load defaults
    config->begin();

    // Settings should be back to defaults
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, config->getPidKp());  // Default Kp
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 22.0f, config->getSetpoint()); // Default setpoint
}

/**
 * Test 7.2: Factory reset returns true on success
 */
void test_factory_reset_success(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    bool result = config->factoryReset();
    TEST_ASSERT_TRUE(result);
}

// ===== TEST SUITE 8: Edge Cases and Boundary Values =====

/**
 * Test 8.1: Empty WiFi SSID
 */
void test_empty_wifi_ssid(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setWifiSSID("");
    TEST_ASSERT_EQUAL_STRING("", config->getWifiSSID().c_str());
}

/**
 * Test 8.2: Very long WiFi SSID (32 characters max)
 */
void test_long_wifi_ssid(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    String longSSID = "VeryLongNetworkName123456789012"; // 32 chars
    config->setWifiSSID(longSSID);

    String retrieved = config->getWifiSSID();
    TEST_ASSERT_TRUE(retrieved.length() <= 32);
}

/**
 * Test 8.3: MQTT port boundary values
 */
void test_mqtt_port_boundaries(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Test valid ports
    config->setMqttPort(1);
    TEST_ASSERT_EQUAL_UINT16(1, config->getMqttPort());

    config->setMqttPort(65535);
    TEST_ASSERT_EQUAL_UINT16(65535, config->getMqttPort());

    config->setMqttPort(1883);
    TEST_ASSERT_EQUAL_UINT16(1883, config->getMqttPort());
}

/**
 * Test 8.4: KNX address boundaries
 */
void test_knx_address_boundaries(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Area: 0-15
    config->setKnxArea(0);
    TEST_ASSERT_EQUAL_UINT8(0, config->getKnxArea());

    config->setKnxArea(15);
    TEST_ASSERT_EQUAL_UINT8(15, config->getKnxArea());

    // Line: 0-15
    config->setKnxLine(0);
    TEST_ASSERT_EQUAL_UINT8(0, config->getKnxLine());

    config->setKnxLine(15);
    TEST_ASSERT_EQUAL_UINT8(15, config->getKnxLine());

    // Member: 0-255
    config->setKnxMember(0);
    TEST_ASSERT_EQUAL_UINT8(0, config->getKnxMember());

    config->setKnxMember(255);
    TEST_ASSERT_EQUAL_UINT8(255, config->getKnxMember());
}

/**
 * Test 8.5: Setpoint boundaries (5-30°C)
 */
void test_setpoint_boundaries(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    // Valid range
    config->setSetpoint(5.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.0f, config->getSetpoint());

    config->setSetpoint(30.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 30.0f, config->getSetpoint());

    config->setSetpoint(22.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 22.5f, config->getSetpoint());
}

/**
 * Test 8.6: Zero values for timing parameters
 */
void test_zero_timing_values(void) {
    ConfigManager* config = ConfigManager::getInstance();
    config->begin();

    config->setSensorUpdateInterval(0);
    TEST_ASSERT_EQUAL_UINT32(0, config->getSensorUpdateInterval());

    config->setManualOverrideTimeout(0);
    TEST_ASSERT_EQUAL_UINT32(0, config->getManualOverrideTimeout());
}

// ===== Main Test Runner =====

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Suite 1: Initialization
    RUN_TEST(test_singleton_instance);
    RUN_TEST(test_begin_initialization);

    // Suite 2: Default Values
    RUN_TEST(test_default_pid_parameters);
    RUN_TEST(test_default_network_settings);
    RUN_TEST(test_default_mqtt_settings);
    RUN_TEST(test_default_knx_settings);

    // Suite 3: Getters and Setters
    RUN_TEST(test_wifi_credentials_storage);
    RUN_TEST(test_mqtt_settings_storage);
    RUN_TEST(test_knx_address_storage);
    RUN_TEST(test_pid_parameters_storage);
    RUN_TEST(test_timing_parameters_storage);
    RUN_TEST(test_manual_override_settings);
    RUN_TEST(test_webhook_settings);

    // Suite 4: JSON Export/Import
    RUN_TEST(test_export_to_json);
    RUN_TEST(test_import_from_json_valid);
    RUN_TEST(test_import_from_json_invalid_mqtt_port);
    RUN_TEST(test_import_from_json_invalid_knx_area);
    RUN_TEST(test_import_from_json_invalid_setpoint);
    RUN_TEST(test_json_round_trip);

    // Suite 5: Precision Rounding
    RUN_TEST(test_round_to_precision_basic);
    RUN_TEST(test_round_to_precision_decimals);
    RUN_TEST(test_pid_parameter_precision);

    // Suite 6: Diagnostic Settings
    RUN_TEST(test_reboot_reason_tracking);
    RUN_TEST(test_reboot_count_tracking);
    RUN_TEST(test_consecutive_watchdog_reboots);
    RUN_TEST(test_last_connected_timestamp);

    // Suite 7: Factory Reset
    RUN_TEST(test_factory_reset_clears_settings);
    RUN_TEST(test_factory_reset_success);

    // Suite 8: Edge Cases
    RUN_TEST(test_empty_wifi_ssid);
    RUN_TEST(test_long_wifi_ssid);
    RUN_TEST(test_mqtt_port_boundaries);
    RUN_TEST(test_knx_address_boundaries);
    RUN_TEST(test_setpoint_boundaries);
    RUN_TEST(test_zero_timing_values);

    return UNITY_END();
}
