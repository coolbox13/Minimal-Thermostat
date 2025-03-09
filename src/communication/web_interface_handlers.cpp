#include "web_interface.h"
#include <ArduinoJson.h>

void WebInterface::handleRoot() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }
  String html = generateHtml();
  server.send(200, "text/html", html);
}

void WebInterface::handleSave() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!configManager) {
    server.send(500, "text/plain", "Configuration manager not available");
    return;
  }
  
  // Device settings
  if (server.hasArg("deviceName")) {
    configManager->setDeviceName(server.arg("deviceName").c_str());
  }
  
  if (server.hasArg("sendInterval")) {
    int interval = server.arg("sendInterval").toInt();
    if (interval < 1000) interval = 1000; // Minimum 1 second
    configManager->setSendInterval(interval);
    if (sensorInterface) {
      sensorInterface->setUpdateInterval(interval);
    }
  }
  
  if (server.hasArg("pidInterval")) {
    int interval = server.arg("pidInterval").toInt();
    if (interval < 1000) interval = 1000; // Minimum 1 second
    configManager->setPidInterval(interval);
    if (pidController) {
      pidController->setUpdateInterval(interval);
    }
  }
  
  // KNX physical address
  if (server.hasArg("knxPhysicalArea") && server.hasArg("knxPhysicalLine") && server.hasArg("knxPhysicalMember")) {
    int area = server.arg("knxPhysicalArea").toInt();
    int line = server.arg("knxPhysicalLine").toInt();
    int member = server.arg("knxPhysicalMember").toInt();
    
    configManager->setKnxPhysicalAddress(area, line, member);
    // KNX interface would need to be reinitialized with new address
  }
  
  // KNX enabled/disabled
  configManager->setKnxEnabled(server.hasArg("knxEnabled"));
  
  // KNX group addresses
  if (server.hasArg("knxTempArea") && server.hasArg("knxTempLine") && server.hasArg("knxTempMember")) {
    int area = server.arg("knxTempArea").toInt();
    int line = server.arg("knxTempLine").toInt();
    int member = server.arg("knxTempMember").toInt();
    
    configManager->setKnxTemperatureGA(area, line, member);
    if (knxInterface) {
      knxInterface->setTemperatureGA(area, line, member);
    }
  }
  
  if (server.hasArg("knxSetpointArea") && server.hasArg("knxSetpointLine") && server.hasArg("knxSetpointMember")) {
    int area = server.arg("knxSetpointArea").toInt();
    int line = server.arg("knxSetpointLine").toInt();
    int member = server.arg("knxSetpointMember").toInt();
    
    configManager->setKnxSetpointGA(area, line, member);
    if (knxInterface) {
      knxInterface->setSetpointGA(area, line, member);
    }
  }
  
  if (server.hasArg("knxValveArea") && server.hasArg("knxValveLine") && server.hasArg("knxValveMember")) {
    int area = server.arg("knxValveArea").toInt();
    int line = server.arg("knxValveLine").toInt();
    int member = server.arg("knxValveMember").toInt();
    
    configManager->setKnxValveGA(area, line, member);
    if (knxInterface) {
      knxInterface->setValvePositionGA(area, line, member);
    }
  }
  
  if (server.hasArg("knxModeArea") && server.hasArg("knxModeLine") && server.hasArg("knxModeMember")) {
    int area = server.arg("knxModeArea").toInt();
    int line = server.arg("knxModeLine").toInt();
    int member = server.arg("knxModeMember").toInt();
    
    configManager->setKnxModeGA(area, line, member);
    if (knxInterface) {
      knxInterface->setModeGA(area, line, member);
    }
  }
  
  // MQTT settings
  configManager->setMqttEnabled(server.hasArg("mqttEnabled"));
  
  if (server.hasArg("mqttServer")) {
    configManager->setMqttServer(server.arg("mqttServer").c_str());
  }
  
  if (server.hasArg("mqttPort")) {
    configManager->setMqttPort(server.arg("mqttPort").toInt());
  }
  
  if (server.hasArg("mqttUser")) {
    configManager->setMqttUser(server.arg("mqttUser").c_str());
  }
  
  if (server.hasArg("mqttPassword")) {
    configManager->setMqttPassword(server.arg("mqttPassword").c_str());
  }
  
  if (server.hasArg("mqttClientId")) {
    configManager->setMqttClientId(server.arg("mqttClientId").c_str());
  }
  
  // PID settings
  if (server.hasArg("kp")) {
    float kp = server.arg("kp").toFloat();
    configManager->setKp(kp);
    if (pidController) {
      pidController->setTunings(kp, pidController->getKi(), pidController->getKd());
    }
  }
  
  if (server.hasArg("ki")) {
    float ki = server.arg("ki").toFloat();
    configManager->setKi(ki);
    if (pidController) {
      pidController->setTunings(pidController->getKp(), ki, pidController->getKd());
    }
  }
  
  if (server.hasArg("kd")) {
    float kd = server.arg("kd").toFloat();
    configManager->setKd(kd);
    if (pidController) {
      pidController->setTunings(pidController->getKp(), pidController->getKi(), kd);
    }
  }
  
  if (server.hasArg("setpoint")) {
    float setpoint = server.arg("setpoint").toFloat();
    configManager->setSetpoint(setpoint);
    if (thermostatState) {
      thermostatState->setTargetTemperature(setpoint);
    }
  }
  
  // Save configuration
  configManager->saveConfig();
  
  // Redirect back to root
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "Saved. Redirecting...");
}

