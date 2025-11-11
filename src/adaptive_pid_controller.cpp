// adaptive_pid_controller.c - modified initialization function

#include "adaptive_pid_controller.h"
#include "config.h"
#include "config_manager.h"
#include "logger.h"
#include <math.h>

// Define TAG for logging
static const char* TAG = "PID";

// Internal state variables
static float prev_error = 0.0f;      // Previous error for derivative term
static float integral_error = 0.0f;   // Accumulated integral error
static float prev_temp = 0.0f;        // Previous temperature
static float setpoint_time = 0.0f;    // Time since setpoint change
static float adaptation_timer = 0.0f; // Time since last adaptation
static float adaptation_interval_sec = 60.0f; // Adaptation interval in seconds (configurable)
static int oscillation_count = 0;     // Count of oscillations for tuning

// Performance history
static float error_sum = 0.0f;        // Sum of errors for performance evaluation
static float max_overshoot = 0.0f;    // Maximum overshoot
static int samples_count = 0;         // Number of samples taken
static float crossed_setpoint = 0;    // Flag for overshoot detection
static float previous_error_sign = 0; // Previous error sign for oscillation detection
static float rise_time_marker = -1;   // For rise time measurement

// Global controller state
AdaptivePID_Input g_pid_input;
AdaptivePID_Output g_pid_output;

// Temperature history for auto-tuning
float g_temperature_history[HISTORY_SIZE];
float g_setpoint_history[HISTORY_SIZE];
int g_history_index = 0;

// Forward declaration of internal function
static void adaptParameters(AdaptivePID_Input *input, AdaptivePID_Output *output, 
                           int oscillations, float overshoot, float avg_error);

/**
 * @brief Initialize the adaptive PID controller with parameters from storage.
 * 
 * Sets up the PID controller with values loaded from ConfigManager.
 */
void initializePIDController(void) {
    // Get config manager instance
    ConfigManager* configManager = ConfigManager::getInstance();
    
    // Start with default temperature (will be updated on first control cycle)
    g_pid_input.current_temp = 22.0f;
    
    // Load PID parameters from ConfigManager
    g_pid_input.setpoint_temp = configManager->getSetpoint(); // Using the pointer->method() syntax
    g_pid_input.Kp = configManager->getPidKp();
    g_pid_input.Ki = configManager->getPidKi();
    g_pid_input.Kd = configManager->getPidKd();

    g_pid_input.valve_feedback = 0.0f;  // Start with valve closed

    // Output constraints
    g_pid_input.output_min = 0.0f;   // Minimum valve position
    g_pid_input.output_max = 100.0f; // Maximum valve position

    // Control parameters
    g_pid_input.deadband = configManager->getPidDeadband(); // Load from config
    g_pid_input.dt = 1.0f;           // 1 second control interval

    // Load adaptation interval from config
    adaptation_interval_sec = configManager->getPidAdaptationInterval();

    // Adaptation parameters - enable by default
    g_pid_input.adaptation_enabled = 1;   // Enable self-learning
    g_pid_input.adaptation_rate = 0.05f;  // Conservative adaptation rate (0.05 = 5%)
    
    // Initialize the controller
    AdaptivePID_Init(&g_pid_input);
    
    // Clear history arrays
    for (int i = 0; i < HISTORY_SIZE; i++) {
        g_temperature_history[i] = g_pid_input.current_temp;
        g_setpoint_history[i] = g_pid_input.setpoint_temp;
    }
    
    // Run auto-tuning after collecting some data (this will happen automatically)
    g_history_index = 0;
    
    // Log the loaded parameters
    LOG_I(TAG, "PID controller initialized with parameters from storage");
    LOG_I(TAG, "Kp: %.3f, Ki: %.3f, Kd: %.3f, Setpoint: %.2f°C", 
          g_pid_input.Kp, g_pid_input.Ki, g_pid_input.Kd, g_pid_input.setpoint_temp);
}

/**
 * @brief Set a new proportional gain value.
 *
 * @param kp New proportional gain value.
 */
void setPidKp(float kp) {
    // HIGH PRIORITY FIX: Add maximum limit (Audit Fix #5)
    // Validate range to prevent extreme gains that cause instability
    // Max 100.0 is reasonable for HVAC applications
    if (kp >= 0.0f && kp <= 100.0f) {
        g_pid_input.Kp = kp;
        LOG_D(TAG, "Kp updated to: %.3f", kp);
    } else {
        LOG_W(TAG, "Invalid Kp value (%.3f) - must be between 0 and 100", kp);
    }
}

