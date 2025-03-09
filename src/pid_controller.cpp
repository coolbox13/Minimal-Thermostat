#include "pid_controller.h"

PIDController::PIDController() {
    config.kp = 1.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;
    config.minOutput = 0.0f;
    config.maxOutput = 100.0f;
    config.sampleTime = 30000.0f; // 30 seconds
    setpoint = 0.0f;
    input = 0.0f;
    output = 0.0f;
    integral = 0.0f;
    prevError = 0.0f;
    lastTime = 0;
    firstRun = true;
}

void PIDController::begin() {
    lastTime = millis();
    firstRun = true;
}

void PIDController::loop() {
    unsigned long now = millis();
    if (now - lastTime >= static_cast<unsigned long>(config.sampleTime)) {
        computePID();
        lastTime = now;
    }
}

void PIDController::configure(const PIDConfig& newConfig) {
    config = newConfig;
    resetIntegral();
}

void PIDController::setUpdateInterval(unsigned long interval) {
    config.sampleTime = static_cast<float>(interval);
}

void PIDController::setSetpoint(float newSetpoint) {
    if (setpoint != newSetpoint) {
        setpoint = newSetpoint;
        resetIntegral();
    }
}

void PIDController::setInput(float newInput) {
    input = newInput;
}

float PIDController::getOutput() const {
    return output;
}

bool PIDController::isActive() const {
    return true; // Assuming the controller is always active
}

void PIDController::setActive(bool state) {
    // This method is no longer used in the new implementation
}

ThermostatStatus PIDController::getLastError() const {
    return ThermostatStatus::OK; // Assuming the controller is always OK
}

void PIDController::reset() {
    resetIntegral();
}

void PIDController::computePID() {
    if (firstRun) {
        prevError = setpoint - input;
        firstRun = false;
        return;
    }

    float error = setpoint - input;
    float dt = config.sampleTime / 1000.0f;

    // Proportional term
    float proportional = config.kp * error;

    // Integral term
    integral += error * dt;
    float integralTerm = config.ki * integral;

    // Derivative term
    float derivative = (error - prevError) / dt;
    float derivativeTerm = config.kd * derivative;

    // Calculate output
    output = proportional + integralTerm + derivativeTerm;

    // Clamp output
    if (output > config.maxOutput) {
        output = config.maxOutput;
    } else if (output < config.minOutput) {
        output = config.minOutput;
    }

    // Save error for next iteration
    prevError = error;
}

void PIDController::resetIntegral() {
    integral = 0.0f;
    firstRun = true;
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