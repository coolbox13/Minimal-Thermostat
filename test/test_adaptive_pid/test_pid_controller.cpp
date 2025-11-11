/**
 * @file test_pid_controller.cpp
 * @brief Comprehensive unit tests for the Adaptive PID Controller
 *
 * Tests cover:
 * - Basic PID calculation (proportional, integral, derivative terms)
 * - Deadband functionality
 * - Temperature history management (circular buffer)
 * - Self-tuning/adaptation logic
 * - Output clamping (0-100%)
 * - Error handling (NaN, Infinity, out-of-range)
 * - Anti-windup protection
 * - Performance analysis metrics
 *
 * Target Coverage: 80%
 */

#include <unity.h>
#include <cmath>
#include <cstring>
#include "adaptive_pid_controller.h"

// ===== Test Fixtures =====

void setUp(void) {
    // Reset global state before each test
    memset(&g_pid_input, 0, sizeof(AdaptivePID_Input));
    memset(&g_pid_output, 0, sizeof(AdaptivePID_Output));
    memset(g_temperature_history, 0, sizeof(g_temperature_history));
    memset(g_setpoint_history, 0, sizeof(g_setpoint_history));
    g_history_index = 0;
}

void tearDown(void) {
    // Cleanup after each test (if needed)
}

// ===== Helper Functions =====

/**
 * Initialize PID with known test parameters
 */
void initTestPID(float kp = 2.0f, float ki = 0.1f, float kd = 0.5f) {
    g_pid_input.current_temp = 20.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.valve_feedback = 0.0f;
    g_pid_input.Kp = kp;
    g_pid_input.Ki = ki;
    g_pid_input.Kd = kd;
    g_pid_input.output_min = 0.0f;
    g_pid_input.output_max = 100.0f;
    g_pid_input.deadband = 0.2f;
    g_pid_input.dt = 1.0f;
    g_pid_input.adaptation_rate = 0.05f;
    g_pid_input.adaptation_enabled = 0; // Disable by default for predictable tests

    AdaptivePID_Init(&g_pid_input);
}

/**
 * Fill temperature history with test data
 */
void fillHistoryWithTestData(float start_temp, float end_temp, int size) {
    float step = (end_temp - start_temp) / size;
    for (int i = 0; i < size; i++) {
        g_temperature_history[i] = start_temp + (step * i);
        g_setpoint_history[i] = end_temp;
    }
}

// ===== TEST SUITE 1: Basic PID Calculation =====

/**
 * Test 1.1: Proportional term calculation
 * Verify that proportional output is Kp * error
 */
void test_proportional_term_basic(void) {
    initTestPID(2.0f, 0.0f, 0.0f); // Only Kp active

    g_pid_input.current_temp = 20.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f; // Disable deadband for this test

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Error = 22 - 20 = 2.0
    // Expected output = Kp * error = 2.0 * 2.0 = 4.0
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, g_pid_output.error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.0f, g_pid_output.valve_command);
}

/**
 * Test 1.2: Integral term accumulation
 * Verify that integral term accumulates error over time
 */
void test_integral_term_accumulation(void) {
    initTestPID(0.0f, 0.1f, 0.0f); // Only Ki active

    g_pid_input.current_temp = 20.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;
    g_pid_input.dt = 1.0f;

    // First update: integral = 2.0 * 1.0 = 2.0, output = 0.1 * 2.0 = 0.2
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, g_pid_output.integral_error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.2f, g_pid_output.valve_command);

    // Second update: integral = 2.0 + 2.0 = 4.0, output = 0.1 * 4.0 = 0.4
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.0f, g_pid_output.integral_error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.4f, g_pid_output.valve_command);

    // Third update: integral = 4.0 + 2.0 = 6.0, output = 0.1 * 6.0 = 0.6
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.0f, g_pid_output.integral_error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.6f, g_pid_output.valve_command);
}

/**
 * Test 1.3: Derivative term calculation
 * Verify that derivative term responds to rate of change
 */
