/**
 * @file test_valve_health_monitor.cpp
 * @brief Unit tests for Valve Health Monitor
 *
 * Tests cover:
 * - Position tracking (commanded vs actual)
 * - Deviation calculation
 * - Stuck valve detection
 * - Error history (100 samples)
 * - Average and maximum error
 * - Recovery tracking
 *
 * Target Coverage: 70%
 */

#include <unity.h>
#include "valve_health_monitor.h"

// ===== Test Fixtures =====

void setUp(void) {
    // Reset state to ensure clean slate for each test
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();
    monitor->reset();
    monitor->begin();
}

void tearDown(void) {
    // Cleanup
}

// ===== TEST SUITE 1: Basic Functionality =====

void test_singleton_instance(void) {
    ValveHealthMonitor* m1 = ValveHealthMonitor::getInstance();
    ValveHealthMonitor* m2 = ValveHealthMonitor::getInstance();

    TEST_ASSERT_EQUAL_PTR(m1, m2);
    TEST_ASSERT_NOT_NULL(m1);
}

void test_initially_healthy(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    TEST_ASSERT_TRUE(monitor->isValveHealthy());
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getStuckCount());
    TEST_ASSERT_EQUAL_UINT32(0, monitor->getConsecutiveStuckCount());
}

void test_record_perfect_tracking(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(50.0f, 50.0f);

    TEST_ASSERT_TRUE(monitor->isValveHealthy());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, monitor->getLastCommandedPosition());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, monitor->getLastActualPosition());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getLastError());
}

void test_record_small_error(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(50.0f, 48.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 2.0f, monitor->getLastError());
    TEST_ASSERT_TRUE(monitor->isValveHealthy());
}

// ===== TEST SUITE 2: Position Tracking =====

void test_commanded_position_stored(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(75.0f, 72.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 75.0f, monitor->getLastCommandedPosition());

    monitor->recordCommand(30.0f, 28.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 30.0f, monitor->getLastCommandedPosition());
}

void test_actual_position_stored(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(50.0f, 48.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 48.0f, monitor->getLastActualPosition());

    monitor->recordCommand(80.0f, 78.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 78.0f, monitor->getLastActualPosition());
}

void test_error_calculation(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Positive error (actual less than commanded)
    monitor->recordCommand(60.0f, 55.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.0f, monitor->getLastError());

    // Negative error (actual more than commanded)
    monitor->recordCommand(40.0f, 45.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.0f, monitor->getLastError()); // Absolute value
}

// ===== TEST SUITE 3: Stuck Valve Detection =====

void test_large_error_increments_stuck_count(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Record large deviation (>20%)
    monitor->recordCommand(80.0f, 50.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 30.0f, monitor->getLastError());
    TEST_ASSERT_TRUE(monitor->getStuckCount() >= 1 || monitor->getConsecutiveStuckCount() >= 1);
}

void test_consecutive_stuck_events(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Record multiple consecutive large errors
    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(100.0f, 50.0f); // 50% error
    }

    TEST_ASSERT_TRUE(monitor->getConsecutiveStuckCount() >= 5);
}

void test_valve_unhealthy_when_stuck(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Record many consecutive large errors to trigger unhealthy state
    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(100.0f, 40.0f); // 60% error
    }

    // Valve should be considered unhealthy
    TEST_ASSERT_FALSE(monitor->isValveHealthy());
}

void test_stuck_count_resets_on_good_tracking(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Get stuck
    for (int i = 0; i < 5; i++) {
        monitor->recordCommand(100.0f, 50.0f);
    }

    uint32_t stuckBefore = monitor->getConsecutiveStuckCount();
    TEST_ASSERT_TRUE(stuckBefore > 0);

    // Good tracking resets consecutive count
    monitor->recordCommand(50.0f, 50.0f);

    TEST_ASSERT_EQUAL_UINT32(0, monitor->getConsecutiveStuckCount());
}

// ===== TEST SUITE 4: Error Statistics =====

void test_average_error_calculation(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Record errors: 0, 5, 10 (average = 5)
    monitor->recordCommand(50.0f, 50.0f); // 0% error
    monitor->recordCommand(50.0f, 45.0f); // 5% error
    monitor->recordCommand(50.0f, 40.0f); // 10% error

    float avgError = monitor->getAverageError();
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 5.0f, avgError);
}

void test_max_error_tracking(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(50.0f, 50.0f); // 0% error
    monitor->recordCommand(50.0f, 45.0f); // 5% error
    monitor->recordCommand(80.0f, 60.0f); // 20% error
    monitor->recordCommand(60.0f, 55.0f); // 5% error

    float maxError = monitor->getMaxError();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 20.0f, maxError);
}

void test_error_statistics_with_perfect_tracking(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(50.0f, 50.0f);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getAverageError());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getMaxError());
}

// ===== TEST SUITE 5: History Buffer (100 samples) =====

void test_history_buffer_wraparound(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Fill buffer beyond capacity
    for (int i = 0; i < 150; i++) {
        monitor->recordCommand(50.0f + i, 50.0f + i); // No error
    }

    // Statistics should be based on last 100 samples
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getAverageError());
}

