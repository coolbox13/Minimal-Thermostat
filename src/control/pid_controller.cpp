#include "pid_controller.h"
#include <Arduino.h>
#include "thermostat_state.h"
#include <esp_log.h>

static const char* TAG = "PIDController";

PIDController::PIDController() {
    config.kp = 1.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;
    config.outputMin = 0.0f;
    config.outputMax = 100.0f;
    config.sampleTime = 30000.0f;
    setpoint = 0.0f;
    input = 0.0f;
    output = 0.0f;
    integral = 0.0f;
    prevError = 0.0f;
    lastTime = 0;
    firstRun = true;
    active = false;
    lastError = ThermostatStatus::OK;
}

bool PIDController::begin() {
    lastTime = millis();
    firstRun = true;
    return true;
}

void PIDController::loop() {
    if (!active) return;

    unsigned long now = millis();
    if (now - lastTime >= static_cast<unsigned long>(config.sampleTime)) {
        computePID();
        lastTime = now;
    }
}

void PIDController::configure(const void* configData) {
    if (configData) {
        const PIDConfig* newConfig = static_cast<const PIDConfig*>(configData);
        config = *newConfig;
        resetIntegral();
    }
}

void PIDController::setUpdateInterval(unsigned long interval) {
    config.sampleTime = static_cast<float>(interval);
}

void PIDController::setSetpoint(float value) {
    if (setpoint != value) {
        setpoint = value;
        resetIntegral();
    }
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
            lastTime = millis();
        }
        resetIntegral();
    }
}

ThermostatStatus PIDController::getLastError() const {
    return lastError;
}

void PIDController::reset() {
    resetIntegral();
}

bool PIDController::saveConfig() {
    return true;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    
    config.outputMin = min;
    config.outputMax = max;
    
    if (output > max) {
        output = max;
    } else if (output < min) {
        output = min;
    }
}

void PIDController::setDirection(bool reverse) {
    // Not used in this implementation
}

void PIDController::computePID() {
    if (firstRun) {
        prevError = setpoint - input;
        firstRun = false;
        return;
    }

    float error = setpoint - input;
    float dt = config.sampleTime / 1000.0f;

    float proportional = config.kp * error;
    integral += error * dt;
    float integralTerm = config.ki * integral;
    float derivative = (error - prevError) / dt;
    float derivativeTerm = config.kd * derivative;

    output = proportional + integralTerm + derivativeTerm;

    if (output > config.outputMax) {
        output = config.outputMax;
    } else if (output < config.outputMin) {
        output = config.outputMin;
    }

    prevError = error;
}

void PIDController::resetIntegral() {
    integral = 0.0f;
    firstRun = true;
}
