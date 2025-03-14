#ifndef CONFIG_H
#define CONFIG_H

// KNX Configuration
#define KNX_AREA 1
#define KNX_LINE 1
#define KNX_MEMBER 159

// KNX Debug Configuration
// Add this line to control KNX debug output
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
#define PID_UPDATE_INTERVAL 10000 // Update PID controller every 10 seconds

// Initial PID Parameters (will be auto-tuned)
#define PID_KP_INITIAL 2.0      // Proportional gain
#define PID_KI_INITIAL 0.1      // Integral gain
#define PID_KD_INITIAL 0.5      // Derivative gain
#define PID_DEADBAND 0.2        // Temperature deadband (±°C)

#endif // CONFIG_H