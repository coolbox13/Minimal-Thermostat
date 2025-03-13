#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
// Update the include path to match the ESP32_ENVY_PORT library
#include <esp-knx-ip.h>
#include "bme280_sensor.h"
#include "config.h"
#include "utils.h"
#include "home_assistant.h"

// Global variables
BME280Sensor bme280;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
HomeAssistant homeAssistant(mqttClient, "esp32_thermostat");
float temperature = 0;
float humidity = 0;
float pressure = 0;
int valvePosition = 0;  // 0-100%
address_t valveAddress;
address_t temperatureAddress;
address_t humidityAddress;
address_t pressureAddress;

// Function declarations
void setupWiFi();
void setupMQTT();
void setupKNX();
void registerValveControlWithHA();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void updateSensorReadings();
void publishStatus();
void setValvePosition(int position);
void sendHardcodedDiscoveryMessage();

// Define the callback function with the correct signature
void knxCallback(message_t const &msg, void *arg);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 KNX Thermostat - Simplified Version");
  
  if (!bme280.begin()) {
    Serial.println("Failed to initialize BME280 sensor!");
  }
  
  setupWiFi();
  setupMQTT();
  setupKNX();

  // Send hardcoded discovery message for testing
  sendHardcodedDiscoveryMessage();

  // Then try the regular registration
  registerValveControlWithHA();
}

void loop() {
  // Handle KNX communications
  knx.loop();
  
  // Monitor and decode KNX debug messages
  monitorKnxDebugMessages();
  
  // Handle MQTT communications
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  
  // Update sensor readings and publish status every 30 seconds
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 30000) {
    updateSensorReadings();
    publishStatus();
    lastUpdate = millis();
  }
}

void setupWiFi() {
  Serial.println("Setting up WiFi...");
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  
  if (!wifiManager.autoConnect("ESP32-Thermostat-AP")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);  // Increased buffer size for discovery messages
  
  // Initialize Home Assistant integration when connected
  if (mqttClient.connected()) {
    homeAssistant.begin();
  }
}

// Update the function declaration to match the library's expected signature
void knxCallback(message_t const &msg, void *arg);

