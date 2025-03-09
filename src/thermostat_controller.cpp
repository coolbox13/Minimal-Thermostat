#include "thermostat_controller.h"

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
    if (!sensorInterface || !pidController) {
        return;
    }

    // Update current temperature
    currentTemp = sensorInterface->getTemperature();

    // Update PID controller
    if (mode != ThermostatMode::OFF) {
        pidController->setSetpoint(targetTemp);
        output = pidController->compute(currentTemp);

        switch (mode) {
            case ThermostatMode::HEAT:
                isHeating = currentTemp < (targetTemp - hysteresis);
                break;
            case ThermostatMode::COOL:
                isHeating = currentTemp > (targetTemp + hysteresis);
                break;
            case ThermostatMode::AUTO:
                if (currentTemp < (targetTemp - hysteresis)) {
                    isHeating = true;
                } else if (currentTemp > (targetTemp + hysteresis)) {
                    isHeating = false;
                }
                break;
            default:
                isHeating = false;
                break;
        }
    } else {
        isHeating = false;
        output = 0;
    }
} 