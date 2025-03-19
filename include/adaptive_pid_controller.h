// adaptive_pid_controller.h
#ifndef ADAPTIVE_PID_CONTROLLER_H
#define ADAPTIVE_PID_CONTROLLER_H

#include <stdint.h>

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
 */

/**
 * @brief Data structure for PID controller inputs
 */
typedef struct {
    float current_temp;         /**< Current room temperature (°C) */
    float setpoint_temp;        /**< Desired room temperature (°C) */
    float valve_feedback;       /**< Actual valve position (0-100%) */
    float Kp, Ki, Kd;           /**< PID gains */
    float output_min, output_max; /**< Output limits (e.g., 0-100%) */
    float deadband;             /**< Deadband tolerance (°C) */
    float dt;                   /**< Sample time (seconds) */
    
    // Adaptation parameters
    float adaptation_rate;      /**< How quickly to adapt (0-1) */
    uint8_t adaptation_enabled; /**< Enable/disable self-learning (1/0) */
} AdaptivePID_Input;

/**
 * @brief Data structure for PID controller outputs
 */
typedef struct {
    float valve_command;        /**< Calculated valve position (0-100%) */
    float error;                /**< Current error (°C) */
    float integral_error;       /**< Accumulated error */
    float derivative_error;     /**< Rate of change of error */
} AdaptivePID_Output;

/**
 * @brief Data structure for controller performance metrics
 */
typedef struct {
    float settling_time;        /**< Time to reach within 5% of setpoint */
    float overshoot;            /**< Maximum overshoot percentage */
    float steady_state_error;   /**< Average error after settling */
    float oscillation_count;    /**< Number of oscillations around setpoint */
    float rise_time;            /**< Time to first reach setpoint */
} AdaptivePID_Performance;

// Global controller state (used by functions)
extern AdaptivePID_Input g_pid_input;
extern AdaptivePID_Output g_pid_output;

// Temperature history for auto-tuning
#define HISTORY_SIZE 300  /**< 5 minutes at 1 second interval */
extern float g_temperature_history[HISTORY_SIZE];
extern float g_setpoint_history[HISTORY_SIZE];
extern int g_history_index;

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