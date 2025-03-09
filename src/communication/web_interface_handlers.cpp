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
  if (server.hasArg("knxTemperatureGA")) {
    uint8_t area = server.arg("knxTemperatureGA_area").toInt();
    uint8_t line = server.arg("knxTemperatureGA_line").toInt();
    uint8_t member = server.arg("knxTemperatureGA_member").toInt();
    configManager->setKnxTemperatureGA(area, line, member);
    if (knxInterface) {
      knxInterface->setTemperatureGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  if (server.hasArg("knxSetpointGA")) {
    uint8_t area = server.arg("knxSetpointGA_area").toInt();
    uint8_t line = server.arg("knxSetpointGA_line").toInt();
    uint8_t member = server.arg("knxSetpointGA_member").toInt();
    configManager->setKnxSetpointGA(area, line, member);
    if (knxInterface) {
      knxInterface->setSetpointGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  if (server.hasArg("knxValveGA")) {
    uint8_t area = server.arg("knxValveGA_area").toInt();
    uint8_t line = server.arg("knxValveGA_line").toInt();
    uint8_t member = server.arg("knxValveGA_member").toInt();
    configManager->setKnxValveGA(area, line, member);
    if (knxInterface) {
      knxInterface->setValvePositionGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  if (server.hasArg("knxModeGA")) {
    uint8_t area = server.arg("knxModeGA_area").toInt();
    uint8_t line = server.arg("knxModeGA_line").toInt();
    uint8_t member = server.arg("knxModeGA_member").toInt();
    configManager->setKnxModeGA(area, line, member);
    if (knxInterface) {
      knxInterface->setModeGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  // MQTT settings
  configManager->setMqttEnabled(server.hasArg("mqttEnabled"));
  
  if (server.hasArg("mqttServer") && server.hasArg("mqttPort")) {
    const char* mqttServer = server.arg("mqttServer").c_str();
    uint16_t mqttPort = server.arg("mqttPort").toInt();
    configManager->setMqttServer(mqttServer);
    configManager->setMqttPort(mqttPort);
    if (mqttInterface) {
      mqttInterface->setServer(mqttServer, mqttPort);
    }
  }
  
  if (server.hasArg("mqttUser") && server.hasArg("mqttPassword")) {
    const char* mqttUser = server.arg("mqttUser").c_str();
    const char* mqttPassword = server.arg("mqttPassword").c_str();
    configManager->setMqttUser(mqttUser);
    configManager->setMqttPassword(mqttPassword);
    if (mqttInterface) {
      mqttInterface->setCredentials(mqttUser, mqttPassword);
    }
  }
  
  if (server.hasArg("mqttClientId")) {
    const char* mqttClientId = server.arg("mqttClientId").c_str();
    configManager->setMqttClientId(mqttClientId);
    if (mqttInterface) {
      mqttInterface->setClientId(mqttClientId);
    }
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

  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!validateCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }

  addSecurityHeaders();

  String json = server.arg("plain");
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  if (!doc.containsKey("setpoint")) {
    server.send(400, "text/plain", "Missing setpoint value");
    return;
  }

  float setpoint = doc["setpoint"];
  if (protocolManager) {
    protocolManager->handleIncomingCommand(
      CommandSource::SOURCE_WEB_API,
      CommandType::CMD_SET_TEMPERATURE,
      setpoint
    );
    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    server.send(500, "text/plain", "Protocol manager not initialized");
  }
}

void WebInterface::handleSaveConfig() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!validateCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }

  addSecurityHeaders();

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

  // Update web authentication
  if (doc.containsKey("webUsername")) {
    configManager->setWebUsername(doc["webUsername"].as<const char*>());
  }

  if (doc.containsKey("webPassword")) {
    configManager->setWebPassword(doc["webPassword"].as<const char*>());
  }

  // Save configuration
  configManager->saveConfig();
  server.send(200, "application/json", "{\"status\":\"success\"}");
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

  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!validateCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }

  addSecurityHeaders();

  if (configManager) {
    configManager->factoryReset();
    configManager->saveConfig();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Factory reset complete\"}");
    
    // Schedule a reboot
    delay(500);
    ESP.restart();
  } else {
    server.send(500, "text/plain", "Configuration manager not available");
  }
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