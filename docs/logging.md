Summary of Changes
1. EventLog Class (Persistent Logging)
Location: include/event_log.h and src/event_log.cpp
Features:
Stores up to 100 log entries in SPIFFS (circular buffer)
Persists across device reboots
Stores only ERROR and WARNING levels to minimize flash wear
JSON format for easy parsing
2. MQTT Logging
Publishes log events to esp32_thermostat/logs topic
JSON payload includes: timestamp, level, tag, and message
Configurable enable/disable flag
Integrated with existing MQTT client
3. Web Interface
New page: /logs.html for viewing event logs
Features:
Tabular display with color-coded log levels
Filtering by log level (All, Errors Only, Warnings & Errors, etc.)
Auto-refresh every 10 seconds
Clear logs functionality
Shows timestamp (formatted as uptime)
Sorts logs newest-first
Added "Event Logs" button to main dashboard
4. REST API Endpoints
GET /api/logs - Retrieve all log entries as JSON
POST /api/logs/clear - Clear all stored logs
5. Integration
Modified src/main.cpp to:
Initialize EventLog on startup
Connect EventLog to existing Logger callback
Set up MQTT publishing callback
Modified src/web_server.cpp to add new endpoints
How It Works
Logging: When the system logs an ERROR or WARNING (using LOG_E() or LOG_W() macros), it's automatically:

Stored in SPIFFS via EventLog
Published to MQTT topic if connected
Viewing Logs: Access logs via:

Web UI at http://[device-ip]/logs.html
REST API at http://[device-ip]/api/logs
MQTT subscription to esp32_thermostat/logs
Persistence: Logs survive device reboots and are loaded from SPIFFS on startup

All changes have been committed and pushed to the branch claude/add-logging-feature-011CUx7hsCSoLHUW5qcUcsAb.