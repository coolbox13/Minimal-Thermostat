// Basic includes
#include <Arduino.h>

// Include the ThermostatState first since other components depend on it
#include "thermostat_state.h"

// Then include your component headers
#include "knx_interface.h"
#include "config_manager.h"
#include "pid_controller.h"
#include <esp_log.h>

static const char* TAG = "PIDController";

PIDController::PIDController() :
    setpoint(0.0f),
    input(0.0f),
    output(0.0f),
    active(false),
    lastError(ThermostatStatus::OK),
    prevError(0.0f),
    integral(0.0f),
    derivative(0.0f),
    lastTime(0) {
    // Initialize config with default values
    config.kp = 1.0f;
    config.ki = 0.1f;
    config.kd = 0.01f;
    config.interval = 30000; // 30 seconds
    config.outputMin = 0.0f;
    config.outputMax = 100.0f;
    config.reverse = false;
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

void PIDController::setActive(bool active) {
    if (this->active != active) {
        this->active = active;
        if (active) {
            lastTime = millis();
        }
        reset();
    }
}

ThermostatStatus PIDController::getLastError() const {
    return lastError;
}

void PIDController::reset() {
    integral = 0.0f;
    prevError = 0.0f;
    derivative = 0.0f;
    output = 0.0f;
    lastTime = 0;
    lastError = ThermostatStatus::OK;
}

void PIDController::configure(const void* config) {
    if (config) {
        const PIDConfig* pidConfig = static_cast<const PIDConfig*>(config);
        this->config = *pidConfig;
    }
}

bool PIDController::saveConfig() {
    // No persistent storage implemented yet
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
    output = clamp(output, min, max);
}

void PIDController::setDirection(bool reverse) {
    config.reverse = reverse;
}

void PIDController::computePID() {
    float error = setpoint - input;
    if (config.reverse) {
        error = -error;
    }

    // Proportional term
    float pTerm = config.kp * error;

    // Integral term
    integral += error * (config.interval / 1000.0f);
    float iTerm = config.ki * integral;

    // Derivative term
    derivative = (error - prevError) / (config.interval / 1000.0f);
    float dTerm = config.kd * derivative;

    // Calculate output
    output = pTerm + iTerm + dTerm;
    output = clamp(output, config.outputMin, config.outputMax);

    // Store error for next iteration
    prevError = error;

    ESP_LOGD(TAG, "PID: SP=%.2f, PV=%.2f, P=%.2f, I=%.2f, D=%.2f, OUT=%.2f",
             setpoint, input, pTerm, iTerm, dTerm, output);
}

float PIDController::clamp(float value, float min, float max) const {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}