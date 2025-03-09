#include "thermostat_controller.h"
#include <esp_log.h>

static const char* TAG = "ThermostatController";

ThermostatController::ThermostatController(PIDController* pid, SensorInterface* sensor)
    : currentTemp(0)
    , targetTemp(21)
    , output(0)
    , isHeating(false)
    , mode(ThermostatMode::OFF)
    , pidController(pid)
    , sensorInterface(sensor)
    , hysteresis(0.5) {
}

ThermostatController::~ThermostatController() {
    // PIDController and SensorInterface are owned by the main application
}

void ThermostatController::begin() {
    if (sensorInterface) {
        sensorInterface->begin();
    }
    if (pidController) {
        pidController->begin();
    }
}

void ThermostatController::update() {
    if (!pidController || !pidController->isActive()) {
        return;
    }

    float currentTemp = sensorInterface->getTemperature();
    float targetTemp = targetTemp;

    // Update PID controller
    pidController->setSetpoint(targetTemp);
    pidController->setInput(currentTemp);
    pidController->loop();

    // Get and apply the output
    float output = pidController->getOutput();
    sensorInterface->setValvePosition(output);

    ESP_LOGD(TAG, "Thermostat: Target=%.2f, Current=%.2f, Valve=%.2f",
             targetTemp, currentTemp, output);
} 