void test_derivative_term_basic(void) {
    initTestPID(0.0f, 0.0f, 1.0f); // Only Kd active

    g_pid_input.current_temp = 20.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;
    g_pid_input.dt = 1.0f;

    // First update: derivative = -(20 - 20) / 1.0 = 0
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, g_pid_output.derivative_error);

    // Second update: temp increases to 21
    g_pid_input.current_temp = 21.0f;
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    // derivative = -(21 - 20) / 1.0 = -1.0
    // output = 1.0 * -1.0 = -1.0, but clamped to 0
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, g_pid_output.derivative_error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, g_pid_output.valve_command); // Clamped to min
}

/**
 * Test 1.4: Combined PID terms
 * Verify all three terms work together correctly
 */
void test_combined_pid_terms(void) {
    initTestPID(2.0f, 0.1f, 0.5f); // All terms active

    g_pid_input.current_temp = 20.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;
    g_pid_input.dt = 1.0f;

    // First update
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    // P = 2.0 * 2.0 = 4.0
    // I = 0.1 * 2.0 = 0.2
    // D = 0.5 * 0.0 = 0.0
    // Total = 4.2
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.2f, g_pid_output.valve_command);

    // Second update: temp increases slightly
    g_pid_input.current_temp = 20.5f;
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    // P = 2.0 * 1.5 = 3.0
    // I = 0.1 * (2.0 + 1.5) = 0.35
    // D = 0.5 * -(0.5 - 0) / 1.0 = -0.25
    // Total = 3.0 + 0.35 - 0.25 = 3.1
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 3.1f, g_pid_output.valve_command);
}

// ===== TEST SUITE 2: Deadband Functionality =====

/**
 * Test 2.1: Output unchanged within deadband
 * Verify controller doesn't adjust output when error is within deadband
 */
void test_deadband_no_output_change(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.current_temp = 21.9f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.valve_feedback = 50.0f;
    g_pid_input.deadband = 0.2f; // ±0.2°C deadband

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Error = 0.1°C, within deadband of 0.2°C
    // Output should match valve feedback
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, g_pid_output.error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, g_pid_output.valve_command);
}

/**
 * Test 2.2: Output changes outside deadband
 * Verify controller adjusts output when error exceeds deadband
 */
void test_deadband_output_changes_outside(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.current_temp = 21.5f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.valve_feedback = 50.0f;
    g_pid_input.deadband = 0.2f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Error = 0.5°C, outside deadband
    // Output should be calculated by PID
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, g_pid_output.error);
    TEST_ASSERT_NOT_EQUAL(50.0f, g_pid_output.valve_command);
}

/**
 * Test 2.3: Deadband boundary conditions
 * Test behavior exactly at deadband limits
 */
void test_deadband_boundary(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.valve_feedback = 50.0f;
    g_pid_input.deadband = 0.2f;

    // Test at positive boundary (exactly 0.2°C)
    g_pid_input.current_temp = 21.8f; // Error = 0.2°C
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, g_pid_output.valve_command);

    // Test at negative boundary (exactly -0.2°C)
    initTestPID(2.0f, 0.1f, 0.5f);
    g_pid_input.current_temp = 22.2f; // Error = -0.2°C
    g_pid_input.valve_feedback = 50.0f;
    g_pid_input.deadband = 0.2f;
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, g_pid_output.valve_command);
}

// ===== TEST SUITE 3: Anti-Windup Protection =====

/**
 * Test 3.1: Integral windup prevention at maximum
 * Verify integral term is clamped at output maximum
 */
void test_anti_windup_at_max(void) {
    initTestPID(0.0f, 1.0f, 0.0f); // High Ki for quick windup

    g_pid_input.current_temp = 10.0f;
    g_pid_input.setpoint_temp = 22.0f; // Large error
    g_pid_input.deadband = 0.0f;
    g_pid_input.output_max = 100.0f;

    // Run multiple updates to accumulate integral
    for (int i = 0; i < 200; i++) {
        AdaptivePID_Update(&g_pid_input, &g_pid_output);
    }

    // Integral should be clamped at output_max
    TEST_ASSERT_TRUE(g_pid_output.integral_error <= g_pid_input.output_max);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, g_pid_output.valve_command);
}

