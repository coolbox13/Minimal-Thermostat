#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1
#define CORE_DEBUG_LEVEL 5
#include <Arduino.h>
#include <HardwareSerial.h>
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
    Serial2.begin(115200);
    Serial2.println("ESP32 KNX Thermostat starting...");
    
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
    pidController.begin();
    pidController.configure(static_cast<const void*>(&configManager.getPidConfig()));

    // Initialize protocol manager
    if (!protocolManager.begin()) {
        Serial2.println("Failed to initialize protocol manager");
        return;
    }

    // Configure and add MQTT interface if enabled
    if (configManager.getMQTTEnabled()) {
        StaticJsonDocument<512> mqttConfig;
        mqttConfig["enabled"] = true;
        mqttConfig["server"] = configManager.getMQTTServer();
        mqttConfig["port"] = configManager.getMQTTPort();
        mqttConfig["username"] = configManager.getMQTTUser();
        mqttConfig["password"] = configManager.getMQTTPassword();
        mqttConfig["clientId"] = configManager.getDeviceName();
        mqttConfig["topicPrefix"] = "esp32/thermostat/";

        auto mqttInterface = new MQTTInterface(&thermostatState);
        if (mqttInterface->configure(mqttConfig)) {
            protocolManager.addProtocol(mqttInterface);
            Serial2.println("MQTT interface configured and added");
        } else {
            Serial2.println("Failed to configure MQTT interface");
            delete mqttInterface;
        }
    }

    // Configure and add KNX interface if enabled
    if (configManager.getKnxEnabled()) {
        StaticJsonDocument<512> knxConfig;
        JsonObject physical = knxConfig.createNestedObject("physical");
        physical["area"] = configManager.getKnxPhysicalArea();
        physical["line"] = configManager.getKnxPhysicalLine();
        physical["device"] = configManager.getKnxPhysicalMember();

        auto knxInterface = new KNXInterface(&thermostatState);
        if (knxInterface->configure(knxConfig)) {
            protocolManager.addProtocol(knxInterface);
            Serial2.println("KNX interface configured and added");
        } else {
            Serial2.println("Failed to configure KNX interface");
            delete knxInterface;
        }
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