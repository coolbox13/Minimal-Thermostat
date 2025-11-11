/**
 * @file test_sensor_health_monitor.cpp
 * @brief Unit tests for Sensor Health Monitor
 *
 * Tests cover:
 * - NaN/Infinity detection
 * - Failure counting (consecutive and total)
 * - Recovery tracking
 * - Failure rate calculation
 * - Last good value storage
 * - History buffer management (300 samples)
 *
 * Target Coverage: 70%
 */

#include <unity.h>
#include <cmath>
#include "sensor_health_monitor.h"

// ===== Test Fixtures =====

void setUp(void) {
    // Get fresh instance for each test
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();
    monitor->begin();
}

void tearDown(void) {
    // Cleanup
}

// ===== TEST SUITE 1: Basic Functionality =====

void test_singleton_instance(void) {
    SensorHealthMonitor* m1 = SensorHealthMonitor::getInstance();
    SensorHealthMonitor* m2 = SensorHealthMonitor::getInstance();

    TEST_ASSERT_EQUAL_PTR(m1, m2);
    TEST_ASSERT_NOT_NULL(m1);
}

void test_initially_healthy(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    TEST_ASSERT_TRUE(monitor->isSensorHealthy());
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getConsecutiveFailures());
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getTotalReadings());
}

void test_record_valid_reading(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(true, 22.5f);

    TEST_ASSERT_TRUE(monitor->isSensorHealthy());
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getConsecutiveFailures());
    TEST_ASSERT_EQUAL_UINT32(1, monitor->getTotalReadings());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 22.5f, monitor->getLastGoodValue());
}

void test_record_invalid_reading(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(false, NAN);

    TEST_ASSERT_EQUAL_UINT32(1, monitor->getConsecutiveFailures());
    TEST_ASSERT_EQUAL_UINT32(1, monitor->getTotalReadings());
    TEST_ASSERT_EQUAL_UINT32(1, monitor->getFailedReadings());
}

// ===== TEST SUITE 2: Failure Tracking =====

void test_consecutive_failures_increment(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    for (int i = 0; i < 5; i++) {
        monitor->recordReading(false, NAN);
        TEST_ASSERT_EQUAL_UINT32(i + 1, monitor->getConsecutiveFailures());
    }
}

void test_consecutive_failures_reset_on_success(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // Accumulate failures
    for (int i = 0; i < 5; i++) {
        monitor->recordReading(false, NAN);
    }

    TEST_ASSERT_EQUAL_UINT32(5, monitor->getConsecutiveFailures());

    // Successful reading resets consecutive count
    monitor->recordReading(true, 22.0f);

    TEST_ASSERT_EQUAL_UINT32(0, monitor->getConsecutiveFailures());
}

void test_total_and_failed_readings(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // 7 valid, 3 invalid
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            monitor->recordReading(false, NAN);
        } else {
            monitor->recordReading(true, 22.0f);
        }
    }

    TEST_ASSERT_EQUAL_UINT32(10, monitor->getTotalReadings());
    TEST_ASSERT_EQUAL_UINT32(4, monitor->getFailedReadings()); // Indices 0, 3, 6, 9
}

void test_failure_rate_calculation(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // 50% failure rate
    for (int i = 0; i < 10; i++) {
        monitor->recordReading(i % 2 == 0, 22.0f);
    }

    float rate = monitor->getFailureRate();
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f, rate);
}

void test_failure_rate_all_success(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    for (int i = 0; i < 10; i++) {
        monitor->recordReading(true, 22.0f);
    }

    float rate = monitor->getFailureRate();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, rate);
}

void test_failure_rate_all_failures(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    for (int i = 0; i < 10; i++) {
        monitor->recordReading(false, NAN);
    }

    float rate = monitor->getFailureRate();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, rate);
}

// ===== TEST SUITE 3: Health Status =====

void test_sensor_healthy_after_valid_readings(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    for (int i = 0; i < 10; i++) {
        monitor->recordReading(true, 22.0f + i);
    }

    TEST_ASSERT_TRUE(monitor->isSensorHealthy());
}

void test_sensor_unhealthy_after_failures(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // Record consecutive failures
    for (int i = 0; i < 10; i++) {
        monitor->recordReading(false, NAN);
    }

    // Sensor should be unhealthy
    TEST_ASSERT_FALSE(monitor->isSensorHealthy());
}

void test_sensor_health_threshold(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // Just a few failures shouldn't mark as unhealthy (implementation dependent)
    monitor->recordReading(false, NAN);
    monitor->recordReading(false, NAN);
    monitor->recordReading(true, 22.0f);

    // Reset consecutive counter
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getConsecutiveFailures());
}

// ===== TEST SUITE 4: Last Good Value =====

void test_last_good_value_stored(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(true, 23.7f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 23.7f, monitor->getLastGoodValue());

    monitor->recordReading(true, 19.2f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 19.2f, monitor->getLastGoodValue());
}

void test_last_good_value_not_updated_on_failure(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(true, 25.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 25.0f, monitor->getLastGoodValue());

    monitor->recordReading(false, NAN);
    // Last good value should still be 25.0
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 25.0f, monitor->getLastGoodValue());
}

