#include "pid_controller.h"
#include <Arduino.h>

PIDController::PIDController() : active(false), lastTime(0), lastError(ThermostatStatus::OK) {
    config.kp = 2.0f;
    config.ki = 0.5f;
    config.kd = 1.0f;
    config.minOutput = 0.0f;
    config.maxOutput = 100.0f;
    config.sampleTime = 30000.0f; // 30 seconds
    setpoint = 0.0f;
    input = 0.0f;
    output = 0.0f;
    integral = 0.0f;
    lastInput = 0.0f;
}

bool PIDController::begin() {
    lastTime = millis();
    integral = 0.0f;
    lastInput = input;
    active = true;
    return true;
}

void PIDController::loop() {
    if (!active) return;
    
    unsigned long now = millis();
    unsigned long timeChange = (now - lastTime);
    
    if (timeChange >= config.sampleTime) {
        // Compute PID output
        output = computePID();
        
        // Remember for next time
        lastTime = now;
    }
}

void PIDController::configure(const void* configData) {
    if (configData) {
        const PIDConfig* newConfig = static_cast<const PIDConfig*>(configData);
        config = *newConfig;
    }
}

void PIDController::setSetpoint(float sp) {
    setpoint = sp;
}

void PIDController::setInput(float in) {
    input = in;
}

float PIDController::getOutput() const {
    return output;
}

void PIDController::setActive(bool state) {
    if (active != state) {
        active = state;
        if (active) {
            // Reset integral when activating
            resetIntegral();
        }
    }
}

bool PIDController::isActive() const {
    return active;
}

ThermostatStatus PIDController::getLastError() const {
    return lastError;
}

float PIDController::computePID() {
    // Calculate error
    float error = setpoint - input;
    
    // Calculate the proportional term
    float pTerm = config.kp * error;
    
    // Calculate the integral term
    integral += (config.ki * error);
    
    // Limit integral to prevent windup
    if (integral > config.maxOutput) integral = config.maxOutput;
    else if (integral < config.minOutput) integral = config.minOutput;
    
    // Calculate the derivative term
    float dInput = input - lastInput;
    float dTerm = -config.kd * dInput; // Negative because dInput = input - lastInput
    
    // Remember last input for next time
    lastInput = input;
    
    // Calculate total output
    float result = pTerm + integral + dTerm;
    
    // Limit output
    if (result > config.maxOutput) result = config.maxOutput;
    else if (result < config.minOutput) result = config.minOutput;
    
    return result;
}

void PIDController::resetIntegral() {
    integral = 0.0f;
    lastInput = input;
}

void PIDController::setUpdateInterval(unsigned long interval) {
    config.sampleTime = static_cast<float>(interval);
}

void PIDController::reset() {
    resetIntegral();
}

void PIDController::setTunings(float kp, float ki, float kd) {
    config.kp = kp;
    config.ki = ki;
    config.kd = kd;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    
    config.minOutput = min;
    config.maxOutput = max;
    
    // Clamp current output to new limits
    output = clamp(output, min, max);
    integral = clamp(integral, min, max);
}

void PIDController::setDirection(bool reverse) {
    // This method is no longer used in the new implementation
}

float PIDController::clamp(float value, float min, float max) const {
    if (value > max) return max;
    if (value < min) return min;
    return value;
} 