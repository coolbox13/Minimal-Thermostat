#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "interfaces/control_interface.h"

// PID configuration structure
struct PIDConfig {
    float kp;
    float ki;
    float kd;
    unsigned long interval;
    float outputMin;
    float outputMax;
    bool reverse;
};

class PIDController : public ControlInterface {
public:
    // Constructor
    PIDController();

    // ControlInterface implementation
    bool begin() override;
    void loop() override;
    void setUpdateInterval(unsigned long interval) override;
    void setSetpoint(float value) override;
    void setInput(float value) override;
    float getOutput() const override;
    bool isActive() const override;
    void setActive(bool active) override;
    ThermostatStatus getLastError() const override;
    void reset() override;
    void configure(const void* config) override;
    bool saveConfig() override;

    // PID specific methods
    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void setDirection(bool reverse);
    
    // PID parameter getters
    float getKp() const { return config.kp; }
    float getKi() const { return config.ki; }
    float getKd() const { return config.kd; }
    float getOutputMin() const { return config.outputMin; }
    float getOutputMax() const { return config.outputMax; }
    bool isReverse() const { return config.reverse; }

private:
    // Configuration
    PIDConfig config;
    
    // Runtime state
    float setpoint;
    float input;
    float output;
    bool active;
    ThermostatStatus lastError;
    
    // PID calculation variables
    float lastError;
    float integral;
    float derivative;
    unsigned long lastTime;
    
    // Internal helpers
    void computePID();
    float clamp(float value, float min, float max) const;
};

#endif // PID_CONTROLLER_H
