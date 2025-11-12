#include "MockPreferences.h"

// Define static storage members
std::map<std::string, int> MockPreferences::intValues;
std::map<std::string, unsigned int> MockPreferences::uintValues;
std::map<std::string, long> MockPreferences::longValues;
std::map<std::string, unsigned long> MockPreferences::ulongValues;
std::map<std::string, float> MockPreferences::floatValues;
std::map<std::string, double> MockPreferences::doubleValues;
std::map<std::string, std::string> MockPreferences::stringValues;
std::map<std::string, bool> MockPreferences::boolValues;
std::map<std::string, uint8_t> MockPreferences::ucharValues;
std::map<std::string, uint16_t> MockPreferences::ushortValues;
