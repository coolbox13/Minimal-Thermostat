#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1
#define CORE_DEBUG_LEVEL 5
#include <Arduino.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include "config_manager.h"
#include "thermostat_state.h"
#include "protocol_manager.h"
#include "communication/mqtt/mqtt_interface.h"
#include "communication/knx/knx_interface.h"
#include "sensors/bme280_sensor_interface.h"
#include "pid_controller.h"
#include "web/esp_web_server.h"
#include "web/web_interface.h"

static const char* TAG = "Main";

// Global objects
ConfigManager configManager;
ThermostatState thermostatState;
ProtocolManager protocolManager(&thermostatState);
BME280SensorInterface sensorInterface;
PIDController pidController;
ESPWebServer webServer(&configManager, &thermostatState);
WebInterface webInterface(&configManager, &sensorInterface, &pidController, &thermostatState, &protocolManager);
KNXInterface knxInterface(&thermostatState);
MQTTInterface mqttInterface(&thermostatState);

void setup() {
    // Initialize serial communication
    Serial2.begin(115200);
    Serial2.println("ESP32 KNX Thermostat starting...");
    
    // Initialize file system
    if (!LittleFS.begin()) {
        Serial2.println("Failed to mount file system");
        return;
    }
    
    // Initialize configuration
    if (!configManager.begin()) {
        Serial2.println("Failed to initialize configuration");
        return;
    }

    // Setup WiFi
    if (!configManager.setupWiFi()) {
        Serial2.println("Failed to connect to WiFi");
        return;
    }

    // Initialize sensor
    if (!sensorInterface.begin()) {
        Serial2.println("Failed to initialize BME280 sensor");
        return;
    }

    // Initialize PID controller
    if (!pidController.begin()) {
        Serial2.println("Failed to initialize PID controller");
        return;
    }

    // Initialize protocol manager
    if (!protocolManager.begin()) {
        Serial2.println("Failed to initialize protocol manager");
        return;
    }

    // MQTT setup
    if (configManager.getMqttEnabled()) {
        Serial2.println("MQTT is enabled");
        mqttInterface.setServer(configManager.getMQTTServer(), configManager.getMQTTPort());
        if (configManager.getMQTTUser()[0] != '\0') {
            mqttInterface.setCredentials(configManager.getMQTTUser(), configManager.getMQTTPassword());
        }
        
        // Add to protocol manager
        mqttInterface.begin();
        protocolManager.addProtocol(&mqttInterface);
        Serial2.println("MQTT interface added");
    }

    // KNX setup
    if (configManager.getKnxEnabled()) {
        ESP_LOGI(TAG, "Initializing KNX interface");
        // Configure KNX settings
        DynamicJsonDocument knxConfig(512);
        uint8_t area, line, member;
        configManager.getKnxPhysicalAddress(area, line, member);
        knxConfig["physical"]["area"] = area;
        knxConfig["physical"]["line"] = line;
        knxConfig["physical"]["device"] = member;
        
        // Add more KNX configuration...
        knxInterface.configure(knxConfig);
        protocolManager.addProtocol(&knxInterface);
        Serial2.println("KNX interface configured and added");
    }

    // Initialize web server
    if (!webServer.begin()) {
        Serial2.println("Failed to start web server");
        return;
    }

    Serial2.println("ESP32 KNX Thermostat initialized successfully");
}

void loop() {
    // Update sensor readings
    sensorInterface.loop();

    // Update web interface
    webInterface.loop();

    // Update KNX interface if enabled
    if (configManager.getKnxEnabled()) {
        knxInterface.loop();
    }

    // Update PID controller
    pidController.update(sensorInterface.getTemperature());

    // Update thermostat state
    thermostatState.setCurrentTemperature(sensorInterface.getTemperature());
    thermostatState.setCurrentHumidity(sensorInterface.getHumidity());
    thermostatState.setCurrentPressure(sensorInterface.getPressure());
    thermostatState.setValvePosition(pidController.getOutput());

    // Handle protocol updates
    protocolManager.update();

    // Small delay to prevent tight looping
    delay(10);
}