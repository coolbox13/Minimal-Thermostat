#ifndef PERSISTENCE_MANAGER_H
#define PERSISTENCE_MANAGER_H

#include <Preferences.h>
#include <ArduinoJson.h>

class PersistenceManager {
public:
    static PersistenceManager* getInstance() {
        if (_instance == nullptr) {
            _instance = new PersistenceManager();
        }
        return _instance;
    }

private:
    static PersistenceManager* _instance;

    // Make constructor private for singleton pattern
    PersistenceManager();

    // Delete copy constructor and assignment operator
    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;
public:
    ~PersistenceManager();

    bool begin();
    void end();

    // PID parameters
    bool setPIDParameters(float kp, float ki, float kd);
    bool getPIDParameters(float& kp, float& ki, float& kd);

    // Setpoint temperature
    bool setSetpointTemperature(float temp);
    float getSetpointTemperature();

    // Adaptation parameters
    bool setAdaptationRate(float rate);
    float getAdaptationRate();
    bool setAdaptationEnabled(bool enabled);
    bool getAdaptationEnabled();

    // Deadband
    bool setDeadband(float deadband);
    float getDeadband();

    // Valve position
    bool setLastValvePosition(float position);
    float getLastValvePosition();

    // Debug and monitoring
    void printStoredValues();
    JsonObject getStoredValues(JsonDocument& doc);

private:
    Preferences preferences;
    static constexpr const char* NAMESPACE = "thermostat";
    
    // Storage keys
    static constexpr const char* KEY_PID_KP = "pid_kp";
    static constexpr const char* KEY_PID_KI = "pid_ki";
    static constexpr const char* KEY_PID_KD = "pid_kd";
    static constexpr const char* KEY_SETPOINT = "setpoint";
    static constexpr const char* KEY_ADAPT_RATE = "adapt_rate";
    static constexpr const char* KEY_ADAPT_ENABLED = "adapt_en";
    static constexpr const char* KEY_DEADBAND = "deadband";
    static constexpr const char* KEY_VALVE_POS = "valve_pos";
};

#endif // PERSISTENCE_MANAGER_H