#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "thermostat_state.h"
#include "sensor_interface.h"
#include "pid_controller.h"

class WebInterface {
public:
  // Constructor
  WebInterface();
  
  // Initialize web server
  bool begin(ThermostatState* state, 
             SensorInterface* sensor,
             PIDController* pid);
  
  // Process HTTP requests
  void handle();

private:
  // Web server
  WebServer server;
  
  // References to components
  ThermostatState* thermostatState;
  SensorInterface* sensorInterface;
  PIDController* pidController;
  
  // HTTP request handlers
  void handleRoot();
  void handleGetStatus();
  void handleSetpoint();
  void handleNotFound();
  
  // Setup MDNS
  void setupMDNS();
};

#endif // WEB_INTERFACE_H