/**
 * @brief Set a new integral gain value.
 *
 * @param ki New integral gain value.
 */
void setPidKi(float ki) {
    // HIGH PRIORITY FIX: Add maximum limit (Audit Fix #5)
    // Validate range to prevent extreme gains that cause instability
    // Max 10.0 is reasonable for HVAC applications
    if (ki >= 0.0f && ki <= 10.0f) {
        g_pid_input.Ki = ki;
        LOG_D(TAG, "Ki updated to: %.3f", ki);
    } else {
        LOG_W(TAG, "Invalid Ki value (%.3f) - must be between 0 and 10", ki);
    }
}

/**
 * @brief Set a new derivative gain value.
 *
 * @param kd New derivative gain value.
 */
void setPidKd(float kd) {
    // HIGH PRIORITY FIX: Add maximum limit (Audit Fix #5)
    // Validate range to prevent extreme gains that cause instability
    // Max 10.0 is reasonable for HVAC applications
    if (kd >= 0.0f && kd <= 10.0f) {
        g_pid_input.Kd = kd;
        LOG_D(TAG, "Kd updated to: %.3f", kd);
    } else {
        LOG_W(TAG, "Invalid Kd value (%.3f) - must be between 0 and 10", kd);
    }
}

/**
 * @brief Update the PID controller with current temperature and valve readings.
 * 
 * This should be called regularly from the main loop.
 * 
 * @param current_temp Current temperature reading
 * @param valve_position Current valve/actuator position
 */
void updatePIDController(float current_temp, float valve_position) {
    // Update controller inputs
    g_pid_input.current_temp = current_temp;
    g_pid_input.valve_feedback = valve_position;
    
    // Update temperature history
    g_temperature_history[g_history_index] = current_temp;
    g_setpoint_history[g_history_index] = g_pid_input.setpoint_temp;
    g_history_index = (g_history_index + 1) % HISTORY_SIZE;
    
    // Run PID update
    AdaptivePID_Update(&g_pid_input, &g_pid_output);
    
    // Try auto-tuning if we have enough data (once when the buffer is full)
    static int auto_tuned = 0;
    if (!auto_tuned && g_history_index == 0) {
        AdaptivePID_AutoTune(&g_pid_input, g_temperature_history, HISTORY_SIZE);
        auto_tuned = 1;
    }
}

/**
 * @brief Get the current PID output value.
 * 
 * @return The calculated valve command (0-100%).
 */
float getPIDOutput(void) {
    return g_pid_output.valve_command;
}

/**
 * @brief Set a new temperature setpoint.
 * 
 * @param setpoint New temperature setpoint in °C.
 */
void setTemperatureSetpoint(float setpoint) {
    g_pid_input.setpoint_temp = setpoint;
}

/**
 * @brief Initialize the adaptive PID controller.
 * 
 * Resets internal state variables to default values.
 * 
 * @param input Pointer to the PID input structure.
 */
void AdaptivePID_Init(AdaptivePID_Input *input) {
    prev_error = 0.0f;
    integral_error = 0.0f;
    prev_temp = input->current_temp;
    setpoint_time = 0.0f;
    adaptation_timer = 0.0f;
    oscillation_count = 0;
    error_sum = 0.0f;
    max_overshoot = 0.0f;
    samples_count = 0;
    crossed_setpoint = 0;
    previous_error_sign = 0;
    rise_time_marker = -1;
    
    // Set default adaptation rate if not specified
    if (input->adaptation_rate <= 0.0f || input->adaptation_rate > 1.0f) {
        input->adaptation_rate = 0.05f; // Default conservative adaptation rate
    }
}

