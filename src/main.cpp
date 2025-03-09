#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "config_manager.h"
#include "thermostat_state.h"
#include "protocol_manager.h"
#include "communication/mqtt/mqtt_interface.h"
#include "communication/knx/knx_interface.h"
#include "sensors/bme280_sensor_interface.h"
#include "pid_controller.h"
#include "web/esp_web_server.h"

// Global objects
ConfigManager configManager;
ThermostatState thermostatState;
ProtocolManager protocolManager(&thermostatState);
BME280SensorInterface sensorInterface;
PIDController pidController;
ESPWebServer webServer(&configManager, &thermostatState);

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("Starting ESP32 KNX Thermostat...");

    // Initialize configuration
    if (!configManager.begin()) {
        Serial.println("Failed to initialize configuration");
        return;
    }

    // Setup WiFi
    if (!configManager.setupWiFi()) {
        Serial.println("Failed to connect to WiFi");
        return;
    }

    // Initialize sensor
    if (!sensorInterface.begin()) {
        Serial.println("Failed to initialize BME280 sensor");
        return;
    }

    // Initialize thermostat state
    thermostatState.begin();

    // Initialize PID controller
    pidController.begin();
    pidController.configure(configManager.getPidConfig());

    // Initialize protocol manager
    if (!protocolManager.begin()) {
        Serial.println("Failed to initialize protocol manager");
        return;
    }

    // Configure and add MQTT interface if enabled
    if (configManager.isMqttEnabled()) {
        StaticJsonDocument<512> mqttConfig;
        mqttConfig["enabled"] = true;
        mqttConfig["server"] = configManager.getMqttServer();
        mqttConfig["port"] = configManager.getMqttPort();
        mqttConfig["username"] = configManager.getMqttUsername();
        mqttConfig["password"] = configManager.getMqttPassword();
        mqttConfig["clientId"] = configManager.getDeviceName();
        mqttConfig["topicPrefix"] = "esp32/thermostat/";

        auto mqttInterface = new MQTTInterface(&thermostatState);
        if (mqttInterface->configure(mqttConfig)) {
            protocolManager.addProtocol(mqttInterface);
            Serial.println("MQTT interface configured and added");
        } else {
            Serial.println("Failed to configure MQTT interface");
            delete mqttInterface;
        }
    }

    // Configure and add KNX interface if enabled
    if (configManager.isKnxEnabled()) {
        StaticJsonDocument<512> knxConfig;
        JsonObject physical = knxConfig.createNestedObject("physical");
        physical["area"] = configManager.getKnxArea();
        physical["line"] = configManager.getKnxLine();
        physical["device"] = configManager.getKnxDevice();

        auto knxInterface = new KNXInterface(&thermostatState);
        if (knxInterface->configure(knxConfig)) {
            protocolManager.addProtocol(knxInterface);
            Serial.println("KNX interface configured and added");
        } else {
            Serial.println("Failed to configure KNX interface");
            delete knxInterface;
        }
    }

    // Initialize web server
    if (!webServer.begin()) {
        Serial.println("Failed to start web server");
        return;
    }

    Serial.println("ESP32 KNX Thermostat initialized successfully");
}

void loop() {
    // Update sensor readings
    sensorInterface.loop();

    // Update thermostat state
    thermostatState.loop();

    // Update PID controller
    pidController.setInput(sensorInterface.getTemperature());
    pidController.setSetpoint(thermostatState.getTargetTemperature());
    pidController.loop();

    // Update valve position
    sensorInterface.setValvePosition(pidController.getOutput());

    // Update protocol manager
    protocolManager.loop();

    // Small delay to prevent tight looping
    delay(10);
}