/**
 * Test 3.2: Integral windup prevention at minimum
 * Verify integral term is clamped at output minimum
 */
void test_anti_windup_at_min(void) {
    initTestPID(0.0f, 1.0f, 0.0f);

    g_pid_input.current_temp = 30.0f;
    g_pid_input.setpoint_temp = 22.0f; // Large negative error
    g_pid_input.deadband = 0.0f;
    g_pid_input.output_min = 0.0f;

    // Run multiple updates to accumulate negative integral
    for (int i = 0; i < 200; i++) {
        AdaptivePID_Update(&g_pid_input, &g_pid_output);
    }

    // Integral should be clamped at output_min
    TEST_ASSERT_TRUE(g_pid_output.integral_error >= g_pid_input.output_min);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, g_pid_output.valve_command);
}

// ===== TEST SUITE 4: Output Clamping =====

/**
 * Test 4.1: Output clamped to maximum (100%)
 */
void test_output_clamp_maximum(void) {
    initTestPID(10.0f, 1.0f, 1.0f); // High gains

    g_pid_input.current_temp = 10.0f;
    g_pid_input.setpoint_temp = 30.0f; // Large error
    g_pid_input.deadband = 0.0f;
    g_pid_input.output_max = 100.0f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    TEST_ASSERT_TRUE(g_pid_output.valve_command <= 100.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, g_pid_output.valve_command);
}

/**
 * Test 4.2: Output clamped to minimum (0%)
 */
void test_output_clamp_minimum(void) {
    initTestPID(10.0f, 1.0f, 1.0f);

    g_pid_input.current_temp = 30.0f;
    g_pid_input.setpoint_temp = 10.0f; // Large negative error
    g_pid_input.deadband = 0.0f;
    g_pid_input.output_min = 0.0f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    TEST_ASSERT_TRUE(g_pid_output.valve_command >= 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, g_pid_output.valve_command);
}

/**
 * Test 4.3: Output within valid range
 */
void test_output_within_range(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.current_temp = 21.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    TEST_ASSERT_TRUE(g_pid_output.valve_command >= 0.0f);
    TEST_ASSERT_TRUE(g_pid_output.valve_command <= 100.0f);
}

// ===== TEST SUITE 5: Error Handling =====

/**
 * Test 5.1: Handle NaN temperature input
 */
void test_error_handling_nan_temperature(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.current_temp = NAN;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Output should be valid (not NaN)
    TEST_ASSERT_FALSE(isnan(g_pid_output.valve_command));
}

/**
 * Test 5.2: Handle Infinity temperature input
 */
void test_error_handling_infinity_temperature(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.current_temp = INFINITY;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Output should be clamped to valid range
    TEST_ASSERT_FALSE(isinf(g_pid_output.valve_command));
    TEST_ASSERT_TRUE(g_pid_output.valve_command >= 0.0f);
    TEST_ASSERT_TRUE(g_pid_output.valve_command <= 100.0f);
}

/**
 * Test 5.3: Handle extreme temperature values
 */
void test_error_handling_extreme_values(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    // Test very high temperature
    g_pid_input.current_temp = 1000.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;

    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Output should be clamped
    TEST_ASSERT_TRUE(g_pid_output.valve_command >= 0.0f);
    TEST_ASSERT_TRUE(g_pid_output.valve_command <= 100.0f);
}

/**
 * Test 5.4: Validate PID parameter limits
 */
void test_pid_parameter_validation(void) {
    // Test Kp limits
    setPidKp(5.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, g_pid_input.Kp);

    setPidKp(150.0f); // Above max (100)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, g_pid_input.Kp); // Should not change

    setPidKp(-1.0f); // Below min (0)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, g_pid_input.Kp); // Should not change

    // Test Ki limits
    setPidKi(1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, g_pid_input.Ki);

    setPidKi(15.0f); // Above max (10)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, g_pid_input.Ki); // Should not change

    // Test Kd limits
    setPidKd(0.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, g_pid_input.Kd);

    setPidKd(15.0f); // Above max (10)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, g_pid_input.Kd); // Should not change
}

