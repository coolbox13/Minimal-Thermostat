#pragma once

#include <Arduino.h>
#include "interfaces/control_interface.h"

// PID configuration structure
struct PIDConfig {
    float kp;           // Proportional gain
    float ki;           // Integral gain
    float kd;           // Derivative gain
    float outputMin;    // Minimum output value
    float outputMax;    // Maximum output value
    float sampleTime;   // Sample time in milliseconds
};

class PIDController : public ControlInterface {
public:
    PIDController();
    virtual ~PIDController() = default;

    // ControlInterface implementation
    bool begin() override;
    void loop() override;
    void configure(const void* config) override;
    void setSetpoint(float value) override;
    void setInput(float value) override;
    float getOutput() const override;
    bool isActive() const override;
    void setActive(bool state) override;
    ThermostatStatus getLastError() const override;
    void reset() override;
    bool saveConfig() override;

    // PID specific methods
    void setOutputLimits(float min, float max);
    void setDirection(bool reverse);
    float getKp() const { return config.kp; }
    float getKi() const { return config.ki; }
    float getKd() const { return config.kd; }
    float getOutputMin() const { return config.outputMin; }
    float getOutputMax() const { return config.outputMax; }
    float getSampleTime() const { return config.sampleTime; }

protected:
    void computePID();
    void resetIntegral();

    PIDConfig config;
    float setpoint;
    float input;
    float output;
    float integral;
    float prevError;
    unsigned long lastTime;
    bool firstRun;
    bool active;
    ThermostatStatus lastError;
};
