
// PID constants for thermostat control

//const float Kp = 1.0;
//const float Ki = 0.1;
//const float Kd = 0.01;


// Function to set the thermostat to a specified temperature using PID control
void pidThermostat() {

/*   In case when needed
  extern float proportional;
  extern float integral;
  extern float derivative;
  extern float setpoint;
  extern float temperature;
*/
  extern float Kproportional;
  extern float Kintegral;
  extern float Kderivative;
  extern float setpoint;
  extern float temperature;
  extern float previousError;
  extern float integral;

  // Calculate the error between the current and target temperatures
  float error = setpoint - temperature;

  // Update the integral term
  integral += error * Kintegral;

  // Calculate the derivative term
  float derivative = (error - previousError) * Kderivative;

  // Calculate the PID output
  float output = Kproportional * error + integral + derivative;

  // Set the thermostat to the calculated output temperature
  sendPID_Output(output);

  // Update the previous error
  previousError = error;
}

