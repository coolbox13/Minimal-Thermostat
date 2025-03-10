#include "thermostat_controller.h"
#include <Arduino.h>

ThermostatController::ThermostatController(
    SensorInterface* sensor,
    ControlInterface* controller,
    ThermostatState* state)
    : sensorInterface(sensor),
      controlInterface(controller),
      thermostatState(state),
      lastUpdateTime(0),
      updateInterval(30000) // 30 seconds
{
}

bool ThermostatController::begin() {
    if (!sensorInterface || !controlInterface || !thermostatState) {
        return false;
    }
    
    // Initialize components
    bool sensorOk = sensorInterface->begin();
    bool controlOk = controlInterface->begin();
    
    // Set initial setpoint from state
    controlInterface->setSetpoint(thermostatState->getSetpoint());
    
    return sensorOk && controlOk;
}

void ThermostatController::loop() {
    if (!sensorInterface || !controlInterface || !thermostatState) {
        return;
    }
    
    // Update sensor readings
    sensorInterface->loop();
    
    // Update controller
    controlInterface->loop();
    
    // Periodic update of control logic
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= updateInterval) {
        update();
        lastUpdateTime = currentTime;
    }
}

void ThermostatController::update() {
    // Get current temperature from sensor
    float currentTemp = sensorInterface->getTemperature();
    
    // Update state with current temperature
    thermostatState->setCurrentTemperature(currentTemp);
    
    // Check if heating is needed based on mode and temperature
    bool shouldHeat = shouldActivateHeating();
    
    // Update controller state
    controlInterface->setActive(shouldHeat);
    
    // Set current temperature as input to the controller
    controlInterface->setInput(currentTemp);
    
    // Get output from controller (valve position)
    float output = controlInterface->getOutput();
    
    // Update state with valve position
    thermostatState->setValvePosition(output);
    
    // Update heating state
    thermostatState->setHeatingActive(output > 0);
}

bool ThermostatController::shouldActivateHeating() {
    // Get current mode
    ThermostatMode mode = thermostatState->getMode();
    
    // Get current temperature
    float currentTemp = thermostatState->getCurrentTemperature();
    
    // Get target temperature based on mode
    float targetTemp = thermostatState->getSetpoint();
    
    // Check if heating should be activated based on mode and temperature
    switch (mode) {
        case ThermostatMode::OFF:
            return false;
            
        case ThermostatMode::ANTIFREEZE:
            // Activate heating only if temperature is below antifreeze threshold
            return currentTemp < 5.0f; // 5°C is typical antifreeze temperature
            
        case ThermostatMode::AWAY:
        case ThermostatMode::ECO:
        case ThermostatMode::COMFORT:
            // Activate heating if current temperature is below target
            return currentTemp < targetTemp;
            
        case ThermostatMode::BOOST:
            // Always activate heating in boost mode
            return true;
            
        default:
            return false;
    }
} 