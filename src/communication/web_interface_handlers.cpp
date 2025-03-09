#include "web/web_interface.h"
#include "config_manager.h"
#include "sensor_interface.h"
#include "thermostat_state.h"
#include "pid_controller.h"
#include "protocol_manager.h"
#include "communication/knx/knx_interface.h"
#include "mqtt_interface.h"
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

  if (!validateCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
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
    uint32_t interval = server.arg("sendInterval").toInt();
    configManager->setSendInterval(interval);
    if (sensorInterface) {
      sensorInterface->setSendInterval(interval);
    }
  }
  
  if (server.hasArg("pidInterval")) {
    uint32_t interval = server.arg("pidInterval").toInt();
    configManager->setPidInterval(interval);
    if (pidController) {
      pidController->setInterval(interval);
    }
  }
  
  // KNX physical address
  if (server.hasArg("knxArea") && server.hasArg("knxLine") && server.hasArg("knxMember")) {
    uint8_t area = server.arg("knxArea").toInt();
    uint8_t line = server.arg("knxLine").toInt();
    uint8_t member = server.arg("knxMember").toInt();
    
    configManager->setKnxPhysicalAddress(area, line, member);
    // KNX interface would need to be reinitialized with new address
  }
  
  // KNX enabled/disabled
  configManager->setKnxEnabled(server.hasArg("knxEnabled"));
  
  // KNX group addresses
  if (server.hasArg("knxTempArea") && server.hasArg("knxTempLine") && server.hasArg("knxTempMember")) {
    uint8_t area = server.arg("knxTempArea").toInt();
    uint8_t line = server.arg("knxTempLine").toInt();
    uint8_t member = server.arg("knxTempMember").toInt();
    configManager->setKnxTemperatureGA(area, line, member);
    if (protocolManager) {
      protocolManager->updateKnxTemperatureGA(area, line, member);
    }
  }
  
  if (server.hasArg("knxSetpointArea") && server.hasArg("knxSetpointLine") && server.hasArg("knxSetpointMember")) {
    uint8_t area = server.arg("knxSetpointArea").toInt();
    uint8_t line = server.arg("knxSetpointLine").toInt();
    uint8_t member = server.arg("knxSetpointMember").toInt();
    configManager->setKnxSetpointGA(area, line, member);
    if (protocolManager) {
      protocolManager->updateKnxSetpointGA(area, line, member);
    }
  }
  
  if (server.hasArg("knxValveArea") && server.hasArg("knxValveLine") && server.hasArg("knxValveMember")) {
    uint8_t area = server.arg("knxValveArea").toInt();
    uint8_t line = server.arg("knxValveLine").toInt();
    uint8_t member = server.arg("knxValveMember").toInt();
    configManager->setKnxValveGA(area, line, member);
    if (protocolManager) {
      protocolManager->updateKnxValveGA(area, line, member);
    }
  }
  
  if (server.hasArg("knxModeArea") && server.hasArg("knxModeLine") && server.hasArg("knxModeMember")) {
    uint8_t area = server.arg("knxModeArea").toInt();
    uint8_t line = server.arg("knxModeLine").toInt();
    uint8_t member = server.arg("knxModeMember").toInt();
    configManager->setKnxModeGA(area, line, member);
    if (protocolManager) {
      protocolManager->updateKnxModeGA(area, line, member);
    }
  }
  
  // MQTT settings
  configManager->setMQTTEnabled(server.hasArg("mqttEnabled"));
  
  if (server.hasArg("mqttServer")) {
    const char* mqttServer = server.arg("mqttServer").c_str();
    configManager->setMQTTServer(mqttServer);
  }
  
  if (server.hasArg("mqttPort")) {
    uint16_t mqttPort = server.arg("mqttPort").toInt();
    configManager->setMQTTPort(mqttPort);
  }
  
  if (server.hasArg("mqttUser")) {
    const char* mqttUser = server.arg("mqttUser").c_str();
    configManager->setMQTTUser(mqttUser);
  }
  
  if (server.hasArg("mqttPassword")) {
    const char* mqttPassword = server.arg("mqttPassword").c_str();
    configManager->setMQTTPassword(mqttPassword);
  }
  
  if (server.hasArg("mqttClientId")) {
    const char* mqttClientId = server.arg("mqttClientId").c_str();
    configManager->setMQTTClientId(mqttClientId);
  }
  
  // PID parameters
  if (server.hasArg("pidKp")) {
    float kp = server.arg("pidKp").toFloat();
    if (pidController) {
      pidController->setKp(kp);
    }
  }
  
  if (server.hasArg("pidKi")) {
    float ki = server.arg("pidKi").toFloat();
    if (pidController) {
      pidController->setKi(ki);
    }
  }
  
  if (server.hasArg("pidKd")) {
    float kd = server.arg("pidKd").toFloat();
    if (pidController) {
      pidController->setKd(kd);
    }
  }
  
  // Setpoint
  if (server.hasArg("setpoint")) {
    float setpoint = server.arg("setpoint").toFloat();
    configManager->setSetpoint(setpoint);
    if (thermostatState) {
      thermostatState->setTargetTemperature(setpoint);
    }
  }
  
  // Save configuration
  configManager->saveConfig();
  
  server.send(200, "text/plain", "Settings saved");
}

void WebInterface::handleGetStatus() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!thermostatState || !sensorInterface) {
    server.send(500, "text/plain", "Internal server error");
    return;
  }

  StaticJsonDocument<256> doc;
  
  doc["currentTemp"] = sensorInterface->getTemperature();
  doc["targetTemp"] = thermostatState->getTargetTemperature();
  doc["mode"] = thermostatState->getMode();
  doc["output"] = 0.0;
  
  if (pidController) {
    doc["output"] = pidController->getOutput();
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

  if (!validateCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }

  if (!server.hasArg("setpoint")) {
    server.send(400, "text/plain", "Missing setpoint parameter");
    return;
  }

  float setpoint = server.arg("setpoint").toFloat();
  if (protocolManager) {
    protocolManager->setSetpoint(setpoint);
  }

  server.send(200, "text/plain", "Setpoint updated");
}

void WebInterface::handleSaveConfig() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!validateCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }

  String json = server.arg("plain");
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  if (doc.containsKey("webUsername")) {
    configManager->setWebUsername(doc["webUsername"].as<const char*>());
  }

  if (doc.containsKey("webPassword")) {
    configManager->setWebPassword(doc["webPassword"].as<const char*>());
  }

  configManager->saveConfig();
  server.send(200, "text/plain", "Configuration saved");
}

void WebInterface::handleReboot() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }
  
  server.send(200, "text/plain", "Device will reboot in 5 seconds...");
  
  delay(5000);
  ESP.restart();
}

void WebInterface::handleFactoryReset() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing CSRF token");
    return;
  }

  configManager->resetToDefaults();
  
  server.send(200, "text/plain", "Factory reset complete. Device will reboot in 5 seconds...");
  
  delay(5000);
  ESP.restart();
}

void WebInterface::handleNotFound() {
  if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "File Not Found");
  }
}