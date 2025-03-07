#include "web_interface.h"
#include <ArduinoJson.h>

WebInterface::WebInterface() : 
  server(80),
  thermostatState(nullptr),
  sensorInterface(nullptr),
  pidController(nullptr) {
}

bool WebInterface::begin(ThermostatState* state, 
                        SensorInterface* sensor,
                        PIDController* pid) {
  
  thermostatState = state;
  sensorInterface = sensor;
  pidController = pid;
  
  // Set up web server routes
  server.on("/", HTTP_GET, [this](){ this->handleRoot(); });
  server.on("/status", HTTP_GET, [this](){ this->handleGetStatus(); });
  server.on("/setpoint", HTTP_POST, [this](){ this->handleSetpoint(); });
  server.onNotFound([this](){ this->handleNotFound(); });
  
  // Start the web server
  server.begin();
  
  // Set up mDNS
  setupMDNS();
  
  Serial.println("Web server started");
  return true;
}

void WebInterface::handle() {
  server.handleClient();
}

void WebInterface::handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>KNX Thermostat</title>";
  html += "<style>body{font-family:Arial;margin:20px;} .container{max-width:600px;margin:0 auto;}</style>";
  html += "</head><body><div class='container'>";
  html += "<h1>KNX Thermostat</h1>";
  
  html += "<div style='margin:20px 0;padding:15px;border:1px solid #ddd;border-radius:5px;'>";
  html += "<h2>Sensor Status</h2>";
  
  if (sensorInterface && sensorInterface->isAvailable()) {
    float temperature = thermostatState->currentTemperature;
    float humidity = thermostatState->currentHumidity;
    float pressure = thermostatState->currentPressure;
    
    html += "<p>Temperature: " + String(temperature) + " °C</p>";
    html += "<p>Humidity: " + String(humidity) + " %</p>";
    html += "<p>Pressure: " + String(pressure) + " hPa</p>";
    html += "<p>Setpoint: " + String(thermostatState->targetTemperature) + " °C</p>";
    html += "<p>Valve Position: " + String(thermostatState->valvePosition) + " %</p>";

    // Setpoint form
    html += "<form action='/setpoint' method='post'>";
    html += "<label for='setpoint'>Change Setpoint:</label>";
    html += "<input type='number' id='setpoint' name='value' step='0.5' value='" + String(thermostatState->targetTemperature) + "'>";
    html += "<input type='submit' value='Set'>";
    html += "</form>";

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

void WebInterface::handleGetStatus() {
  if (!thermostatState) {
    server.send(500, "text/plain", "Thermostat state not available");
    return;
  }
  
  DynamicJsonDocument doc(512);
  
  // Device information
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis() / 1000;
  
  // Sensor readings
  doc["temperature"] = thermostatState->currentTemperature;
  doc["humidity"] = thermostatState->currentHumidity;
  doc["pressure"] = thermostatState->currentPressure;
  
  // Control state
  doc["setpoint"] = thermostatState->targetTemperature;
  doc["valvePosition"] = thermostatState->valvePosition;
  doc["mode"] = static_cast<int>(thermostatState->operatingMode);
  doc["heatingActive"] = thermostatState->heatingActive;
  
  // PID information
  if (pidController) {
    JsonObject pid = doc.createNestedObject("pid");
    pid["kp"] = pidController->getKp();
    pid["ki"] = pidController->getKi();
    pid["kd"] = pidController->getKd();
    pid["error"] = pidController->getLastError();
    pid["pTerm"] = pidController->getProportionalTerm();
    pid["iTerm"] = pidController->getIntegralTerm();
    pid["dTerm"] = pidController->getDerivativeTerm();
  }
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

void WebInterface::handleSetpoint() {
  if (!thermostatState) {
    server.send(500, "text/plain", "Thermostat state not available");
    return;
  }
  
  if (server.hasArg("value")) {
    float setpoint = server.arg("value").toFloat();
    
    if (setpoint > 0 && setpoint < 40) {  // Sanity check
      thermostatState->setTargetTemperature(setpoint);
      server.sendHeader("Location", "/", true);   // Redirect to root
      server.send(302, "text/plain", "Redirecting...");
    } else {
      server.send(400, "text/plain", "Invalid setpoint value");
    }
  } else {
    server.send(400, "text/plain", "Missing setpoint value");
  }
}

void WebInterface::handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void WebInterface::setupMDNS() {
  // Set up mDNS responder
  const char* hostname = "knx-thermostat";
  
  if (MDNS.begin(hostname)) {
    Serial.print("mDNS responder started: http://");
    Serial.print(hostname);
    Serial.println(".local");
    
    // Add service to mDNS
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder");
  }
}