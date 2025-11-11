/**
 * @file test_precision.cpp
 * @brief Unit tests for ConfigManager::roundToPrecision utility function
 *
 * These tests verify the correct rounding behavior for PID parameters and setpoints.
 */

#include <unity.h>
#include "config_manager.h"

void setUp(void) {
    // Runs before each test
}

void tearDown(void) {
    // Runs after each test
}

// Test rounding to 0 decimal places
void test_roundToPrecision_zero_decimals() {
    TEST_ASSERT_EQUAL_FLOAT(5.0, ConfigManager::roundToPrecision(5.4, 0));
    TEST_ASSERT_EQUAL_FLOAT(6.0, ConfigManager::roundToPrecision(5.5, 0));
    TEST_ASSERT_EQUAL_FLOAT(6.0, ConfigManager::roundToPrecision(5.6, 0));
}

// Test rounding to 1 decimal place (setpoint precision)
void test_roundToPrecision_one_decimal() {
    TEST_ASSERT_EQUAL_FLOAT(22.0, ConfigManager::roundToPrecision(22.04, 1));
    TEST_ASSERT_EQUAL_FLOAT(22.1, ConfigManager::roundToPrecision(22.05, 1));
    TEST_ASSERT_EQUAL_FLOAT(22.1, ConfigManager::roundToPrecision(22.14, 1));
    TEST_ASSERT_EQUAL_FLOAT(22.2, ConfigManager::roundToPrecision(22.15, 1));
}

// Test rounding to 2 decimal places (Kp precision)
void test_roundToPrecision_two_decimals() {
    TEST_ASSERT_EQUAL_FLOAT(2.00, ConfigManager::roundToPrecision(2.004, 2));
    TEST_ASSERT_EQUAL_FLOAT(2.01, ConfigManager::roundToPrecision(2.005, 2));
    TEST_ASSERT_EQUAL_FLOAT(2.01, ConfigManager::roundToPrecision(2.014, 2));
    TEST_ASSERT_EQUAL_FLOAT(2.02, ConfigManager::roundToPrecision(2.015, 2));
}

// Test rounding to 3 decimal places (Ki/Kd precision)
void test_roundToPrecision_three_decimals() {
    TEST_ASSERT_EQUAL_FLOAT(0.100, ConfigManager::roundToPrecision(0.1004, 3));
    TEST_ASSERT_EQUAL_FLOAT(0.101, ConfigManager::roundToPrecision(0.1005, 3));
    TEST_ASSERT_EQUAL_FLOAT(0.101, ConfigManager::roundToPrecision(0.1014, 3));
    TEST_ASSERT_EQUAL_FLOAT(0.102, ConfigManager::roundToPrecision(0.1015, 3));
}

// Test negative numbers
void test_roundToPrecision_negative_numbers() {
    TEST_ASSERT_EQUAL_FLOAT(-5.0, ConfigManager::roundToPrecision(-5.4, 0));
    TEST_ASSERT_EQUAL_FLOAT(-6.0, ConfigManager::roundToPrecision(-5.5, 0));  // Round away from zero (symmetric with 5.5->6.0)
    TEST_ASSERT_EQUAL_FLOAT(-22.1, ConfigManager::roundToPrecision(-22.05, 1));
    TEST_ASSERT_EQUAL_FLOAT(-2.01, ConfigManager::roundToPrecision(-2.005, 2));
}

// Test edge cases
void test_roundToPrecision_edge_cases() {
    // Zero
    TEST_ASSERT_EQUAL_FLOAT(0.0, ConfigManager::roundToPrecision(0.0, 2));

    // Very small numbers
    TEST_ASSERT_EQUAL_FLOAT(0.001, ConfigManager::roundToPrecision(0.0006, 3));

    // Large numbers
    TEST_ASSERT_EQUAL_FLOAT(1000.0, ConfigManager::roundToPrecision(999.999, 0));
}

// Test typical PID parameter values
void test_roundToPrecision_typical_pid_values() {
    // Kp - 2 decimals
    TEST_ASSERT_EQUAL_FLOAT(2.50, ConfigManager::roundToPrecision(2.5, 2));
    TEST_ASSERT_EQUAL_FLOAT(2.00, ConfigManager::roundToPrecision(2.0, 2));

    // Ki - 3 decimals
    TEST_ASSERT_EQUAL_FLOAT(0.100, ConfigManager::roundToPrecision(0.1, 3));
    TEST_ASSERT_EQUAL_FLOAT(0.050, ConfigManager::roundToPrecision(0.05, 3));

    // Kd - 3 decimals
    TEST_ASSERT_EQUAL_FLOAT(0.500, ConfigManager::roundToPrecision(0.5, 3));
    TEST_ASSERT_EQUAL_FLOAT(1.000, ConfigManager::roundToPrecision(1.0, 3));

    // Setpoint - 1 decimal
    TEST_ASSERT_EQUAL_FLOAT(22.5, ConfigManager::roundToPrecision(22.5, 1));
    TEST_ASSERT_EQUAL_FLOAT(20.0, ConfigManager::roundToPrecision(20.0, 1));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_roundToPrecision_zero_decimals);
    RUN_TEST(test_roundToPrecision_one_decimal);
    RUN_TEST(test_roundToPrecision_two_decimals);
    RUN_TEST(test_roundToPrecision_three_decimals);
    RUN_TEST(test_roundToPrecision_negative_numbers);
    RUN_TEST(test_roundToPrecision_edge_cases);
    RUN_TEST(test_roundToPrecision_typical_pid_values);

    return UNITY_END();
}