void WebInterface::handleGetStatus() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!thermostatState || !sensorInterface) {
    server.send(500, "text/plain", "Thermostat state or sensor interface not available");
    return;
  }

  StaticJsonDocument<512> doc;
  
  doc["currentTemp"] = sensorInterface->getTemperature();
  doc["targetTemp"] = thermostatState->getTargetTemperature();
  doc["humidity"] = sensorInterface->getHumidity();
  doc["heating"] = thermostatState->isHeating();
  doc["mode"] = thermostatState->getMode();
  
  if (pidController) {
    doc["pidOutput"] = pidController->getOutput();
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void WebInterface::handleSetpoint() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!thermostatState || !protocolManager) {
    server.send(500, "text/plain", "Thermostat state or protocol manager not available");
    return;
  }

  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Missing setpoint value");
    return;
  }

  float setpoint = server.arg("value").toFloat();
  if (setpoint < 5.0 || setpoint > 30.0) { // Basic range check
    server.send(400, "text/plain", "Invalid setpoint value (must be between 5-30°C)");
    return;
  }

  protocolManager->handleCommand(CMD_SET_TEMPERATURE, setpoint, SOURCE_WEB_API);
  server.send(200, "text/plain", "Setpoint updated");
}

void WebInterface::handleSaveConfig() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!configManager) {
    server.send(500, "text/plain", "Configuration manager not available");
    return;
  }

  // Parse JSON from POST data
  StaticJsonDocument<1024> doc;
  String jsonStr = server.arg("plain");
  DeserializationError error = deserializeJson(doc, jsonStr);
  
  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  // Update configuration
  bool needsSave = false;
  
  if (doc.containsKey("webUsername")) {
    configManager->setWebUsername(doc["webUsername"].as<const char*>());
    needsSave = true;
  }
  
  if (doc.containsKey("webPassword")) {
    configManager->setWebPassword(doc["webPassword"].as<const char*>());
    needsSave = true;
  }

  if (needsSave) {
    configManager->saveConfig();
    server.send(200, "text/plain", "Configuration saved");
  } else {
    server.send(400, "text/plain", "No valid configuration provided");
  }
}

void WebInterface::handleReboot() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }
  
  server.send(200, "text/plain", "Device is rebooting...");
  delay(500); // Give time for the response to be sent
  ESP.restart();
}

void WebInterface::handleFactoryReset() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!configManager) {
    server.send(500, "text/plain", "Configuration manager not available");
    return;
  }

  configManager->resetToDefaults();
  server.send(200, "text/plain", "Factory reset completed. Device will reboot...");
  delay(500); // Give time for the response to be sent
  ESP.restart();
}

void WebInterface::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}