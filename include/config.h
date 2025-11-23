#ifndef CONFIG_H
#define CONFIG_H

// Firmware Version
#define FIRMWARE_VERSION "1.6.0"

// KNX Configuration
#define KNX_AREA 1
#define KNX_LINE 1
#define KNX_MEMBER 159

// KNX Debug Configuration
#ifndef KNX_DEBUG_ENABLED
#define KNX_DEBUG_ENABLED 0  // Set to 1 to enable KNX debug messages
#endif

// KNX Group Address components for valve control (format: main/mid/sub)
#define KNX_GA_VALVE_MAIN 1
#define KNX_GA_VALVE_MID 1
#define KNX_GA_VALVE_SUB 1

// KNX Test valve address - Used for PID output
#define KNX_GA_TEST_VALVE_MAIN 10
#define KNX_GA_TEST_VALVE_MID 2
#define KNX_GA_TEST_VALVE_SUB 2

// KNX Valve status/feedback address - Used to read actual valve position from the bus
// By default, same as command address (if valve doesn't have separate feedback)
#define KNX_GA_VALVE_STATUS_MAIN 1
#define KNX_GA_VALVE_STATUS_MID 1
#define KNX_GA_VALVE_STATUS_SUB 2

// KNX Group Addresses for BME280 sensor data
#define KNX_GA_TEMPERATURE_MAIN 0
#define KNX_GA_TEMPERATURE_MID 0
#define KNX_GA_TEMPERATURE_SUB 4

#define KNX_GA_HUMIDITY_MAIN 0
#define KNX_GA_HUMIDITY_MID 0
#define KNX_GA_HUMIDITY_SUB 5

#define KNX_GA_PRESSURE_MAIN 0
#define KNX_GA_PRESSURE_MID 0
#define KNX_GA_PRESSURE_SUB 6

// MQTT Configuration
#define MQTT_SERVER "192.168.178.32"  // Replace with your MQTT broker address
#define MQTT_PORT 1883
#define MQTT_USER ""        // Replace with your MQTT username
#define MQTT_PASSWORD "" // Replace with your MQTT password

// MQTT Topics
#define MQTT_TOPIC_TEMPERATURE "thermostat/temperature"
#define MQTT_TOPIC_HUMIDITY "thermostat/humidity"
#define MQTT_TOPIC_PRESSURE "thermostat/pressure"
#define MQTT_TOPIC_VALVE_STATUS "thermostat/valve/status"
#define MQTT_TOPIC_VALVE_COMMAND "thermostat/valve/set"

// PID Configuration
#define PID_SETPOINT 22.0       // Default temperature setpoint (°C)
#define PID_UPDATE_INTERVAL 10000 // Update PID controller every 10 seconds (ms)
#define PID_ADAPTATION_INTERVAL_SEC 60.0f  // PID parameter adaptation interval (60 seconds)
#define PID_CONFIG_WRITE_INTERVAL_MS 300000  // Write PID config to flash max once per 5 minutes

// Initial PID Parameters (will be auto-tuned)
#define PID_KP_INITIAL 2.0      // Proportional gain
#define PID_KI_INITIAL 0.1      // Integral gain
#define PID_KD_INITIAL 0.5      // Derivative gain
#define PID_DEADBAND 0.2        // Temperature deadband (±°C)

// WiFi Configuration Timeouts
#define WIFI_CONNECT_TIMEOUT_SEC 180        // WiFi connection timeout during setup (3 minutes)
#define WIFI_CONNECT_TIMEOUT_MS 10000       // WiFi connection attempt timeout (10 seconds)
#define WIFI_RECONNECT_TIMEOUT_MS 10000     // WiFi reconnection timeout (10 seconds)
#define INTERNET_CHECK_INTERVAL_MS 300000   // Internet connectivity check interval (5 minutes)
#define SENSOR_UPDATE_INTERVAL_MS 30000     // Sensor reading update interval (30 seconds)
#define CONNECTIVITY_CHECK_INTERVAL_MS 300000  // Connectivity check interval in main loop (5 minutes)

// Watchdog timer configurations
#define SYSTEM_WATCHDOG_TIMEOUT 2700000  // 45-minute system watchdog (in ms)
#define WIFI_WATCHDOG_TIMEOUT 1800000    // 30-minute WiFi watchdog (in ms)
#define MAX_RECONNECT_ATTEMPTS 10        // Maximum reconnection attempts before reboot
#define MAX_CONSECUTIVE_RESETS 3  // Maximum consecutive resets before entering safe mode

// NTP Configuration
#define NTP_SERVER "pool.ntp.org"        // Default NTP server
#define NTP_TIMEZONE_OFFSET 0            // Timezone offset in seconds from UTC (0 = UTC)
                                         // Examples: UTC+1 = 3600, UTC+2 = 7200, UTC-5 = -18000
#define NTP_DAYLIGHT_OFFSET 0            // Daylight saving offset in seconds (0 = no DST)
                                         // For CEST (Central European Summer Time): 3600
#define NTP_SYNC_TIMEOUT_MS 10000        // NTP sync timeout (10 seconds)

#endif // CONFIG_H