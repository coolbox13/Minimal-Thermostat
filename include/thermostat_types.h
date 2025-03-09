#ifndef THERMOSTAT_TYPES_H
#define THERMOSTAT_TYPES_H

// Operating modes for the thermostat
enum class ThermostatMode {
    OFF = 0,
    COMFORT = 1,
    STANDBY = 2,
    ECONOMY = 3,
    PROTECTION = 4
};

// Temperature ranges and limits
struct ThermostatLimits {
    static constexpr float MIN_TEMPERATURE = 5.0f;
    static constexpr float MAX_TEMPERATURE = 30.0f;
    static constexpr float DEFAULT_TEMPERATURE = 21.0f;
    static constexpr float MIN_VALVE_POSITION = 0.0f;
    static constexpr float MAX_VALVE_POSITION = 100.0f;
};

// Status codes for operations
enum class ThermostatStatus {
    OK = 0,
    ERROR_SENSOR = -1,
    ERROR_COMMUNICATION = -2,
    ERROR_CONFIGURATION = -3
};

#endif // THERMOSTAT_TYPES_H 