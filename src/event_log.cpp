#include "event_log.h"
#include "serial_monitor.h"
#include "serial_redirect.h"

// Redirect Serial to CapturedSerial for web monitor
#define Serial CapturedSerial

EventLog::EventLog()
    : _mqttLoggingEnabled(false)
{
    _entries.reserve(MAX_ENTRIES);
}

EventLog::~EventLog() {
    saveToLittleFS();
}

EventLog& EventLog::getInstance() {
    static EventLog instance;
    return instance;
}

bool EventLog::begin() {
    // Check if LittleFS is already mounted (by web server)
    // If not mounted, try to mount it (for cases where EventLog is initialized first)
    // Note: LittleFS.begin() returns true if already mounted, false if mount fails
    // Specify partition name "spiffs" to match partition table (PlatformIO requirement)
    // Mount at "/littlefs" (can't mount to root "/" - it's reserved)
    if (!LittleFS.begin(false, "/littlefs", 5, "spiffs")) {  // false = don't format if mount fails
        Serial.println("EventLog: LittleFS not available, using memory-only logging");
        return false;
    }

    // Load existing logs from LittleFS
    return loadFromLittleFS();
}

void EventLog::addEntry(LogLevel level, const char* tag, const char* message) {
    // Create new log entry
    LogEntry entry(millis(), level, tag, message);

    // Add to in-memory vector
    _entries.push_back(entry);

    // If we exceed max entries, remove the oldest
    if (_entries.size() > MAX_ENTRIES) {
        _entries.erase(_entries.begin());
    }

    // Save to LittleFS (asynchronously would be better, but keep it simple for now)
    saveToLittleFS();

    // Publish to MQTT if enabled
    if (_mqttLoggingEnabled && _mqttCallback) {
        publishToMQTT(level, tag, message);
    }
}

String EventLog::getEntriesJSON() {
    return getFilteredEntriesJSON(LOG_VERBOSE, nullptr);
}

String EventLog::getFilteredEntriesJSON(LogLevel minLevel, const char* tag) {
    DynamicJsonDocument doc(8192);  // Adjust size as needed
    JsonArray array = doc.to<JsonArray>();

    for (const auto& entry : _entries) {
        // Apply filters (higher level = less important, so we skip if entry.level > minLevel)
        if (entry.level > minLevel) continue;
        if (tag != nullptr && entry.tag != tag) continue;

        // Create JSON object for this entry
        JsonObject obj = array.createNestedObject();
        obj["timestamp"] = entry.timestamp;
        obj["level"] = logLevelToString(entry.level);
        obj["tag"] = entry.tag;
        obj["message"] = entry.message;
    }

    String result;
    serializeJson(doc, result);
    return result;
}

void EventLog::clear() {
    _entries.clear();
    saveToLittleFS();
}

size_t EventLog::getCount() const {
    return _entries.size();
}

void EventLog::setMQTTLoggingEnabled(bool enabled) {
    _mqttLoggingEnabled = enabled;
}

bool EventLog::isMQTTLoggingEnabled() const {
    return _mqttLoggingEnabled;
}

void EventLog::setMQTTCallback(std::function<void(LogLevel, const char*, const char*)> callback) {
    _mqttCallback = callback;
}

const char* EventLog::logLevelToString(LogLevel level) {
    switch (level) {
        case LOG_NONE:    return "NONE";
        case LOG_ERROR:   return "ERROR";
        case LOG_WARNING: return "WARNING";
        case LOG_INFO:    return "INFO";
        case LOG_DEBUG:   return "DEBUG";
        case LOG_VERBOSE: return "VERBOSE";
        default:          return "UNKNOWN";
    }
}

bool EventLog::loadFromLittleFS() {
    // Check if LittleFS is mounted before attempting file operations
    // LittleFS.begin(false) returns true if already mounted, false if not mounted
    // Specify partition name "spiffs" to match partition table (PlatformIO requirement)
    // Mount at "/littlefs" (can't mount to root "/" - it's reserved)
    if (!LittleFS.begin(false, "/littlefs", 5, "spiffs")) {
        Serial.println("EventLog: LittleFS not mounted, skipping load");
        return false;
    }

    // LOG_FILE is "/event_log.json" - LittleFS.open() uses filesystem-relative paths
    // When mounted at "/littlefs", filesystem root "/" maps to VFS "/littlefs"
    // So we use LOG_FILE directly (it already has leading "/")
    String logPath = String(LOG_FILE);
    if (!LittleFS.exists(logPath)) {
        Serial.println("EventLog: No existing log file found, starting fresh");
        return true;
    }

    File file = LittleFS.open(logPath, "r");
    if (!file) {
        Serial.println("EventLog: Failed to open log file for reading");
        return false;
    }

    // Read the file content
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("EventLog: Failed to parse log file: ");
        Serial.println(error.c_str());
        return false;
    }

    // Clear existing entries
    _entries.clear();

    // Load entries from JSON
    JsonArray array = doc.as<JsonArray>();
    for (JsonObject obj : array) {
        unsigned long timestamp = obj["timestamp"];
        String levelStr = obj["level"].as<String>();
        String tag = obj["tag"].as<String>();
        String message = obj["message"].as<String>();

        // Convert level string to LogLevel enum
        LogLevel level = LOG_INFO;
        if (levelStr == "ERROR") level = LOG_ERROR;
        else if (levelStr == "WARNING") level = LOG_WARNING;
        else if (levelStr == "INFO") level = LOG_INFO;
        else if (levelStr == "DEBUG") level = LOG_DEBUG;
        else if (levelStr == "VERBOSE") level = LOG_VERBOSE;

        LogEntry entry(timestamp, level, tag.c_str(), message.c_str());
        _entries.push_back(entry);
    }

    Serial.print("EventLog: Loaded ");
    Serial.print(_entries.size());
    Serial.println(" log entries from LittleFS");

    return true;
}

bool EventLog::saveToLittleFS() {
    // Check if LittleFS is mounted before attempting file operations
    // LittleFS.begin(false) returns true if already mounted, false if not mounted
    // Specify partition name "spiffs" to match partition table (PlatformIO requirement)
    // Mount at "/littlefs" (can't mount to root "/" - it's reserved)
    if (!LittleFS.begin(false, "/littlefs", 5, "spiffs")) {
        // Silently fail - filesystem not available, use memory-only logging
        return false;
    }

    // LOG_FILE is "/event_log.json" - LittleFS.open() uses filesystem-relative paths
    String logPath = String(LOG_FILE);
    File file = LittleFS.open(logPath, "w");
    if (!file) {
        Serial.println("EventLog: Failed to open log file for writing");
        return false;
    }

    // Create JSON array
    DynamicJsonDocument doc(8192);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& entry : _entries) {
        JsonObject obj = array.createNestedObject();
        obj["timestamp"] = entry.timestamp;
        obj["level"] = logLevelToString(entry.level);
        obj["tag"] = entry.tag;
        obj["message"] = entry.message;
    }

    // Write to file
    size_t bytesWritten = serializeJson(doc, file);
    file.close();

    if (bytesWritten == 0) {
        Serial.println("EventLog: Failed to write log file");
        return false;
    }

    return true;
}

void EventLog::publishToMQTT(LogLevel level, const char* tag, const char* message) {
    if (_mqttCallback) {
        _mqttCallback(level, tag, message);
    }
}
