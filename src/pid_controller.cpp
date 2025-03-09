#include "pid_controller.h"

PIDController::PIDController() {
    // Initialize configuration with default values
    config.kp = 1.0;
    config.ki = 0.1;
    config.kd = 0.01;
    config.interval = 30000; // 30 seconds
    config.outputMin = 0.0;
    config.outputMax = 100.0;
    config.reverse = false;
    
    // Initialize runtime state
    setpoint = 21.0;
    input = 0.0;
    output = 0.0;
    active = false;
    lastError = ThermostatStatus::OK;
    
    // Initialize PID variables
    prevError = 0.0;
    integral = 0.0;
    derivative = 0.0;
    lastTime = 0;
}

bool PIDController::begin() {
    reset();
    return true;
}

void PIDController::loop() {
    if (!active) return;
    
    unsigned long now = millis();
    if (now - lastTime >= config.interval) {
        computePID();
        lastTime = now;
    }
}

void PIDController::setUpdateInterval(unsigned long interval) {
    config.interval = interval;
}

void PIDController::setSetpoint(float value) {
    setpoint = value;
}

void PIDController::setInput(float value) {
    input = value;
}

float PIDController::getOutput() const {
    return output;
}

bool PIDController::isActive() const {
    return active;
}

void PIDController::setActive(bool state) {
    if (active != state) {
        active = state;
        if (active) {
            // Reset PID state when activating
            reset();
        }
    }
}

ThermostatStatus PIDController::getLastError() const {
    return lastError;
}

void PIDController::reset() {
    integral = 0.0;
    prevError = 0.0;
    derivative = 0.0;
    output = 0.0;
    lastTime = millis();
}

void PIDController::configure(const void* configData) {
    if (configData) {
        const PIDConfig* newConfig = static_cast<const PIDConfig*>(configData);
        config = *newConfig;
    }
}

bool PIDController::saveConfig() {
    // Configuration saving would be implemented here if needed
    return true;
}

void PIDController::setTunings(float kp, float ki, float kd) {
    config.kp = kp;
    config.ki = ki;
    config.kd = kd;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    
    config.outputMin = min;
    config.outputMax = max;
    
    // Clamp current output to new limits
    output = clamp(output, min, max);
    integral = clamp(integral, min, max);
}

void PIDController::setDirection(bool reverse) {
    config.reverse = reverse;
}

void PIDController::computePID() {
    float error = setpoint - input;
    if (config.reverse) error = -error;
    
    // Compute integral with anti-windup
    integral += (config.ki * error * config.interval / 1000.0);
    integral = clamp(integral, config.outputMin, config.outputMax);
    
    // Compute derivative
    derivative = (error - prevError) * 1000.0 / config.interval;
    
    // Compute output
    output = (config.kp * error) + integral + (config.kd * derivative);
    output = clamp(output, config.outputMin, config.outputMax);
    
    // Save error for next iteration
    prevError = error;
}

float PIDController::clamp(float value, float min, float max) const {
    if (value > max) return max;
    if (value < min) return min;
    return value;
} 