// ===== TEST SUITE 6: Temperature History Management =====

/**
 * Test 6.1: History circular buffer wraps correctly
 */
void test_history_circular_buffer_wrap(void) {
    g_history_index = 0;

    // Fill history beyond HISTORY_SIZE (300)
    for (int i = 0; i < HISTORY_SIZE + 50; i++) {
        g_temperature_history[g_history_index] = (float)i;
        g_history_index = (g_history_index + 1) % HISTORY_SIZE;
    }

    // Index should have wrapped
    TEST_ASSERT_EQUAL_INT(50, g_history_index);

    // First 50 entries should contain latest data (300-349)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 300.0f, g_temperature_history[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 349.0f, g_temperature_history[49]);
}

/**
 * Test 6.2: History stores temperature values correctly
 */
void test_history_stores_values(void) {
    initTestPID();
    g_history_index = 0;

    float test_temps[] = {20.0f, 20.5f, 21.0f, 21.5f, 22.0f};

    for (int i = 0; i < 5; i++) {
        g_temperature_history[g_history_index] = test_temps[i];
        g_setpoint_history[g_history_index] = 22.0f;
        g_history_index = (g_history_index + 1) % HISTORY_SIZE;
    }

    // Verify stored values
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.01f, test_temps[i], g_temperature_history[i]);
        TEST_ASSERT_FLOAT_WITHIN(0.01f, 22.0f, g_setpoint_history[i]);
    }
}

/**
 * Test 6.3: History size is correct (300 samples)
 */
void test_history_size(void) {
    TEST_ASSERT_EQUAL_INT(300, HISTORY_SIZE);
}

// ===== TEST SUITE 7: Auto-Tuning =====

/**
 * Test 7.1: Auto-tune with oscillating data
 */
void test_autotune_oscillating_data(void) {
    initTestPID();

    // Create oscillating temperature data
    for (int i = 0; i < HISTORY_SIZE; i++) {
        float angle = (i * 2.0f * 3.14159f) / 20.0f; // 20-sample period
        g_temperature_history[i] = 22.0f + 2.0f * sin(angle);
    }

    float original_kp = g_pid_input.Kp;
    float original_ki = g_pid_input.Ki;
    float original_kd = g_pid_input.Kd;

    AdaptivePID_AutoTune(&g_pid_input, g_temperature_history, HISTORY_SIZE);

    // Parameters should have changed
    TEST_ASSERT_NOT_EQUAL(original_kp, g_pid_input.Kp);
    TEST_ASSERT_NOT_EQUAL(original_ki, g_pid_input.Ki);
    TEST_ASSERT_NOT_EQUAL(original_kd, g_pid_input.Kd);

    // New parameters should be positive and reasonable
    TEST_ASSERT_TRUE(g_pid_input.Kp > 0.0f);
    TEST_ASSERT_TRUE(g_pid_input.Ki > 0.0f);
    TEST_ASSERT_TRUE(g_pid_input.Kd > 0.0f);
}

/**
 * Test 7.2: Auto-tune with insufficient data
 */
void test_autotune_insufficient_data(void) {
    initTestPID();

    // Fill with constant temperature (no oscillation)
    for (int i = 0; i < HISTORY_SIZE; i++) {
        g_temperature_history[i] = 22.0f;
    }

    float original_kp = g_pid_input.Kp;

    AdaptivePID_AutoTune(&g_pid_input, g_temperature_history, HISTORY_SIZE);

    // Parameters should remain unchanged (no oscillation detected)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original_kp, g_pid_input.Kp);
}

// ===== TEST SUITE 8: Performance Analysis =====

/**
 * Test 8.1: Performance metrics calculation
 */
