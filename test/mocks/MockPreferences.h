#ifndef MOCK_PREFERENCES_H
#define MOCK_PREFERENCES_H

#include <stdint.h>
#include <map>
#include <string>

/**
 * Mock implementation of ESP32 Preferences class for testing
 * Stores data in memory instead of NVS flash
 * Uses static storage so all instances share the same data (like real Preferences)
 */
class MockPreferences {
private:
    // Static storage shared across all instances (simulates NVS flash)
    static std::map<std::string, int> intValues;
    static std::map<std::string, unsigned int> uintValues;
    static std::map<std::string, long> longValues;
    static std::map<std::string, unsigned long> ulongValues;
    static std::map<std::string, float> floatValues;
    static std::map<std::string, double> doubleValues;
    static std::map<std::string, std::string> stringValues;
    static std::map<std::string, bool> boolValues;
    static std::map<std::string, uint8_t> ucharValues;
    static std::map<std::string, uint16_t> ushortValues;

    bool isOpen;
    std::string namespaceName;

public:
    MockPreferences() : isOpen(false) {}

    bool begin(const char* name, bool readOnly = false) {
        namespaceName = name;
        isOpen = true;
        return true;
    }

    void end() {
        isOpen = false;
    }

    bool clear() {
        intValues.clear();
        uintValues.clear();
        longValues.clear();
        ulongValues.clear();
        floatValues.clear();
        doubleValues.clear();
        stringValues.clear();
        boolValues.clear();
        ucharValues.clear();
        ushortValues.clear();
        return true;
    }

    bool remove(const char* key) {
        std::string k(key);
        intValues.erase(k);
        uintValues.erase(k);
        longValues.erase(k);
        ulongValues.erase(k);
        floatValues.erase(k);
        doubleValues.erase(k);
        stringValues.erase(k);
        boolValues.erase(k);
        ucharValues.erase(k);
        ushortValues.erase(k);
        return true;
    }

    // Int methods
    int getInt(const char* key, int defaultValue = 0) {
        if (intValues.count(key)) return intValues[key];
        return defaultValue;
    }

    size_t putInt(const char* key, int value) {
        intValues[key] = value;
        return sizeof(int);
    }

    // UInt methods
    unsigned int getUInt(const char* key, unsigned int defaultValue = 0) {
        if (uintValues.count(key)) return uintValues[key];
        return defaultValue;
    }

    size_t putUInt(const char* key, unsigned int value) {
        uintValues[key] = value;
        return sizeof(unsigned int);
    }

    // Long methods
    long getLong(const char* key, long defaultValue = 0) {
        if (longValues.count(key)) return longValues[key];
        return defaultValue;
    }

    size_t putLong(const char* key, long value) {
        longValues[key] = value;
        return sizeof(long);
    }

    // ULong methods
    unsigned long getULong(const char* key, unsigned long defaultValue = 0) {
        if (ulongValues.count(key)) return ulongValues[key];
        return defaultValue;
    }

    size_t putULong(const char* key, unsigned long value) {
        ulongValues[key] = value;
        return sizeof(unsigned long);
    }

    // Float methods
    float getFloat(const char* key, float defaultValue = 0.0f) {
        if (floatValues.count(key)) return floatValues[key];
        return defaultValue;
    }

    size_t putFloat(const char* key, float value) {
        floatValues[key] = value;
        return sizeof(float);
    }

    // Double methods
    double getDouble(const char* key, double defaultValue = 0.0) {
        if (doubleValues.count(key)) return doubleValues[key];
        return defaultValue;
    }

    size_t putDouble(const char* key, double value) {
        doubleValues[key] = value;
        return sizeof(double);
    }

    // String methods
    std::string getString(const char* key, const std::string& defaultValue = "") {
        if (stringValues.count(key)) return stringValues[key];
        return defaultValue;
    }

    size_t putString(const char* key, const std::string& value) {
        stringValues[key] = value;
        return value.length();
    }

    size_t putString(const char* key, const char* value) {
        stringValues[key] = value;
        return strlen(value);
    }

    // Bool methods
    bool getBool(const char* key, bool defaultValue = false) {
        if (boolValues.count(key)) return boolValues[key];
        return defaultValue;
    }

    size_t putBool(const char* key, bool value) {
        boolValues[key] = value;
        return sizeof(bool);
    }

    // UChar methods
    uint8_t getUChar(const char* key, uint8_t defaultValue = 0) {
        if (ucharValues.count(key)) return ucharValues[key];
        return defaultValue;
    }

    size_t putUChar(const char* key, uint8_t value) {
        ucharValues[key] = value;
        return sizeof(uint8_t);
    }

    // UShort methods
    uint16_t getUShort(const char* key, uint16_t defaultValue = 0) {
        if (ushortValues.count(key)) return ushortValues[key];
        return defaultValue;
    }

    size_t putUShort(const char* key, uint16_t value) {
        ushortValues[key] = value;
        return sizeof(uint16_t);
    }

    // Test utility - check if key exists
    bool hasKey(const char* key) const {
        std::string k(key);
        return intValues.count(k) || uintValues.count(k) || longValues.count(k) ||
               ulongValues.count(k) || floatValues.count(k) || doubleValues.count(k) ||
               stringValues.count(k) || boolValues.count(k) ||
               ucharValues.count(k) || ushortValues.count(k);
    }

    // Alias for hasKey (ESP32 Preferences API compatibility)
    bool isKey(const char* key) const {
        return hasKey(key);
    }
};

#endif // MOCK_PREFERENCES_H
