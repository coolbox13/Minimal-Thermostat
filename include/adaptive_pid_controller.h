// adaptive_pid_controller.h
#ifndef ADAPTIVE_PID_CONTROLLER_H
#define ADAPTIVE_PID_CONTROLLER_H

#include <stdint.h>

// Data structure for PID inputs
typedef struct {
    float current_temp;         // Current room temperature (°C)
    float setpoint_temp;        // Desired room temperature (°C)
    float valve_feedback;       // Actual valve position (0-100%)
    float Kp, Ki, Kd;           // PID gains
    float output_min, output_max; // Output limits (e.g., 0-100%)
    float deadband;             // Deadband tolerance (°C)
    float dt;                   // Sample time (seconds)
    
    // Adaptation parameters
    float adaptation_rate;      // How quickly to adapt (0-1)
    uint8_t adaptation_enabled; // Enable/disable self-learning (1/0)
} AdaptivePID_Input;

// Data structure for PID outputs
typedef struct {
    float valve_command;        // Calculated valve position (0-100%)
    float error;                // Current error (°C)
    float integral_error;       // Accumulated error
    float derivative_error;     // Rate of change of error
} AdaptivePID_Output;

// Data structure for performance metrics
typedef struct {
    float settling_time;        // Time to reach within 5% of setpoint
    float overshoot;            // Maximum overshoot percentage
    float steady_state_error;   // Average error after settling
    float oscillation_count;    // Number of oscillations around setpoint
    float rise_time;            // Time to first reach setpoint
} AdaptivePID_Performance;

// Global controller state (used by functions)
extern AdaptivePID_Input g_pid_input;
extern AdaptivePID_Output g_pid_output;

// Temperature history for auto-tuning
#define HISTORY_SIZE 300  // 5 minutes at 1 second interval
extern float g_temperature_history[HISTORY_SIZE];
extern float g_setpoint_history[HISTORY_SIZE];
extern int g_history_index;

// Function prototypes
void initializePIDController(void);
void updatePIDController(float current_temp, float valve_position);
float getPIDOutput(void);

// These are kept for direct access if needed
void AdaptivePID_Init(AdaptivePID_Input *input); 
void AdaptivePID_Update(AdaptivePID_Input *input, AdaptivePID_Output *output);
void AdaptivePID_AutoTune(AdaptivePID_Input *input, float *temperature_history, int history_size);
AdaptivePID_Performance AdaptivePID_AnalyzePerformance(float *temperature_history, float *setpoint_history, int history_size, float dt);

// Set new temperature setpoint
void setTemperatureSetpoint(float setpoint);

#endif // ADAPTIVE_PID_CONTROLLER_H