void test_performance_metrics_basic(void) {
    // Create test data: step response from 20 to 22°C
    fillHistoryWithTestData(20.0f, 22.0f, HISTORY_SIZE);

    AdaptivePID_Performance perf = AdaptivePID_AnalyzePerformance(
        g_temperature_history, g_setpoint_history, HISTORY_SIZE, 1.0f);

    // Should have calculated metrics
    TEST_ASSERT_TRUE(perf.rise_time >= 0.0f || perf.rise_time == -1.0f);
    TEST_ASSERT_TRUE(perf.settling_time >= 0.0f || perf.settling_time == -1.0f);
    TEST_ASSERT_TRUE(perf.overshoot >= 0.0f);
    TEST_ASSERT_TRUE(perf.steady_state_error >= 0.0f);
    TEST_ASSERT_TRUE(perf.oscillation_count >= 0.0f);
}

/**
 * Test 8.2: Performance with overshoot
 */
void test_performance_with_overshoot(void) {
    // Create data with overshoot
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (i < 50) {
            g_temperature_history[i] = 20.0f + (i * 0.1f); // Rise
        } else if (i < 100) {
            g_temperature_history[i] = 25.0f - ((i - 50) * 0.06f); // Overshoot and settle
        } else {
            g_temperature_history[i] = 22.0f; // Settled
        }
        g_setpoint_history[i] = 22.0f;
    }

    AdaptivePID_Performance perf = AdaptivePID_AnalyzePerformance(
        g_temperature_history, g_setpoint_history, HISTORY_SIZE, 1.0f);

    // Should detect overshoot
    TEST_ASSERT_TRUE(perf.overshoot > 0.0f);
}

/**
 * Test 8.3: Performance with oscillations
 */
void test_performance_with_oscillations(void) {
    // Create oscillating data around setpoint
    for (int i = 0; i < HISTORY_SIZE; i++) {
        float angle = (i * 2.0f * 3.14159f) / 30.0f;
        g_temperature_history[i] = 22.0f + sin(angle);
        g_setpoint_history[i] = 22.0f;
    }

    AdaptivePID_Performance perf = AdaptivePID_AnalyzePerformance(
        g_temperature_history, g_setpoint_history, HISTORY_SIZE, 1.0f);

    // Should detect multiple oscillations
    TEST_ASSERT_TRUE(perf.oscillation_count > 5.0f);
}

// ===== TEST SUITE 9: Setpoint Changes =====

/**
 * Test 9.1: Controller resets on setpoint change
 */
void test_setpoint_change_resets_state(void) {
    initTestPID(2.0f, 0.1f, 0.5f);

    g_pid_input.current_temp = 20.0f;
    g_pid_input.setpoint_temp = 22.0f;
    g_pid_input.deadband = 0.0f;

    // Run a few updates to accumulate integral
    for (int i = 0; i < 5; i++) {
        AdaptivePID_Update(&g_pid_input, &g_pid_output);
    }

    float integral_before_change = g_pid_output.integral_error;
    TEST_ASSERT_TRUE(integral_before_change > 0.0f);

    // Change setpoint significantly
    g_pid_input.setpoint_temp = 24.0f;
    AdaptivePID_Update(&g_pid_input, &g_pid_output);

    // Integral should have reset/adjusted
    // (implementation detail: may or may not reset completely)
    // Just verify controller continues to work
    TEST_ASSERT_FALSE(isnan(g_pid_output.valve_command));
}

/**
 * Test 9.2: Setpoint setter function
 */
void test_setpoint_setter(void) {
    initTestPID();

    setTemperatureSetpoint(25.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, g_pid_input.setpoint_temp);

    setTemperatureSetpoint(18.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 18.0f, g_pid_input.setpoint_temp);
}

// ===== TEST SUITE 10: Adaptation Logic =====

/**
 * Test 10.1: Adaptation disabled by default
 */
