#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
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
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void updateSensorReadings();
void publishStatus();
void setValvePosition(int position);

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
      mqttClient.subscribe("esp32_thermostat/valve/set");  // Subscribe to valve control topic
      
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
    
    // Update valve position locally without sending to KNX
    valvePosition = constrain(position, 0, 100);
    
    // Only send to test address
    address_t testValveAddress = knx.GA_to_address(10, 2, 2);
    knx.write_1byte_int(testValveAddress, valvePosition);
    
    // Update MQTT status
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    mqttClient.publish(MQTT_TOPIC_VALVE_STATUS, valveStr);
    
    Serial.print("Updated valve position to: ");
    Serial.print(valvePosition);
    Serial.println("% (sent to test address only)");
    return;
  }
  
  // Handle the valve control topic from Home Assistant
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
    
    Serial.print("Parsed valve position: ");
    Serial.println(position);
    
    // Update valve position locally without calling setValvePosition()
    valvePosition = constrain(position, 0, 100);
    
    // Send only to test address
    address_t testValveAddress = knx.GA_to_address(10, 2, 2);
    knx.write_1byte_int(testValveAddress, valvePosition);
    
    // Update MQTT status
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    mqttClient.publish("esp32_thermostat/valve/status", valveStr);
    
    Serial.print("Updated valve position to: ");
    Serial.print(valvePosition);
    Serial.println("% (sent to test address only)");
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
    
    // NOTE: We're not sending to KNX here anymore to avoid duplicates
    // KNX messages are now sent explicitly from the MQTT callback
    
    // Publish to MQTT
    char valveStr[4];
    itoa(valvePosition, valveStr, 10);
    mqttClient.publish(MQTT_TOPIC_VALVE_STATUS, valveStr);
  }
}

/// KNX callback function
void knxCallback(message_t const &msg, void *arg) {
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
  
  // We're still processing KNX messages to update our internal state
  // but we'll handle this without sending duplicates
  
  // 10/2/2 is our test valve address
  address_t testValveAddress = knx.GA_to_address(10, 2, 2);
  
  // Check if this message is for our valve address from KNX
  if (msg.received_on.value == valveAddress.value || 
      msg.received_on.value == testValveAddress.value) {
      
    // Process valve position command
    if (msg.data_len > 0 && msg.ct == KNX_CT_WRITE) {
      // Extract valve position value (assuming it's a scaling value 0-100%)
      int position = (int)msg.data[0];
      
      // Only update local state and MQTT, don't send back to KNX
      valvePosition = constrain(position, 0, 100);
      
      // Publish to MQTT
      char valveStr[4];
      itoa(valvePosition, valveStr, 10);
      mqttClient.publish(MQTT_TOPIC_VALVE_STATUS, valveStr);
      
      Serial.print("Updated valve position from KNX to: ");
      Serial.println(valvePosition);
    }
  }
}