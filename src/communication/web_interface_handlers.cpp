#include "web/web_interface.h"
#include "config_manager.h"
#include "sensor_interface.h"
#include "thermostat_state.h"
#include "pid_controller.h"
#include "protocol_manager.h"
#include "communication/knx/knx_interface.h"
#include "mqtt_interface.h"
#include <ArduinoJson.h>

void WebInterface::handleRoot(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request)) {
    requestAuthentication(request);
    return;
  }
  String html = generateHtml();
  AsyncWebServerResponse* response = request->beginResponse(200, "text/html", html);
  addSecurityHeaders(response);
  request->send(response);
}

void WebInterface::handleSave(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request) || !validateCSRFToken(request)) {
    requestAuthentication(request);
    return;
  }

  if (!configManager) {
    AsyncWebServerResponse* response = request->beginResponse(500, "text/plain", "Configuration manager not available");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }
  
  // Device settings
  if (request->hasParam("deviceName", true)) {
    configManager->setDeviceName(request->getParam("deviceName", true)->value().c_str());
  }
  
  if (request->hasParam("sendInterval", true)) {
    int interval = request->getParam("sendInterval", true)->value().toInt();
    if (interval < 1000) interval = 1000; // Minimum 1 second
    configManager->setSendInterval(interval);
    if (sensorInterface) {
      sensorInterface->setUpdateInterval(interval);
    }
  }
  
  if (request->hasParam("pidInterval", true)) {
    int interval = request->getParam("pidInterval", true)->value().toInt();
    if (interval < 1000) interval = 1000; // Minimum 1 second
    configManager->setPidInterval(interval);
    if (pidController) {
      pidController->setUpdateInterval(interval);
    }
  }
  
  // KNX physical address
  if (request->hasParam("knxPhysicalArea", true) && request->hasParam("knxPhysicalLine", true) && request->hasParam("knxPhysicalMember", true)) {
    int area = request->getParam("knxPhysicalArea", true)->value().toInt();
    int line = request->getParam("knxPhysicalLine", true)->value().toInt();
    int member = request->getParam("knxPhysicalMember", true)->value().toInt();
    
    configManager->setKnxPhysicalAddress(area, line, member);
    // KNX interface would need to be reinitialized with new address
  }
  
  // KNX enabled/disabled
  configManager->setKnxEnabled(request->hasParam("knxEnabled", true));
  
  // KNX group addresses
  if (request->hasParam("knxTemperatureGA_area", true) && request->hasParam("knxTemperatureGA_line", true) && request->hasParam("knxTemperatureGA_member", true)) {
    uint8_t area = request->getParam("knxTemperatureGA_area", true)->value().toInt();
    uint8_t line = request->getParam("knxTemperatureGA_line", true)->value().toInt();
    uint8_t member = request->getParam("knxTemperatureGA_member", true)->value().toInt();
    configManager->setKnxTemperatureGA(area, line, member);
    if (knxInterface) {
      knxInterface->setTemperatureGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  if (request->hasParam("knxSetpointGA_area", true) && request->hasParam("knxSetpointGA_line", true) && request->hasParam("knxSetpointGA_member", true)) {
    uint8_t area = request->getParam("knxSetpointGA_area", true)->value().toInt();
    uint8_t line = request->getParam("knxSetpointGA_line", true)->value().toInt();
    uint8_t member = request->getParam("knxSetpointGA_member", true)->value().toInt();
    configManager->setKnxSetpointGA(area, line, member);
    if (knxInterface) {
      knxInterface->setSetpointGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  if (request->hasParam("knxValveGA_area", true) && request->hasParam("knxValveGA_line", true) && request->hasParam("knxValveGA_member", true)) {
    uint8_t area = request->getParam("knxValveGA_area", true)->value().toInt();
    uint8_t line = request->getParam("knxValveGA_line", true)->value().toInt();
    uint8_t member = request->getParam("knxValveGA_member", true)->value().toInt();
    configManager->setKnxValveGA(area, line, member);
    if (knxInterface) {
      knxInterface->setValvePositionGA({
        .area = area,
        .line = line,
        .member = member
      });
    }
  }
  
  if (request->hasParam("knxModeGA_area", true) && request->hasParam("knxModeGA_line", true) && request->hasParam("knxModeGA_member", true)) {
    uint8_t area = request->getParam("knxModeGA_area", true)->value().toInt();
    uint8_t line = request->getParam("knxModeGA_line", true)->value().toInt();
    uint8_t member = request->getParam("knxModeGA_member", true)->value().toInt();
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
  configManager->setMqttEnabled(request->hasParam("mqttEnabled", true));
  
  if (request->hasParam("mqtt_server", true) && request->hasParam("mqtt_port", true)) {
    const char* mqttServer = request->getParam("mqtt_server", true)->value().c_str();
    uint16_t mqttPort = request->getParam("mqtt_port", true)->value().toInt();
    configManager->setMqttServer(mqttServer);
    configManager->setMqttPort(mqttPort);
    if (mqttInterface) {
      mqttInterface->setServer(mqttServer, mqttPort);
    }
  }
  
  if (request->hasParam("mqtt_user", true) && request->hasParam("mqtt_pass", true)) {
    const char* mqttUser = request->getParam("mqtt_user", true)->value().c_str();
    const char* mqttPassword = request->getParam("mqtt_pass", true)->value().c_str();
    configManager->setMqttUser(mqttUser);
    configManager->setMqttPassword(mqttPassword);
    if (mqttInterface) {
      mqttInterface->setCredentials(mqttUser, mqttPassword);
    }
  }
  
  if (request->hasParam("mqtt_clientId", true)) {
    const char* mqttClientId = request->getParam("mqtt_clientId", true)->value().c_str();
    configManager->setMqttClientId(mqttClientId);
    if (mqttInterface) {
      mqttInterface->setClientId(mqttClientId);
    }
  }
  
  // PID settings
  if (request->hasParam("kp", true)) {
    float kp = request->getParam("kp", true)->value().toFloat();
    configManager->setKp(kp);
    if (pidController) {
      pidController->setTunings(kp, pidController->getKi(), pidController->getKd());
    }
  }
  
  if (request->hasParam("ki", true)) {
    float ki = request->getParam("ki", true)->value().toFloat();
    configManager->setKi(ki);
    if (pidController) {
      pidController->setTunings(pidController->getKp(), ki, pidController->getKd());
    }
  }
  
  if (request->hasParam("kd", true)) {
    float kd = request->getParam("kd", true)->value().toFloat();
    configManager->setKd(kd);
    if (pidController) {
      pidController->setTunings(pidController->getKp(), pidController->getKi(), kd);
    }
  }
  
  if (request->hasParam("setpoint", true)) {
    float setpoint = request->getParam("setpoint", true)->value().toFloat();
    configManager->setSetpoint(setpoint);
    if (thermostatState) {
      thermostatState->setTargetTemperature(setpoint);
    }
  }
  
  // Save configuration
  configManager->saveConfig();
  
  request->redirect("/");
}

void WebInterface::handleGetStatus(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request)) {
    requestAuthentication(request);
    return;
  }

  if (!thermostatState || !sensorInterface) {
    AsyncWebServerResponse* response = request->beginResponse(500, "text/plain", "Thermostat state or sensor interface not available");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }

  DynamicJsonDocument doc(1024);
  
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
  
  AsyncWebServerResponse* jsonResponse = request->beginResponse(200, "application/json", response);
  addSecurityHeaders(jsonResponse);
  request->send(jsonResponse);
}

void WebInterface::handleSetpoint(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request) || !validateCSRFToken(request)) {
    requestAuthentication(request);
    return;
  }

  if (request->method() != HTTP_POST) {
    AsyncWebServerResponse* response = request->beginResponse(405, "text/plain", "Method Not Allowed");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }

  if (!request->hasParam("value", true)) {
    AsyncWebServerResponse* response = request->beginResponse(400, "text/plain", "Missing setpoint value");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }

  float setpoint = request->getParam("value", true)->value().toFloat();
  if (protocolManager) {
    protocolManager->handleIncomingCommand(
      CommandSource::SOURCE_WEB_API,
      CommandType::CMD_SET_TEMPERATURE,
      setpoint
    );
    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"status\":\"success\"}");
    addSecurityHeaders(response);
    request->send(response);
  } else {
    AsyncWebServerResponse* response = request->beginResponse(500, "text/plain", "Protocol manager not initialized");
    addSecurityHeaders(response);
    request->send(response);
  }
}