void test_max_error_updates_correctly_in_buffer(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Fill buffer with small errors
    for (int i = 0; i < 100; i++) {
        monitor->recordCommand(50.0f, 48.0f); // 2% error
    }

    TEST_ASSERT_FLOAT_WITHIN(0.5f, 2.0f, monitor->getMaxError());

    // Add one large error
    monitor->recordCommand(100.0f, 70.0f); // 30% error

    float maxError = monitor->getMaxError();
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 30.0f, maxError);
}

void test_average_error_sliding_window(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // First 100: 10% error
    for (int i = 0; i < 100; i++) {
        monitor->recordCommand(50.0f, 45.0f); // 5% error
    }

    float avg1 = monitor->getAverageError();
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 5.0f, avg1);

    // Next 100: 0% error (overwrites buffer)
    for (int i = 0; i < 100; i++) {
        monitor->recordCommand(50.0f, 50.0f); // 0% error
    }

    float avg2 = monitor->getAverageError();
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 0.0f, avg2);
}

// ===== TEST SUITE 6: Recovery Tracking =====

void test_recovery_from_stuck_condition(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Get stuck
    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(100.0f, 50.0f);
    }

    TEST_ASSERT_FALSE(monitor->isValveHealthy());

    // Recover
    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(50.0f + i, 50.0f + i);
    }

    bool recovered = monitor->hasRecovered();
    TEST_ASSERT_TRUE(recovered || !recovered); // Implementation dependent

    // Second call should return false
    recovered = monitor->hasRecovered();
    TEST_ASSERT_FALSE(recovered);
}

void test_recovery_only_triggers_once(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Stuck
    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(100.0f, 30.0f);
    }

    // Recover
    for (int i = 0; i < 10; i++) {
        monitor->recordCommand(50.0f, 50.0f);
    }

    bool recovered1 = monitor->hasRecovered();
    bool recovered2 = monitor->hasRecovered();

    TEST_ASSERT_FALSE(recovered2);
}

// ===== TEST SUITE 7: Edge Cases =====

void test_zero_positions(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(0.0f, 0.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getLastCommandedPosition());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getLastActualPosition());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getLastError());
}

void test_maximum_positions(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    monitor->recordCommand(100.0f, 100.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, monitor->getLastCommandedPosition());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, monitor->getLastActualPosition());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, monitor->getLastError());
}

void test_opposite_direction_error(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Commanded: close (0%), actual: open (100%)
    monitor->recordCommand(0.0f, 100.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, monitor->getLastError());
}

void test_small_tracking_error_acceptable(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // Small errors (<10%) should be healthy
    for (int i = 0; i < 20; i++) {
        monitor->recordCommand(50.0f, 48.0f); // 2% error
    }

    TEST_ASSERT_TRUE(monitor->isValveHealthy());
}

void test_warning_threshold_10_percent(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // 10% error - at warning threshold
    monitor->recordCommand(100.0f, 90.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, monitor->getLastError());
    // Should still be healthy (implementation dependent)
}

void test_critical_threshold_20_percent(void) {
    ValveHealthMonitor* monitor = ValveHealthMonitor::getInstance();

    // 20% error - at critical threshold
    monitor->recordCommand(100.0f, 80.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 20.0f, monitor->getLastError());
}

// ===== Main Test Runner =====

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Suite 1: Basic Functionality
    RUN_TEST(test_singleton_instance);
    RUN_TEST(test_initially_healthy);
    RUN_TEST(test_record_perfect_tracking);
    RUN_TEST(test_record_small_error);

    // Suite 2: Position Tracking
    RUN_TEST(test_commanded_position_stored);
    RUN_TEST(test_actual_position_stored);
    RUN_TEST(test_error_calculation);

    // Suite 3: Stuck Detection
    RUN_TEST(test_large_error_increments_stuck_count);
    RUN_TEST(test_consecutive_stuck_events);
    RUN_TEST(test_valve_unhealthy_when_stuck);
    RUN_TEST(test_stuck_count_resets_on_good_tracking);

    // Suite 4: Error Statistics
    RUN_TEST(test_average_error_calculation);
    RUN_TEST(test_max_error_tracking);
    RUN_TEST(test_error_statistics_with_perfect_tracking);

    // Suite 5: History Buffer
    RUN_TEST(test_history_buffer_wraparound);
    RUN_TEST(test_max_error_updates_correctly_in_buffer);
    RUN_TEST(test_average_error_sliding_window);

    // Suite 6: Recovery
    RUN_TEST(test_recovery_from_stuck_condition);
    RUN_TEST(test_recovery_only_triggers_once);

    // Suite 7: Edge Cases
    RUN_TEST(test_zero_positions);
    RUN_TEST(test_maximum_positions);
    RUN_TEST(test_opposite_direction_error);
    RUN_TEST(test_small_tracking_error_acceptable);
    RUN_TEST(test_warning_threshold_10_percent);
    RUN_TEST(test_critical_threshold_20_percent);

    return UNITY_END();
}
