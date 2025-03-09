#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <LittleFS.h>

// Thermostat components
#include "config_manager.h"
#include "thermostat_state.h"
#include "sensor_interface.h"
#include "knx_interface.h"
#include "mqtt_interface.h"
#include "web_interface.h"
#include "pid_controller.h"
#include "protocol_manager.h"

// Global objects
ConfigManager configManager;
ThermostatState thermostatState;
SensorInterface sensorInterface;
KNXInterface knxInterface;
MQTTInterface mqttInterface;
WebInterface webInterface;
PIDController pidController;
ProtocolManager protocolManager;

// Task management
TaskHandle_t sensorTask = NULL;
TaskHandle_t communicationTask = NULL;

// Sensor task to run on separate core
void sensorTaskFunction(void * parameter) {
  for(;;) {
    // Update sensor readings
    sensorInterface.update();
    
    // Process PID control
    pidController.update();
    
    vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms yield
  }
}

// Communication task to run on separate core
void communicationTaskFunction(void * parameter) {
  for(;;) {
    // Process protocol communications via protocol manager
    protocolManager.loop();
    
    // Process web interface
    webInterface.handle();
    
    vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms yield
  }
}

void setup() {
  delay(1000); // Allow ESP32 to stabilize at startup
  
  Serial.begin(115200);
  Serial.println("\n\nStarting ESP32-KNX-Thermostat...");

    
  // Initialize file system early
#ifdef ESP32
if (!LITTLEFS.begin(true)) {
  Serial.println("LittleFS mount failed! Formatting...");
  LITTLEFS.format();
}
#else
if (!LittleFS.begin()) {
  Serial.println("LittleFS mount failed!");
}
#endif
  
  // Initialize I2C for sensors
  Wire.begin();

  // Initialize LittleFS
  if (!LittleFS.begin(true)) { // true = format on failure
    Serial.println("LittleFS mount failed even after format attempt");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
  
  // Initialize configuration
  if (!configManager.begin()) {
    Serial.println("Failed to initialize configuration");
  }
  
  // Setup WiFi with increased timeout and retry
  WiFi.setAutoReconnect(true);
  if (!configManager.setupWiFi()) {
    Serial.println("Failed to connect to WiFi");
  }
  
  // Initialize thermostat state with default values
  thermostatState.setTargetTemperature(configManager.getSetpoint());
  
  // Initialize protocol manager
  protocolManager.begin(&thermostatState);
  
  // Initialize sensor interface
  if (!sensorInterface.begin(&thermostatState)) {
    Serial.println("Failed to initialize sensors");
  }
  sensorInterface.setUpdateInterval(configManager.getSendInterval());
  
  // Initialize KNX interface if enabled
  if (configManager.getKnxEnabled()) {
    if (knxInterface.begin(
          configManager.getKnxPhysicalArea(),
          configManager.getKnxPhysicalLine(),
          configManager.getKnxPhysicalMember())) {
      
      Serial.println("KNX interface initialized");
      
      // Set group addresses from configuration
      knxInterface.setTemperatureGA(
        configManager.getKnxTempArea(),
        configManager.getKnxTempLine(),
        configManager.getKnxTempMember());
        
      knxInterface.setSetpointGA(
        configManager.getKnxSetpointArea(),
        configManager.getKnxSetpointLine(),
        configManager.getKnxSetpointMember());
        
      knxInterface.setValvePositionGA(
        configManager.getKnxValveArea(),
        configManager.getKnxValveLine(),
        configManager.getKnxValveMember());
        
      knxInterface.setModeGA(
        configManager.getKnxModeArea(),
        configManager.getKnxModeLine(),
        configManager.getKnxModeMember());
      
      // Register with protocol manager
      knxInterface.registerCallbacks(&thermostatState, &protocolManager);
    } else {
      Serial.println("Failed to initialize KNX interface");
    }
  } else {
    Serial.println("KNX interface disabled in config");
  }
  
  // Initialize MQTT interface if enabled
  if (configManager.getMqttEnabled()) {
    if (mqttInterface.begin(
          &thermostatState,
          configManager.getMqttServer(),
          configManager.getMqttPort(),
          configManager.getMqttUser(),
          configManager.getMqttPassword(),
          configManager.getMqttClientId())) {
      
      Serial.println("MQTT interface initialized");
      mqttInterface.registerProtocolManager(&protocolManager);
    } else {
      Serial.println("Failed to initialize MQTT interface");
    }
  } else {
    Serial.println("MQTT interface disabled in config");
  }
  
  // Register protocols with protocol manager
  protocolManager.registerProtocols(&knxInterface, &mqttInterface);
  
  // Initialize PID controller
  pidController.begin(&thermostatState, 
                      configManager.getKp(), 
                      configManager.getKi(), 
                      configManager.getKd());
  pidController.setUpdateInterval(configManager.getPidInterval());
  
  // Initialize web interface
  webInterface.begin(&thermostatState, 
                    &configManager, 
                    &sensorInterface,
                    &knxInterface,
                    &mqttInterface,
                    &pidController);
  
  Serial.println("Setup completed");

  // Create tasks to run on separate cores
  xTaskCreatePinnedToCore(
    sensorTaskFunction,        // Task function
    "SensorTask",              // Name
    4096,                      // Stack size (bytes)
    NULL,                      // Parameters
    1,                         // Priority (1 is low)
    &sensorTask,               // Task handle
    0                          // Core (0)
  );

  xTaskCreatePinnedToCore(
    communicationTaskFunction,  // Task function
    "CommTask",                 // Name
    8192,                       // Stack size (bytes) - more for network
    NULL,                       // Parameters
    1,                          // Priority (1 is low)
    &communicationTask,         // Task handle
    1                           // Core (1)
  );

  Serial.println("Tasks started on both cores");
}

void loop() {
  // Main loop is now minimal since tasks handle the work
  delay(1000); // Just yield to other tasks
  
  // Background monitoring
  static unsigned long lastHealthCheck = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastHealthCheck > 60000) { // Every minute
    lastHealthCheck = currentTime;
    
    // Check WiFi and reconnect if needed
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost, reconnecting...");
      WiFi.reconnect();
    }
    
    // Log memory usage
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  }
}