/**
 * @file adaptive_pid_controller.h
 * @brief Adaptive PID controller for temperature regulation
 *
 * This module implements a self-tuning PID controller designed specifically
 * for HVAC applications. It features:
 * - Automatic parameter adaptation based on observed performance
 * - Deadband support to prevent oscillation around setpoint
 * - Temperature history tracking for performance analysis
 * - Ziegler-Nichols based auto-tuning
 *
 * @par Control Loop Integration
 * The controller is called from the main loop at PID_UPDATE_INTERVAL (10s).
 * The calculated valve position is sent to KNX/MQTT for actuator control.
 *
 * @par Global State
 * Uses global variables (g_pid_input, g_pid_output) for state to allow
 * access from multiple modules (web API, MQTT, etc.).
 *
 * @par Memory Usage
 * Temperature history buffer: 300 floats = 1.2KB
 * Setpoint history buffer: 300 floats = 1.2KB
 *
 * @see ConfigManager for PID parameter persistence
 * @see KNXManager for valve command transmission
 */

#ifndef ADAPTIVE_PID_CONTROLLER_H
#define ADAPTIVE_PID_CONTROLLER_H

#include <stdint.h>

/**
 * @struct AdaptivePID_Input
 * @brief Input parameters and configuration for the PID controller
 *
 * Contains all inputs needed to compute the control output, including
 * sensor readings, setpoint, tuning parameters, and adaptation settings.
 *
 * @note All temperature values are in degrees Celsius
 * @note Output values are percentages (0-100)
 */
typedef struct {
    float current_temp;         ///< Current room temperature from BME280 (°C)
    float setpoint_temp;        ///< Target temperature from user/preset (°C)
    float valve_feedback;       ///< Actual valve position from KNX feedback (0-100%)
    float Kp;                   ///< Proportional gain - response to current error
    float Ki;                   ///< Integral gain - response to accumulated error
    float Kd;                   ///< Derivative gain - response to rate of change
    float output_min;           ///< Minimum output limit (typically 0%)
    float output_max;           ///< Maximum output limit (typically 100%)
    float deadband;             ///< Temperature tolerance to prevent hunting (°C, default 0.2)
    float dt;                   ///< Sample time between updates (seconds, default 10)

    // Adaptation parameters
    float adaptation_rate;      ///< Learning rate for auto-tuning (0-1, default 0.1)
    uint8_t adaptation_enabled; ///< Enable/disable self-learning (1/0)
} AdaptivePID_Input;

/**
 * @struct AdaptivePID_Output
 * @brief Output values computed by the PID controller
 *
 * Contains the calculated control output and intermediate error terms
 * useful for debugging and visualization.
 */
typedef struct {
    float valve_command;        ///< Calculated valve position to send to actuator (0-100%)
    float error;                ///< Current error: setpoint - current_temp (°C)
    float integral_error;       ///< Accumulated error over time (°C·s)
    float derivative_error;     ///< Rate of error change (°C/s)
} AdaptivePID_Output;

/**
 * @struct AdaptivePID_Performance
 * @brief Performance metrics for controller evaluation
 *
 * Used by the auto-tuning algorithm to evaluate current controller
 * performance and adjust parameters accordingly.
 */
typedef struct {
    float settling_time;        ///< Time to reach within 5% of setpoint (seconds)
    float overshoot;            ///< Maximum overshoot as percentage of step change
    float steady_state_error;   ///< Average error after settling (°C)
    float oscillation_count;    ///< Number of zero-crossings around setpoint
    float rise_time;            ///< Time to first reach setpoint (seconds)
} AdaptivePID_Performance;

/**
 * @name Global Controller State
 * @brief Global variables for PID controller state access
 *
 * These globals allow multiple modules (web server, MQTT, etc.) to read
 * and modify controller state. Access should be done carefully as the
 * controller is not thread-safe.
 * @{
 */

/// @brief Global PID input parameters (readable/writable)
extern AdaptivePID_Input g_pid_input;

/// @brief Global PID output values (read-only except by controller)
extern AdaptivePID_Output g_pid_output;

/** @} */

/**
 * @name Temperature History
 * @brief Circular buffers for auto-tuning and performance analysis
 * @{
 */

/// @brief Number of samples in history buffers (5 minutes at 1 second intervals)
#define HISTORY_SIZE 300

/// @brief Circular buffer of recent temperature readings
extern float g_temperature_history[HISTORY_SIZE];

/// @brief Circular buffer of recent setpoint values
extern float g_setpoint_history[HISTORY_SIZE];

/// @brief Current write position in history buffers (0 to HISTORY_SIZE-1)
extern int g_history_index;

/** @} */

/**
 * @brief Initialize the PID controller with default parameters
 * 
 * Sets up the controller with parameters loaded from ConfigManager.
 * Should be called once at startup.
 */
void initializePIDController(void);

/**
 * @brief Update the PID controller with current system readings
 * 
 * Should be called at regular intervals from the main loop. Takes
 * the current temperature and valve position readings, computes the new
 * control output, and updates the internal state.
 * 
 * @param current_temp Current temperature reading from sensor (°C)
 * @param valve_position Current valve/actuator position (0-100%)
 */
void updatePIDController(float current_temp, float valve_position);

/**
 * @brief Get the current PID control output
 * 
 * @return The calculated valve position command (0-100%)
 */
float getPIDOutput(void);

/**
 * @brief Direct initialization of the PID controller
 * 
 * Allows direct initialization of the PID controller with custom parameters.
 * For more typical use, use initializePIDController() instead.
 * 
 * @param input Pointer to the PID input structure
 */
void AdaptivePID_Init(AdaptivePID_Input *input);

/**
 * @brief Core PID calculation function
 * 
 * Performs the PID calculation and updates the output values.
 * For general use, call updatePIDController() instead.
 * 
 * @param input Pointer to the PID input structure
 * @param output Pointer to the PID output structure
 */
void AdaptivePID_Update(AdaptivePID_Input *input, AdaptivePID_Output *output);

/**
 * @brief Auto-tune the PID parameters
 * 
 * Uses a simplified Ziegler-Nichols method to determine optimal PID
 * parameters based on collected temperature history.
 * 
 * @param input Pointer to the PID input structure to update
 * @param temperature_history Array of historical temperature readings
 * @param history_size Number of readings in the history array
 */
void AdaptivePID_AutoTune(AdaptivePID_Input *input, float *temperature_history, int history_size);

/**
 * @brief Analyze the controller's performance
 * 
 * Evaluates controller performance metrics based on temperature history.
 * 
 * @param temperature_history Array of historical temperature readings
 * @param setpoint_history Array of historical setpoint values
 * @param history_size Number of readings in the history arrays
 * @param dt Time step between readings
 * @return AdaptivePID_Performance structure with calculated metrics
 */
AdaptivePID_Performance AdaptivePID_AnalyzePerformance(float *temperature_history, float *setpoint_history, int history_size, float dt);

/**
 * @brief Set a new temperature setpoint
 * 
 * Changes the target temperature for the controller.
 * 
 * @param setpoint New temperature setpoint in °C
 */
void setTemperatureSetpoint(float setpoint);

/**
 * @brief Set a new proportional gain value
 * 
 * @param kp New proportional gain value
 */
void setPidKp(float kp);

/**
 * @brief Set a new integral gain value
 * 
 * @param ki New integral gain value
 */
void setPidKi(float ki);

/**
 * @brief Set a new derivative gain value
 * 
 * @param kd New derivative gain value
 */
void setPidKd(float kd);

#endif // ADAPTIVE_PID_CONTROLLER_H