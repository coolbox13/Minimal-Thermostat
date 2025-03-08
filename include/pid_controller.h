
#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "thermostat_state.h"

class PIDController {
public:
  // Constructor
  PIDController();
  
  // Initialize with thermostat state and PID constants
  void begin(ThermostatState* state, float kp, float ki, float kd);
  
  // Set PID constants
  void setTunings(float kp, float ki, float kd);
  
  // Set update interval
  void setUpdateInterval(unsigned long interval);
  
  // Calculate PID and update valve position (call periodically)
  void update();
  
  // Reset PID (e.g., when setpoint changes)
  void reset();
  
  // Get PID parameters
  float getKp() const;
  float getKi() const;
  float getKd() const;
  
  // Get last calculated values
  float getLastError() const;
  float getProportionalTerm() const;
  float getIntegralTerm() const;
  float getDerivativeTerm() const;
  float getLastOutput() const;

private:
  // Reference to thermostat state
  ThermostatState* thermostatState;
  
  // PID constants
  float kp;
  float ki;
  float kd;
  
  // Timing
  unsigned long lastUpdateTime;
  unsigned long updateInterval;
  
  // PID state variables
  float lastError;
  float integral;
  float lastOutput;
  
  // For debugging/monitoring
  float proportionalTerm;
  float integralTerm;
  float derivativeTerm;
  
  // Calculate PID output
  float calculate();
  
  // Anti-windup protection
  void limitIntegral();
};

#endif // PID_CONTROLLER_H
