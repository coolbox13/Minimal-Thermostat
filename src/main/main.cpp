#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_log.h>
#include <LittleFS.h>

#include "config_manager.h"
#include "thermostat_state.h"
#include "pid_controller.h"
#include "interfaces/sensor_interface.h"
#include "mqtt_interface.h"
#include "web/web_interface.h"
#include "communication/knx/knx_interface.h"
#include "protocol_manager.h"

static const char* TAG = "Main";

// Global objects
ConfigManager configManager;
ThermostatState thermostatState;
PIDController pidController;
BME280SensorInterface sensorInterface;
KNXInterface knxInterface(&thermostatState);
MQTTInterface mqttInterface(&thermostatState);
WebInterface webInterface(&configManager, &sensorInterface, &pidController, &thermostatState);
ProtocolManager protocolManager(&thermostatState);

// Task handles
TaskHandle_t communicationTask;

// Function declarations
void communicationTaskFunction(void* parameter);

// Communication task
void communicationTaskFunction(void* parameter) {
    const TickType_t xDelay = pdMS_TO_TICKS(100); // 100ms delay

    while (true) {
        // Handle protocol communication
        protocolManager.loop();

        // Handle web interface
        webInterface.loop();

        // Small delay to prevent watchdog issues
        vTaskDelay(xDelay);
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    ESP_LOGI(TAG, "Starting ESP32 KNX Thermostat...");

    // Initialize file system
    if (!LittleFS.begin(true)) {
        ESP_LOGE(TAG, "Failed to mount file system");
        return;
    }

    // Load configuration
    if (!configManager.loadConfig()) {
        ESP_LOGE(TAG, "Failed to load configuration");
        return;
    }

    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(configManager.getWiFiSSID(), configManager.getWiFiPassword());

    // Wait for WiFi connection
    ESP_LOGI(TAG, "Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        ESP_LOGI(TAG, ".");
    }
    ESP_LOGI(TAG, "WiFi connected");
    ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());

    // Initialize MDNS
    if (!MDNS.begin(configManager.getDeviceName())) {
        ESP_LOGE(TAG, "Error setting up MDNS responder!");
    }

    // Initialize sensor
    if (!sensorInterface.begin()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return;
    }

    // Initialize protocols
    protocolManager.begin();

    // Add protocols to manager
    if (configManager.getMQTTEnabled()) {
        mqttInterface.configure(
            configManager.getMQTTServer(),
            configManager.getMQTTPort(),
            configManager.getMQTTUser(),
            configManager.getMQTTPassword()
        );
        mqttInterface.setClientId(configManager.getMQTTClientId());
        protocolManager.addProtocol(&mqttInterface);
    }

    if (configManager.getKNXEnabled()) {
        knxInterface.configure(
            configManager.getKNXIPAddress(),
            configManager.getKNXPort(),
            configManager.getKNXPhysicalAddress()
        );
        protocolManager.addProtocol(&knxInterface);
    }

    // Initialize PID controller
    pidController.configure(configManager.getPIDConfig());
    pidController.begin();

    // Initialize web interface
    webInterface.begin();

    // Create communication task
    xTaskCreatePinnedToCore(
        communicationTaskFunction,
        "CommunicationTask",
        8192,
        NULL,
        1,
        &communicationTask,
        0
    );

    ESP_LOGI(TAG, "Setup complete");
}

void loop() {
    static unsigned long lastWiFiCheck = 0;
    static unsigned long lastHeapCheck = 0;
    const unsigned long CHECK_INTERVAL = 30000; // 30 seconds

    // Update sensor readings
    sensorInterface.loop();

    // Update PID controller
    pidController.loop();

    // Check WiFi connection periodically
    if (millis() - lastWiFiCheck >= CHECK_INTERVAL) {
        if (WiFi.status() != WL_CONNECTED) {
            ESP_LOGW(TAG, "WiFi connection lost, reconnecting...");
            WiFi.reconnect();
        }
        lastWiFiCheck = millis();
    }

    // Log heap info periodically
    if (millis() - lastHeapCheck >= CHECK_INTERVAL) {
        ESP_LOGI(TAG, "Free heap: %d bytes", ESP.getFreeHeap());
        lastHeapCheck = millis();
    }

    // Small delay to prevent watchdog issues
    delay(100);
}