// Helper function to handle setpoint changes
static bool handleSetpointChange(float setpoint_temp) {
    static float last_setpoint = 0.0f;
    if (fabs(last_setpoint - setpoint_temp) > 0.1f) {
        last_setpoint = setpoint_temp;
        setpoint_time = 0.0f;
        max_overshoot = 0.0f;
        oscillation_count = 0;
        crossed_setpoint = 0;
        previous_error_sign = 0;
        rise_time_marker = -1;
        error_sum = 0.0f;
        samples_count = 0;
        return true;
    }
    return false;
}
static void updateTimers(float dt) {
    setpoint_time += dt;
    adaptation_timer += dt;
}
static bool isWithinDeadband(float error, float deadband) {
    return (error >= -deadband && error <= deadband);  // Inclusive boundaries
}
static void updateIntegralErrorWithAntiWindup(AdaptivePID_Input *input, float error) {
    integral_error += error * input->dt;
    if (integral_error > input->output_max) {
        integral_error = input->output_max;
    } else if (integral_error < input->output_min) {
        integral_error = input->output_min;
    }
}
static float computePIDOutput(AdaptivePID_Input *input, float error, float derivative_error) {
    return (input->Kp * error) + (input->Ki * integral_error) + (input->Kd * derivative_error);
}
static float clampOutput(float output, float min, float max) {
    if (output > max) return max;
    if (output < min) return min;
    return output;
}
static void trackPerformanceMetrics(AdaptivePID_Input *input, float error) {
    error_sum += fabs(error);
    samples_count++;
    if ((previous_error_sign < 0 && error > 0) || (previous_error_sign > 0 && error < 0)) {
        oscillation_count++;
        previous_error_sign = (error > 0) ? 1 : -1;
    } else if (error != 0) {
        previous_error_sign = (error > 0) ? 1 : -1;
    }
    if (!crossed_setpoint && ((prev_temp < input->setpoint_temp && input->current_temp >= input->setpoint_temp) ||
        (prev_temp > input->setpoint_temp && input->current_temp <= input->setpoint_temp))) {
        crossed_setpoint = 1;
    }
    if (crossed_setpoint) {
        float current_overshoot = fabs(error) / fabs(input->setpoint_temp);
        if (current_overshoot > max_overshoot) {
            max_overshoot = current_overshoot;
        }
    }
    if (rise_time_marker < 0 &&
        ((input->current_temp >= input->setpoint_temp && input->setpoint_temp > prev_temp) ||
         (input->current_temp <= input->setpoint_temp && input->setpoint_temp < prev_temp))) {
        rise_time_marker = setpoint_time;
    }
}

/**
 * @brief Update the PID controller and compute the valve command.
 *
 * Performs the PID calculation based on the provided inputs and updates the output.
 * If adaptation is enabled, also adjusts PID parameters based on performance.
 *
 * @param input Pointer to the PID input structure.
 * @param output Pointer to the PID output structure.
 */
void AdaptivePID_Update(AdaptivePID_Input *input, AdaptivePID_Output *output) {
    // Validate inputs - handle NaN and Infinity
    if (isnan(input->current_temp) || isinf(input->current_temp)) {
        // On invalid temperature, maintain previous valve position
        output->valve_command = input->valve_feedback;
        output->error = 0.0f;
        output->integral_error = integral_error;
        output->derivative_error = 0.0f;
        return;
    }

    float error = input->setpoint_temp - input->current_temp;
    bool setpointChanged = handleSetpointChange(input->setpoint_temp);
    if (!setpointChanged) {
        updateTimers(input->dt);
    }
    if (isWithinDeadband(error, input->deadband)) {
        output->valve_command = input->valve_feedback;
        output->error = error;
        output->integral_error = integral_error;
        output->derivative_error = 0.0f;
        return;
    }
    updateIntegralErrorWithAntiWindup(input, error);

    // MEDIUM PRIORITY FIX: Calculate derivative on measurement, not error (Audit Fix #7)
    // This prevents "derivative kick" when setpoint changes
    // Negative sign because we want to resist temperature changes
    float derivative_error = -(input->current_temp - prev_temp) / input->dt;
    float raw_output = computePIDOutput(input, error, derivative_error);
    raw_output = clampOutput(raw_output, input->output_min, input->output_max);
    output->valve_command = raw_output;
    output->error = error;
    output->integral_error = integral_error;
    output->derivative_error = derivative_error;
    if (input->adaptation_enabled) {
        trackPerformanceMetrics(input, error);
        if (adaptation_timer >= adaptation_interval_sec) {
            adaptParameters(input, output, oscillation_count, max_overshoot, error_sum / (samples_count > 0 ? samples_count : 1));
            adaptation_timer = 0.0f;
        }
    }
    prev_error = error;
    prev_temp = input->current_temp;
}