void WebInterface::handleSaveConfig(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request) || !validateCSRFToken(request)) {
    requestAuthentication(request);
    return;
  }

  if (request->method() != HTTP_POST) {
    AsyncWebServerResponse* response = request->beginResponse(405, "text/plain", "Method Not Allowed");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }

  if (!configManager) {
    AsyncWebServerResponse* response = request->beginResponse(500, "text/plain", "Configuration manager not available");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }

  // Parse JSON from POST data
  StaticJsonDocument<1024> doc;
  String jsonStr = request->arg("plain");
  DeserializationError error = deserializeJson(doc, jsonStr);

  if (error) {
    AsyncWebServerResponse* response = request->beginResponse(400, "text/plain", "Invalid JSON");
    addSecurityHeaders(response);
    request->send(response);
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
  AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"status\":\"success\"}");
  addSecurityHeaders(response);
  request->send(response);
}

void WebInterface::handleReboot(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request) || !validateCSRFToken(request)) {
    requestAuthentication(request);
    return;
  }
  
  AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "Device will reboot in 5 seconds...");
  addSecurityHeaders(response);
  request->send(response);
  
  delay(5000);
  ESP.restart();
}

void WebInterface::handleFactoryReset(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request) || !validateCSRFToken(request)) {
    requestAuthentication(request);
    return;
  }

  if (request->method() != HTTP_POST) {
    AsyncWebServerResponse* response = request->beginResponse(405, "text/plain", "Method Not Allowed");
    addSecurityHeaders(response);
    request->send(response);
    return;
  }

  configManager->resetToDefaults();
  
  AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "Factory reset complete. Device will reboot in 5 seconds...");
  addSecurityHeaders(response);
  request->send(response);
  
  delay(5000);
  ESP.restart();
}

void WebInterface::handleNotFound(AsyncWebServerRequest* request) {
  if (!handleFileRead(request, request->url())) {
    AsyncWebServerResponse* response = request->beginResponse(404, "text/plain", "File Not Found");
    addSecurityHeaders(response);
    request->send(response);
  }
}