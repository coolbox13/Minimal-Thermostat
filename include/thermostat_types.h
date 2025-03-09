#ifndef THERMOSTAT_TYPES_H
#define THERMOSTAT_TYPES_H

// Operating modes for the thermostat
enum class ThermostatMode {
    OFF = 0,        // System is off
    COMFORT = 1,    // Normal comfort mode
    STANDBY = 2,    // Reduced temperature mode
    ECONOMY = 3,    // Energy saving mode
    PROTECTION = 4  // Building protection mode (frost/heat)
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
    ERROR_CONFIGURATION = -3,
    ERROR_FILESYSTEM = -4
};

#endif // THERMOSTAT_TYPES_H 