/**
 * @brief Internal function to adapt PID parameters based on observed performance.
 * 
 * Uses a simplified rule-based approach to adjust parameters based on
 * oscillation, overshoot, and steady-state error.
 * 
 * @param input Pointer to the PID input structure.
 * @param output Pointer to the PID output structure.
 * @param oscillations Number of oscillations observed.
 * @param overshoot Maximum overshoot percentage.
 * @param avg_error Average absolute error.
 */
static void adaptParameters(AdaptivePID_Input *input, AdaptivePID_Output *output, 
                           int oscillations, float overshoot, float avg_error) {
    float rate = input->adaptation_rate;
    
    // Too many oscillations - reduce Kp and increase Kd
    if (oscillations > 3) {
        input->Kp *= (1.0f - rate * 0.5f);
        input->Kd *= (1.0f + rate);
        input->Ki *= (1.0f - rate * 0.3f);
    }
    
    // High overshoot - reduce Kp and increase Kd
    if (overshoot > 0.1f) { // More than 10% overshoot
        input->Kp *= (1.0f - rate * 0.7f);
        input->Kd *= (1.0f + rate * 0.5f);
    }
    
    // Steady-state error - increase Ki
    if (avg_error > input->deadband && oscillations < 2) {
        input->Ki *= (1.0f + rate);
    }
    
    // Slow response (high rise time) - increase Kp
    if (rise_time_marker > 10.0f && oscillations < 2 && overshoot < 0.05f) {
        input->Kp *= (1.0f + rate * 0.5f);
    }
    
    // Enforce minimum values for stability
    if (input->Kp < 0.1f) input->Kp = 0.1f;
    if (input->Ki < 0.01f) input->Ki = 0.01f;
    if (input->Kd < 0.01f) input->Kd = 0.01f;

    // MEDIUM PRIORITY FIX: Enforce maximum values to prevent runaway adaptation (Audit Fix #6)
    // These match the limits in the setter functions for consistency
    if (input->Kp > 100.0f) input->Kp = 100.0f;
    if (input->Ki > 10.0f) input->Ki = 10.0f;
    if (input->Kd > 10.0f) input->Kd = 10.0f;

    // Reset counters for next adaptation cycle
    oscillation_count = 0;
    error_sum = 0.0f;
    samples_count = 0;
}

/**
 * @brief Automatically tune PID parameters using simplified Ziegler-Nichols method.
 * 
 * Analyzes temperature history to determine appropriate PID parameters.
 * 
 * @param input Pointer to the PID input structure where parameters will be stored.
 * @param temperature_history Array of historical temperature readings.
 * @param history_size Number of readings in the history array.
 */
void AdaptivePID_AutoTune(AdaptivePID_Input *input, float *temperature_history, int history_size) {
    // This implements a simplified version of the relay method
    // Find the oscillation period and amplitude
    
    float min_temp = temperature_history[0];
    float max_temp = temperature_history[0];
    int peaks = 0;
    int peak_indices[10] = {0}; // Store up to 10 peak indices
    
    // Find min and max temperatures
    for (int i = 0; i < history_size; i++) {
        if (temperature_history[i] < min_temp) min_temp = temperature_history[i];
        if (temperature_history[i] > max_temp) max_temp = temperature_history[i];
    }
    
    // Find peaks (local maxima)
    for (int i = 1; i < history_size - 1 && peaks < 10; i++) {
        if (temperature_history[i] > temperature_history[i-1] && 
            temperature_history[i] > temperature_history[i+1] &&
            temperature_history[i] > min_temp + (max_temp - min_temp) * 0.5f) {
            peak_indices[peaks++] = i;
        }
    }
    
    // If we found at least 2 peaks, we can calculate the period
    if (peaks >= 2) {
        float avg_period = 0.0f;
        for (int i = 1; i < peaks; i++) {
            avg_period += peak_indices[i] - peak_indices[i-1];
        }
        avg_period = (avg_period / (peaks - 1)) * input->dt;
        
        float amplitude = max_temp - min_temp;
        
        // Calculate PID parameters using Ziegler-Nichols method
        float Ku = 4.0f / (3.14159f * amplitude); // Ultimate gain
        float Tu = avg_period;                   // Ultimate period
        
        // Classic Ziegler-Nichols for PID
        input->Kp = 0.6f * Ku;
        float Ti = Tu * 0.5f;
        float Td = Tu * 0.125f;
        input->Ki = input->Kp / Ti;
        input->Kd = input->Kp * Td;
        
        // Apply conservative adjustments for HVAC applications
        // (HVAC systems typically need less aggressive control)
        input->Kp *= 0.5f;
        input->Ki *= 0.3f;
        input->Kd *= 0.7f;
    }
}