void setupKNX() {
  Serial.println("Setting up KNX...");
  
  // Configure KNX debug level based on configuration
  if (KNX_DEBUG_ENABLED) {
    esp_log_level_set("KNXIP", ESP_LOG_INFO);
  } else {
    esp_log_level_set("KNXIP", ESP_LOG_WARN);
  }
  
  // Start KNX without web server
  knx.start(nullptr);
  
  // Set physical address (area, line, member)
  knx.physical_address_set(knx.PA_to_address(KNX_AREA, KNX_LINE, KNX_MEMBER));
  
  // Create group addresses for valve control and sensor data
  valveAddress = knx.GA_to_address(KNX_GA_VALVE_MAIN, KNX_GA_VALVE_MID, KNX_GA_VALVE_SUB);
  temperatureAddress = knx.GA_to_address(KNX_GA_TEMPERATURE_MAIN, KNX_GA_TEMPERATURE_MID, KNX_GA_TEMPERATURE_SUB);
  humidityAddress = knx.GA_to_address(KNX_GA_HUMIDITY_MAIN, KNX_GA_HUMIDITY_MID, KNX_GA_HUMIDITY_SUB);
  pressureAddress = knx.GA_to_address(KNX_GA_PRESSURE_MAIN, KNX_GA_PRESSURE_MID, KNX_GA_PRESSURE_SUB);
  
  // Register callback for KNX events - using original method
  knx.callback_register("valve_control", knxCallback, nullptr);
  
  Serial.println("KNX initialized");
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32Thermostat-";
    clientId += String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("MQTT connected");
      
      // Subscribe to topics
      mqttClient.subscribe(MQTT_TOPIC_VALVE_COMMAND);
      
      // Initialize Home Assistant discovery
      homeAssistant.begin();
      
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

// Update your existing MQTT callback to handle valve control messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  
  Serial.print("MQTT message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  // Handle valve position command from original topic
  if (strcmp(topic, MQTT_TOPIC_VALVE_COMMAND) == 0) {
    int position = atoi(message);
    setValvePosition(position);
    return;
  }
  
  // Handle the new valve control topic
  if (strcmp(topic, "esp32_thermostat/valve/set") == 0) {
    // Try to parse the value - might be a direct number or JSON
    int position = 0;
    
    // Check if it's JSON (from light entity)
    if (message[0] == '{') {
      // Simple JSON parsing - look for "brightness" field
      String payloadStr = String(message);
      int brightnessPos = payloadStr.indexOf("\"brightness\"");
      if (brightnessPos > 0) {
        int valueStart = payloadStr.indexOf(":", brightnessPos) + 1;
        int valueEnd = payloadStr.indexOf(",", valueStart);
        if (valueEnd < 0) valueEnd = payloadStr.indexOf("}", valueStart);
        
        if (valueStart > 0 && valueEnd > valueStart) {
          String valueStr = payloadStr.substring(valueStart, valueEnd);
          valueStr.trim();
          position = valueStr.toInt();
        }
      }
    } else {
      // Try direct number parsing
      position = atoi(message);
    }
    
    // Set the position if valid
    setValvePosition(position);
    
    // Send to KNX test address for safety during testing
    address_t testValveAddress = knx.GA_to_address(10, 2, 2);
    knx.write_1byte_int(testValveAddress, position);
    
    Serial.print("Sent valve position to KNX test address: ");
    Serial.println(position);
  }
}

void updateSensorReadings() {
  temperature = bme280.readTemperature();
  humidity = bme280.readHumidity();
  pressure = bme280.readPressure();
  
  Serial.println("Sensor readings updated:");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Pressure: "); Serial.print(pressure); Serial.println(" hPa");
  Serial.print("Valve position: "); Serial.print(valvePosition); Serial.println(" %");
}

void publishStatus() {
  // Use Home Assistant to update all states at once via MQTT
  homeAssistant.updateStates(temperature, humidity, pressure, valvePosition);
  
  // Also send to KNX
  knx.write_2byte_float(temperatureAddress, temperature);
  knx.write_2byte_float(humidityAddress, humidity);
  knx.write_2byte_float(pressureAddress, pressure);
}

void setValvePosition(int position) {
  // Constrain position to 0-100%
  position = constrain(position, 0, 100);
  
  if (position != valvePosition) {
    valvePosition = position;
    Serial.print("Setting valve position to: ");
    Serial.print(valvePosition);
    Serial.println("%");
    
    // Send to KNX - using the proper API method
    uint8_t data = (uint8_t)valvePosition;
    knx.write_1byte_int(valveAddress, valvePosition);
    
    // Publish to MQTT
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    mqttClient.publish(MQTT_TOPIC_VALVE_STATUS, valveStr);
  }
}

// KNX callback function
void knxCallback(message_t const &msg, void *arg) {
  // Check if this is a message for our valve control GA - add this filter to reduce processing
  if (msg.received_on.value == valveAddress.value) {
    // Print the raw message details for debugging
    Serial.print("KNX Message - CT: 0x");
    Serial.print(msg.ct, HEX);
    Serial.print(", Dest: ");
    
    // Convert the destination group address to readable format
    address_t dest = msg.received_on;
    Serial.print(dest.ga.area);
    Serial.print("/");
    Serial.print(dest.ga.line);
    Serial.print("/");
    Serial.print(dest.ga.member);
    
    if (msg.data_len > 0) {
      Serial.print(", Data: ");
      for (int i = 0; i < msg.data_len; i++) {
        Serial.print("0x");
        Serial.print(msg.data[i], HEX);
        Serial.print(" ");
      }
    }
    Serial.println();
    
    // Process valve position command
    if (msg.data_len > 0 && msg.ct == KNX_CT_WRITE) {
      // Extract valve position value (assuming it's a scaling value 0-100%)
      int position = (int)msg.data[0];
      setValvePosition(position);
    }
  }
}

void registerValveControlWithHA() {
  if (!mqttClient.connected()) return;
  
  Serial.println("Registering valve control with Home Assistant...");
  
  // Create topics
  String discoveryPrefix = "homeassistant";
  String controlTopic = "esp32_thermostat/valve/set";
  String statusTopic = "esp32_thermostat/valve/status";
  
  // Subscribe to the control topic
  mqttClient.subscribe(controlTopic.c_str());
  
  // Valve position sensor config
  String sensorTopic = discoveryPrefix + "/sensor/esp32_thermostat/valve_position/config";
  String sensorPayload = "{";
  sensorPayload += "\"name\":\"Valve Position\",";
  sensorPayload += "\"state_topic\":\"" + statusTopic + "\",";
  sensorPayload += "\"unit_of_measurement\":\"%\",";
  sensorPayload += "\"icon\":\"mdi:valve\"";
  sensorPayload += "}";
  
  bool sensorSuccess = mqttClient.publish(sensorTopic.c_str(), sensorPayload.c_str(), true);
  Serial.print("Valve position sensor registration: ");
  Serial.println(sensorSuccess ? "Success" : "Failed");
  
  // Valve control as a simple slider (using light with brightness)
  String sliderTopic = discoveryPrefix + "/light/esp32_thermostat/valve_control/config";
  String sliderPayload = "{";
  sliderPayload += "\"name\":\"Valve Control\",";
  sliderPayload += "\"schema\":\"json\",";
  sliderPayload += "\"brightness\":true,";
  sliderPayload += "\"command_topic\":\"" + controlTopic + "\",";
  sliderPayload += "\"state_topic\":\"" + statusTopic + "\",";
  sliderPayload += "\"brightness_scale\":100,";
  sliderPayload += "\"icon\":\"mdi:radiator\"";
  sliderPayload += "}";
  
  bool sliderSuccess = mqttClient.publish(sliderTopic.c_str(), sliderPayload.c_str(), true);
  Serial.print("Valve control registration: ");
  Serial.println(sliderSuccess ? "Success" : "Failed");
  
  // Initialize valve position
  char valveStr[4];
  itoa(valvePosition, valveStr, 10);
  mqttClient.publish(statusTopic.c_str(), valveStr, true);
  
  Serial.println("Valve control registration complete");
}

// Add this function to your main.cpp
void sendHardcodedDiscoveryMessage() {
  if (!mqttClient.connected()) return;
  
  Serial.println("Sending hardcoded discovery message for testing...");
  
  // Create a discovery topic for valve control
  String discoveryTopic = "homeassistant/light/esp32_thermostat/valve_control/config";
  
  // Create a simple discovery payload
  String payload = "{";
  payload += "\"name\":\"Valve Control Test\",";
  payload += "\"schema\":\"json\",";
  payload += "\"brightness\":true,";
  payload += "\"command_topic\":\"esp32_thermostat/valve/set\",";
  payload += "\"state_topic\":\"esp32_thermostat/valve/status\",";
  payload += "\"brightness_scale\":100,";
  payload += "\"icon\":\"mdi:radiator\"";
  payload += "}";
  
  // Publish with retain flag
  bool success = mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
  
  Serial.print("Published hardcoded discovery message: ");
  Serial.println(success ? "SUCCESS" : "FAILED");
  Serial.print("Topic: ");
  Serial.println(discoveryTopic);
  Serial.print("Payload: ");
  Serial.println(payload);
  
  // Also publish an initial status
  mqttClient.publish("esp32_thermostat/valve/status", "0", true);
}