#include "persistence_manager.h"

PersistenceManager* PersistenceManager::_instance = nullptr;

PersistenceManager::PersistenceManager() {}

PersistenceManager::~PersistenceManager() {
    end();
}

bool PersistenceManager::begin() {
    return preferences.begin(NAMESPACE, false);
}

void PersistenceManager::end() {
    preferences.end();
}

bool PersistenceManager::setPIDParameters(float kp, float ki, float kd) {
    bool success = true;
    success &= preferences.putFloat(KEY_PID_KP, kp);
    success &= preferences.putFloat(KEY_PID_KI, ki);
    success &= preferences.putFloat(KEY_PID_KD, kd);
    return success;
}

bool PersistenceManager::getPIDParameters(float& kp, float& ki, float& kd) {
    kp = preferences.getFloat(KEY_PID_KP, 1.0);
    ki = preferences.getFloat(KEY_PID_KI, 0.1);
    kd = preferences.getFloat(KEY_PID_KD, 0.05);
    return true;
}

bool PersistenceManager::setSetpointTemperature(float temp) {
    return preferences.putFloat(KEY_SETPOINT, temp);
}

float PersistenceManager::getSetpointTemperature() {
    return preferences.getFloat(KEY_SETPOINT, 21.0);
}

bool PersistenceManager::setAdaptationRate(float rate) {
    return preferences.putFloat(KEY_ADAPT_RATE, rate);
}

float PersistenceManager::getAdaptationRate() {
    return preferences.getFloat(KEY_ADAPT_RATE, 0.01);
}

bool PersistenceManager::setAdaptationEnabled(bool enabled) {
    return preferences.putBool(KEY_ADAPT_ENABLED, enabled);
}

bool PersistenceManager::getAdaptationEnabled() {
    return preferences.getBool(KEY_ADAPT_ENABLED, true);
}

bool PersistenceManager::setDeadband(float deadband) {
    return preferences.putFloat(KEY_DEADBAND, deadband);
}

float PersistenceManager::getDeadband() {
    return preferences.getFloat(KEY_DEADBAND, 0.5);
}

bool PersistenceManager::setLastValvePosition(float position) {
    return preferences.putFloat(KEY_VALVE_POS, position);
}

float PersistenceManager::getLastValvePosition() {
    return preferences.getFloat(KEY_VALVE_POS, 0.0);
}

void PersistenceManager::printStoredValues() {
    Serial.println("\nStored Persistence Values:");
    Serial.println("------------------------");
    Serial.printf("PID Parameters: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", 
        preferences.getFloat(KEY_PID_KP, 1.0),
        preferences.getFloat(KEY_PID_KI, 0.1),
        preferences.getFloat(KEY_PID_KD, 0.05));
    Serial.printf("Setpoint Temperature: %.2f°C\n", preferences.getFloat(KEY_SETPOINT, 21.0));
    Serial.printf("Adaptation Rate: %.3f\n", preferences.getFloat(KEY_ADAPT_RATE, 0.01));
    Serial.printf("Adaptation Enabled: %s\n", preferences.getBool(KEY_ADAPT_ENABLED, true) ? "Yes" : "No");
    Serial.printf("Deadband: %.2f°C\n", preferences.getFloat(KEY_DEADBAND, 0.5));
    Serial.printf("Last Valve Position: %.1f%%\n", preferences.getFloat(KEY_VALVE_POS, 0.0));
    Serial.println("------------------------\n");
}

JsonObject PersistenceManager::getStoredValues(JsonDocument& doc) {
    JsonObject root = doc.to<JsonObject>();
    
    JsonObject pid = root.createNestedObject("pid");
    pid["kp"] = preferences.getFloat(KEY_PID_KP, 1.0);
    pid["ki"] = preferences.getFloat(KEY_PID_KI, 0.1);
    pid["kd"] = preferences.getFloat(KEY_PID_KD, 0.05);
    
    root["setpoint"] = preferences.getFloat(KEY_SETPOINT, 21.0);
    root["adaptation_rate"] = preferences.getFloat(KEY_ADAPT_RATE, 0.01);
    root["adaptation_enabled"] = preferences.getBool(KEY_ADAPT_ENABLED, true);
    root["deadband"] = preferences.getFloat(KEY_DEADBAND, 0.5);
    root["valve_position"] = preferences.getFloat(KEY_VALVE_POS, 0.0);
    
    return root;
}