/**
 * @brief Analyze controller performance based on temperature history.
 * 
 * Evaluates key performance metrics like settling time, overshoot,
 * steady-state error, oscillations, and rise time.
 * 
 * @param temperature_history Array of historical temperature readings.
 * @param setpoint_history Array of historical setpoint values.
 * @param history_size Number of readings in the history arrays.
 * @param dt Time step between readings.
 * @return AdaptivePID_Performance structure with calculated metrics.
 */
AdaptivePID_Performance AdaptivePID_AnalyzePerformance(float *temperature_history, float *setpoint_history, int history_size, float dt) {
    AdaptivePID_Performance perf = {0};
    
    if (history_size < 2) return perf;
    
    // Assume the setpoint at the end of the history is our target
    float final_setpoint = setpoint_history[history_size - 1];
    
    // Find initial and final errors
    float initial_error = fabs(setpoint_history[0] - temperature_history[0]);
    float final_error = 0.0f;
    
    // Find when the signal first crosses the setpoint (for rise time)
    int rise_time_index = -1;
    
    // Flag for when we've settled within 5% of setpoint
    int settled = 0;
    int settling_time_index = -1;
    
    // Track oscillations
    int oscillations = 0;
    float prev_slope = 0;
    
    // Track maximum overshoot
    float max_overshoot = 0.0f;
    int crossed_setpoint = 0;
    
    // Process the history data
    for (int i = 1; i < history_size; i++) {
        float error = setpoint_history[i] - temperature_history[i];
        float abs_error = fabs(error);
        float setpoint_diff = fabs(setpoint_history[i] - setpoint_history[i-1]);
        
        // Skip time periods where setpoint changed significantly
        if (setpoint_diff > 0.1f) {
            settled = 0;
            crossed_setpoint = 0;
            continue;
        }
        
        // Update final error (average of last few samples if available)
        if (i >= history_size - 10) {
            final_error += abs_error;
        }
        
        // Check for rise time (first time crossing setpoint)
        if (rise_time_index < 0) {
            if ((temperature_history[i-1] < setpoint_history[i-1] && 
                 temperature_history[i] >= setpoint_history[i]) ||
                (temperature_history[i-1] > setpoint_history[i-1] && 
                 temperature_history[i] <= setpoint_history[i])) {
                rise_time_index = i;
            }
        }
        
        // Check for settling within 5% of setpoint
        if (!settled) {
            if (abs_error <= 0.05f * fabs(final_setpoint)) {
                settled = 1;
                settling_time_index = i;
            }
        }
        
        // Track oscillations (slope sign changes)
        float slope = temperature_history[i] - temperature_history[i-1];
        if (i > 1 && ((prev_slope < 0 && slope > 0) || (prev_slope > 0 && slope < 0))) {
            oscillations++;
        }
        prev_slope = slope;
        
        // Track maximum overshoot
        if (rise_time_index >= 0 && !crossed_setpoint) {
            if ((setpoint_history[i] > temperature_history[i-1] && 
                 setpoint_history[i] <= temperature_history[i]) ||
                (setpoint_history[i] < temperature_history[i-1] && 
                 setpoint_history[i] >= temperature_history[i])) {
                crossed_setpoint = 1;
            }
        }
        
        if (crossed_setpoint) {
            float current_overshoot = abs_error / fabs(final_setpoint);
            if (current_overshoot > max_overshoot) {
                max_overshoot = current_overshoot;
            }
        }
    }
    
    // Calculate average final error
    final_error = final_error / (history_size > 10 ? 10 : history_size);
    
    // Fill in the performance structure
    perf.rise_time = (rise_time_index > 0) ? (rise_time_index * dt) : -1;
    perf.settling_time = (settling_time_index > 0) ? (settling_time_index * dt) : -1;
    perf.overshoot = max_overshoot * 100.0f; // Convert to percentage
    perf.steady_state_error = final_error;
    perf.oscillation_count = oscillations / 2; // Each oscillation has 2 slope changes
    
    return perf;
}