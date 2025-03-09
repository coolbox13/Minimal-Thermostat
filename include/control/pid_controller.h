#pragma once

struct PIDConfig {
    float kp;           // Proportional gain
    float ki;           // Integral gain
    float kd;           // Derivative gain
    float minOutput;    // Minimum output value
    float maxOutput;    // Maximum output value
    float sampleTime;   // Sample time in milliseconds
    float interval;     // Update interval in milliseconds
};

class PIDController {
public:
    PIDController();
    virtual ~PIDController() = default;

    virtual void begin();
    virtual void loop();
    virtual void configure(const PIDConfig& config);
    virtual void setUpdateInterval(unsigned long interval);
    virtual void setSetpoint(float setpoint);
    virtual void setInput(float input);
    virtual float getOutput() const;
    virtual float getKp() const { return config.kp; }
    virtual float getKi() const { return config.ki; }
    virtual float getKd() const { return config.kd; }
    virtual float getMinOutput() const { return config.minOutput; }
    virtual float getMaxOutput() const { return config.maxOutput; }
    virtual float getSampleTime() const { return config.sampleTime; }
    virtual float getInterval() const { return config.interval; }

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
}; 