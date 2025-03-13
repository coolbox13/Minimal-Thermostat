#ifndef CONFIG_H
#define CONFIG_H

// KNX Configuration
#define KNX_AREA 1
#define KNX_LINE 1
#define KNX_MEMBER 159

// KNX Debug Configuration
#define KNX_DEBUG_ENABLED false  // Set to false to disable KNX debug messages

// KNX Group Address components for valve control (format: main/mid/sub)
#define KNX_GA_VALVE_MAIN 1
#define KNX_GA_VALVE_MID 1
#define KNX_GA_VALVE_SUB 1

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

#endif // CONFIG_H