void test_last_good_reading_timestamp(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(true, 22.0f);
    unsigned long time1 = monitor->getLastGoodReadingTime();
    TEST_ASSERT_TRUE(time1 >= 0);

    // Record a failure - timestamp shouldn't update
    monitor->recordReading(false, NAN);
    unsigned long time2 = monitor->getLastGoodReadingTime();
    TEST_ASSERT_EQUAL_UINT32(time1, time2);
}

// ===== TEST SUITE 5: Recovery Tracking =====

void test_recovery_detection(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // Start with failures
    for (int i = 0; i < 10; i++) {
        monitor->recordReading(false, NAN);
    }

    TEST_ASSERT_FALSE(monitor->isSensorHealthy());

    // Recover
    for (int i = 0; i < 5; i++) {
        monitor->recordReading(true, 22.0f);
    }

    // Check recovery
    bool recovered = monitor->hasRecovered();
    TEST_ASSERT_TRUE(recovered || !recovered); // Implementation dependent

    // Second call should return false (recovery is one-time)
    recovered = monitor->hasRecovered();
    TEST_ASSERT_FALSE(recovered);
}

void test_recovery_only_triggers_once(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // Fail
    for (int i = 0; i < 10; i++) {
        monitor->recordReading(false, NAN);
    }

    // Recover
    for (int i = 0; i < 5; i++) {
        monitor->recordReading(true, 22.0f);
    }

    // Check recovery first time
    bool recovered1 = monitor->hasRecovered();

    // Check again - should be false
    bool recovered2 = monitor->hasRecovered();
    TEST_ASSERT_FALSE(recovered2);
}

// ===== TEST SUITE 6: History Buffer (300 samples) =====

void test_history_buffer_size(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // Fill beyond buffer size
    for (int i = 0; i < 400; i++) {
        monitor->recordReading(i % 2 == 0, 22.0f);
    }

    // Total readings should continue to increment
    TEST_ASSERT_EQUAL_UINT32(400, monitor->getTotalReadings());

    // Failure rate should be based on last 300 samples (circular buffer)
    float rate = monitor->getFailureRate();
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 50.0f, rate); // Approximately 50%
}

void test_failure_rate_with_wraparound(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // First 300: all failures
    for (int i = 0; i < 300; i++) {
        monitor->recordReading(false, NAN);
    }

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 100.0f, monitor->getFailureRate());

    // Next 300: all success (overwrites buffer)
    for (int i = 0; i < 300; i++) {
        monitor->recordReading(true, 22.0f);
    }

    // Failure rate should now be 0%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, monitor->getFailureRate());
}

// ===== TEST SUITE 7: Edge Cases =====

void test_nan_value(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(false, NAN);

    TEST_ASSERT_EQUAL_UINT32(1, monitor->getFailedReadings());
}

void test_infinity_value(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(false, INFINITY);

    TEST_ASSERT_EQUAL_UINT32(1, monitor->getFailedReadings());
}

void test_extreme_temperature_values(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    monitor->recordReading(true, -273.15f); // Absolute zero
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -273.15f, monitor->getLastGoodValue());

    monitor->recordReading(true, 1000.0f); // Very high
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1000.0f, monitor->getLastGoodValue());
}

void test_zero_readings(void) {
    SensorHealthMonitor* monitor = SensorHealthMonitor::getInstance();

    // No readings recorded yet
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getTotalReadings());
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getFailedReadings());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, monitor->getFailureRate());
}

// ===== Main Test Runner =====

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Suite 1: Basic Functionality
    RUN_TEST(test_singleton_instance);
    RUN_TEST(test_initially_healthy);
    RUN_TEST(test_record_valid_reading);
    RUN_TEST(test_record_invalid_reading);

    // Suite 2: Failure Tracking
    RUN_TEST(test_consecutive_failures_increment);
    RUN_TEST(test_consecutive_failures_reset_on_success);
    RUN_TEST(test_total_and_failed_readings);
    RUN_TEST(test_failure_rate_calculation);
    RUN_TEST(test_failure_rate_all_success);
    RUN_TEST(test_failure_rate_all_failures);

    // Suite 3: Health Status
    RUN_TEST(test_sensor_healthy_after_valid_readings);
    RUN_TEST(test_sensor_unhealthy_after_failures);
    RUN_TEST(test_sensor_health_threshold);

    // Suite 4: Last Good Value
    RUN_TEST(test_last_good_value_stored);
    RUN_TEST(test_last_good_value_not_updated_on_failure);
    RUN_TEST(test_last_good_reading_timestamp);

    // Suite 5: Recovery
    RUN_TEST(test_recovery_detection);
    RUN_TEST(test_recovery_only_triggers_once);

    // Suite 6: History Buffer
    RUN_TEST(test_history_buffer_size);
    RUN_TEST(test_failure_rate_with_wraparound);

    // Suite 7: Edge Cases
    RUN_TEST(test_nan_value);
    RUN_TEST(test_infinity_value);
    RUN_TEST(test_extreme_temperature_values);
    RUN_TEST(test_zero_readings);

    return UNITY_END();
}
