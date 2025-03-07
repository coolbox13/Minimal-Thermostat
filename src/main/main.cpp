#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_BME280.h>

// Global objects
Adafruit_BME280 bme;
WebServer server(80);
bool bmeAvailable = false;

// Thermostat state
float currentTemperature = 20.0;
float currentHumidity = 50.0;
float targetTemperature = 21.0;
float valvePosition = 0.0;
bool heatingActive = false;

// PID parameters
float kp = 1.0;
float ki = 0.1;
float kd = 0.01;
float lastError = 0.0;
float integral = 0.0;
unsigned long lastPidUpdate = 0;
const unsigned long PID_INTERVAL = 30000; // 30 seconds

// Function prototypes
bool setupSensors();
void setupWebServer();
void handleRoot();
void handleSetpoint();
void updatePID();

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nStarting ESP32-KNX-Thermostat...");
  
  // Initialize sensors
  bmeAvailable = setupSensors();
  
  // Initialize WiFi
  WiFiManager wm;
  bool res = wm.autoConnect("KNX-Thermostat", "password");

  if(!res) {
    Serial.println("Failed to connect to WiFi");
    ESP.restart();
  } 
  else {
    Serial.println("Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  
  // Setup web server
  setupWebServer();
  
  Serial.println("Setup completed");
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Read BME280 every 10 seconds if available
  static unsigned long lastReadTime = 0;
  if (bmeAvailable && millis() - lastReadTime > 10000) {
    currentTemperature = bme.readTemperature();
    currentHumidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    
    Serial.println("Sensor readings:");
    Serial.printf("Temperature: %.2f °C\n", currentTemperature);
    Serial.printf("Humidity: %.2f %%\n", currentHumidity);
    Serial.printf("Pressure: %.2f hPa\n", pressure);
    
    lastReadTime = millis();
  }
  
  // Update PID controller
  updatePID();
  
  delay(10);
}

bool setupSensors() {
  Serial.println("Setting up sensors...");
  
  Wire.begin();
  
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      Serial.println("Could not find BME280 sensor!");
      return false;
    }
  }
  
  Serial.println("BME280 sensor found and initialized!");
  return true;
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setpoint", HTTP_POST, handleSetpoint);
  server.begin();
  
  if (MDNS.begin("knx-thermostat")) {
    Serial.println("mDNS responder started: http://knx-thermostat.local");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder");
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>KNX Thermostat</title>";
  html += "<style>body{font-family:Arial;margin:20px;} .container{max-width:600px;margin:0 auto;}</style>";
  html += "</head><body><div class='container'>";
  html += "<h1>KNX Thermostat</h1>";
  
  html += "<div style='margin:20px 0;padding:15px;border:1px solid #ddd;border-radius:5px;'>";
  html += "<h2>Sensor Status</h2>";
  
  if (bmeAvailable) {
    html += "<p>Temperature: " + String(currentTemperature) + " °C</p>";
    html += "<p>Humidity: " + String(currentHumidity) + " %</p>";
    html += "<p>Setpoint: " + String(targetTemperature) + " °C</p>";
    html += "<form action='/setpoint' method='post'>";
    html += "<label for='setpoint'>Change Setpoint:</label>";
    html += "<input type='number' id='setpoint' name='value' step='0.5' value='" + String(targetTemperature) + "'>";
    html += "<input type='submit' value='Set'>";
    html += "</form>";
    html += "<p>Valve Position: " + String(valvePosition) + " %</p>";
    html += "<p>Heating: " + String(heatingActive ? "Active" : "Inactive") + "</p>";
  } else {
    html += "<p>BME280 sensor not found</p>";
  }
  
  html += "</div>";
  
  html += "<div style='margin:20px 0;padding:15px;border:1px solid #ddd;border-radius:5px;'>";
  html += "<h2>System Information</h2>";
  html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
  html += "<p>RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
  html += "<p>Uptime: " + String(millis() / 1000 / 60) + " minutes</p>";
  html += "</div>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleSetpoint() {
  if (server.hasArg("value")) {
    float setpoint = server.arg("value").toFloat();
    
    if (setpoint > 0 && setpoint < 40) {  // Sanity check
      targetTemperature = setpoint;
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "Redirecting...");
    } else {
      server.send(400, "text/plain", "Invalid setpoint value");
    }
  } else {
    server.send(400, "text/plain", "Missing setpoint value");
  }
}

void updatePID() {
  unsigned long currentTime = millis();
  
  // Update every PID_INTERVAL
  if (currentTime - lastPidUpdate >= PID_INTERVAL) {
    // Calculate error
    float error = targetTemperature - currentTemperature;
    
    // Calculate P, I, D terms
    float pTerm = kp * error;
    
    integral += ki * error;
    if (integral > 100) integral = 100;
    if (integral < 0) integral = 0;
    float iTerm = integral;
    
    float dTerm = kd * (error - lastError);
    
    // Calculate output
    float output = pTerm + iTerm + dTerm;
    if (output > 100) output = 100;
    if (output < 0) output = 0;
    
    valvePosition = output;
    heatingActive = (valvePosition > 0);
    
    // Debug output
    Serial.println("PID Update:");
    Serial.printf("Setpoint: %.2f°C, Current: %.2f°C, Error: %.2f°C\n", 
                  targetTemperature, currentTemperature, error);
    Serial.printf("P-term: %.2f, I-term: %.2f, D-term: %.2f\n", 
                  pTerm, iTerm, dTerm);
    Serial.printf("Output: %.2f%%\n", output);
    
    // Save for next update
    lastError = error;
    lastPidUpdate = currentTime;
  }
}