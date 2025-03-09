#include "pid_controller.h"

PIDController::PIDController() 
    : setpoint(20.0f)  // Default room temperature setpoint
    , input(0.0f)
    , output(0.0f)
    , active(false)
    , lastError(ThermostatStatus::OK)
    , integral(0.0f)
    , derivative(0.0f)
    , lastTime(0) {
    // Default PID configuration
    config.kp = 2.0f;
    config.ki = 0.5f;
    config.kd = 1.0f;
    config.interval = 1000;  // 1 second default interval
    config.outputMin = 0.0f;
    config.outputMax = 100.0f;
    config.reverse = false;
}

bool PIDController::begin() {
    reset();
    return true;
}

void PIDController::loop() {
    if (!active) {
        return;
    }

    unsigned long now = millis();
    if (now - lastTime >= config.interval) {
        computePID();
        lastTime = now;
    }
}

void PIDController::computePID() {
    float error = setpoint - input;
    if (config.reverse) {
        error = -error;
    }

    // Compute integral with anti-windup
    integral += (config.ki * error);
    integral = clamp(integral, config.outputMin, config.outputMax);

    // Compute derivative
    derivative = (error - lastError) * config.kd;
    lastError = error;

    // Compute PID output
    output = (config.kp * error) + integral + derivative;
    output = clamp(output, config.outputMin, config.outputMax);
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

void PIDController::setActive(bool value) {
    if (active != value) {
        active = value;
        if (active) {
            lastTime = millis();  // Reset timing on activation
        }
    }
}

ThermostatStatus PIDController::getLastError() const {
    return lastError;
}

void PIDController::reset() {
    integral = 0.0f;
    derivative = 0.0f;
    lastError = 0.0f;
    output = 0.0f;
    lastTime = millis();
}

void PIDController::configure(const void* configData) {
    if (configData) {
        const PIDConfig* newConfig = static_cast<const PIDConfig*>(configData);
        config = *newConfig;
        reset();  // Reset controller state with new configuration
    }
}

bool PIDController::saveConfig() {
    // TODO: Implement configuration saving to non-volatile storage
    return true;
}

void PIDController::setTunings(float kp, float ki, float kd) {
    config.kp = kp;
    config.ki = ki;
    config.kd = kd;
    reset();  // Reset controller state with new tunings
}

void PIDController::setOutputLimits(float min, float max) {
    if (min < max) {
        config.outputMin = min;
        config.outputMax = max;
        output = clamp(output, min, max);  // Re-clamp current output
        integral = clamp(integral, min, max);  // Re-clamp integral term
    }
}

void PIDController::setDirection(bool reverse) {
    if (config.reverse != reverse) {
        config.reverse = reverse;
        reset();  // Reset controller state with new direction
    }
}

float PIDController::clamp(float value, float min, float max) const {
    if (value < min) return min;
    if (value > max) return max;
    return value;
} 