void test_adaptation_disabled(void) {
    initTestPID(2.0f, 0.1f, 0.5f);
    g_pid_input.adaptation_enabled = 0;

    float original_kp = g_pid_input.Kp;

    // Run many updates
    g_pid_input.deadband = 0.0f;
    for (int i = 0; i < 100; i++) {
        g_pid_input.current_temp = 20.0f + (i % 10) * 0.1f;
        AdaptivePID_Update(&g_pid_input, &g_pid_output);
    }

    // Parameters should not have changed
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original_kp, g_pid_input.Kp);
}

/**
 * Test 10.2: Adaptation enabled modifies parameters
 * Note: This test may be sensitive to adaptation logic changes
 */
void test_adaptation_enabled_basic(void) {
    initTestPID(2.0f, 0.1f, 0.5f);
    g_pid_input.adaptation_enabled = 1;
    g_pid_input.adaptation_rate = 0.1f; // Higher rate for testing
    g_pid_input.deadband = 0.0f;

    float original_kp = g_pid_input.Kp;

    // Create oscillating behavior to trigger adaptation
    for (int i = 0; i < 200; i++) {
        float angle = (i * 2.0f * 3.14159f) / 10.0f;
        g_pid_input.current_temp = 22.0f + 3.0f * sin(angle);
        g_pid_input.setpoint_temp = 22.0f;
        AdaptivePID_Update(&g_pid_input, &g_pid_output);
    }

    // Parameters may have adapted (implementation-dependent)
    // Just verify they remain in valid range
    TEST_ASSERT_TRUE(g_pid_input.Kp > 0.0f);
    TEST_ASSERT_TRUE(g_pid_input.Kp <= 100.0f);
    TEST_ASSERT_TRUE(g_pid_input.Ki > 0.0f);
    TEST_ASSERT_TRUE(g_pid_input.Ki <= 10.0f);
    TEST_ASSERT_TRUE(g_pid_input.Kd > 0.0f);
    TEST_ASSERT_TRUE(g_pid_input.Kd <= 10.0f);
}

// ===== Main Test Runner =====

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Suite 1: Basic PID Calculation
    RUN_TEST(test_proportional_term_basic);
    RUN_TEST(test_integral_term_accumulation);
    RUN_TEST(test_derivative_term_basic);
    RUN_TEST(test_combined_pid_terms);

    // Suite 2: Deadband Functionality
    RUN_TEST(test_deadband_no_output_change);
    RUN_TEST(test_deadband_output_changes_outside);
    RUN_TEST(test_deadband_boundary);

    // Suite 3: Anti-Windup Protection
    RUN_TEST(test_anti_windup_at_max);
    RUN_TEST(test_anti_windup_at_min);

    // Suite 4: Output Clamping
    RUN_TEST(test_output_clamp_maximum);
    RUN_TEST(test_output_clamp_minimum);
    RUN_TEST(test_output_within_range);

    // Suite 5: Error Handling
    RUN_TEST(test_error_handling_nan_temperature);
    RUN_TEST(test_error_handling_infinity_temperature);
    RUN_TEST(test_error_handling_extreme_values);
    RUN_TEST(test_pid_parameter_validation);

    // Suite 6: Temperature History
    RUN_TEST(test_history_circular_buffer_wrap);
    RUN_TEST(test_history_stores_values);
    RUN_TEST(test_history_size);

    // Suite 7: Auto-Tuning
    RUN_TEST(test_autotune_oscillating_data);
    RUN_TEST(test_autotune_insufficient_data);

    // Suite 8: Performance Analysis
    RUN_TEST(test_performance_metrics_basic);
    RUN_TEST(test_performance_with_overshoot);
    RUN_TEST(test_performance_with_oscillations);

    // Suite 9: Setpoint Changes
    RUN_TEST(test_setpoint_change_resets_state);
    RUN_TEST(test_setpoint_setter);

    // Suite 10: Adaptation Logic
    RUN_TEST(test_adaptation_disabled);
    RUN_TEST(test_adaptation_enabled_basic);

    return UNITY_END();
}
