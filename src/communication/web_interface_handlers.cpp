#include "web_interface.h"
#include <ArduinoJson.h>

void WebInterface::handleRoot() {
  String html = generateHtml();
  server.send(200, "text/html", html);
}

void WebInterface::handleSave() {
  if (!configManager) {
    server.send(500, "text/plain", "Configuration manager not available");
    return;
  }
  
  // Device settings
  if (server.hasArg("deviceName")) {
    configManager->setDeviceName(server.arg("deviceName").c_str());
  }
  
  if (server.hasArg("sendInterval")) {
    configManager->setSendInterval(server.arg("sendInterval").toInt());
    if (sensorInterface) {
      sensorInterface->setUpdateInterval(server.arg("sendInterval").toInt());
    }
  }
  
  if (server.hasArg("pidInterval")) {
    configManager->setPidInterval(server.arg("pidInterval").toInt());
    if (pidController) {
      pidController->setUpdateInterval(server.arg("pidInterval").toInt());
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
  if (!thermostatState) {
    server.send(500, "text/plain", "Thermostat state not available");
    return;
  }
  
  DynamicJsonDocument doc(512);
  
  // Device information
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis() / 1000;
  doc["deviceName"] = configManager ? configManager->getDeviceName() : "KNX-Thermostat";
  
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
  
  // Connection state
  JsonObject protocols = doc.createNestedObject("protocols");
  
  if (configManager) {
    // KNX status
    JsonObject knx = protocols.createNestedObject("knx");
    knx["enabled"] = configManager->getKnxEnabled();
    knx["physicalAddress"] = String(configManager->getKnxPhysicalArea()) + "." + 
                             String(configManager->getKnxPhysicalLine()) + "." + 
                             String(configManager->getKnxPhysicalMember());
    
    // MQTT status
    JsonObject mqtt = protocols.createNestedObject("mqtt");
    mqtt["enabled"] = configManager->getMqttEnabled();
    mqtt["connected"] = mqttInterface ? mqttInterface->isConnected() : false;
    mqtt["server"] = configManager->getMqttServer();
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

  if (!thermostatState) {
    server.send(500, "text/plain", "Thermostat state not available");
    return;
  }
  
  if (server.hasArg("value")) {
    float setpoint = server.arg("value").toFloat();
    
    if (setpoint > 0 && setpoint < 40) {  // Sanity check
      // Use protocol manager to handle the command
      if (protocolManager) {
        protocolManager->handleIncomingCommand(SOURCE_WEB_API, CMD_SET_TEMPERATURE, setpoint);
      } else {
        thermostatState->setTargetTemperature(setpoint);
      }
      
      server.sendHeader("Location", "/", true);   // Redirect to root
      server.send(302, "text/plain", "Redirecting...");
    } else {
      server.send(400, "text/plain", "Invalid setpoint value");
    }
  } else {
    server.send(400, "text/plain", "Missing setpoint value");
  }
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
  
  // Handle device settings
  if (server.hasArg("deviceName")) {
    configManager->setDeviceName(server.arg("deviceName").c_str());
  }
  
  if (server.hasArg("sendInterval")) {
    configManager->setSendInterval(server.arg("sendInterval").toInt());
  }
  
  if (server.hasArg("pidInterval")) {
    configManager->setPidInterval(server.arg("pidInterval").toInt());
  }
  
  // Handle KNX settings
  configManager->setKnxEnabled(server.hasArg("knxEnabled"));
  
  if (server.hasArg("knxPhysicalArea") && server.hasArg("knxPhysicalLine") && server.hasArg("knxPhysicalMember")) {
    configManager->setKnxPhysicalAddress(
      server.arg("knxPhysicalArea").toInt(),
      server.arg("knxPhysicalLine").toInt(),
      server.arg("knxPhysicalMember").toInt()
    );
  }
  
  if (server.hasArg("knxTempArea") && server.hasArg("knxTempLine") && server.hasArg("knxTempMember")) {
    configManager->setKnxTemperatureGA(
      server.arg("knxTempArea").toInt(),
      server.arg("knxTempLine").toInt(),
      server.arg("knxTempMember").toInt()
    );
  }
  
  if (server.hasArg("knxSetpointArea") && server.hasArg("knxSetpointLine") && server.hasArg("knxSetpointMember")) {
    configManager->setKnxSetpointGA(
      server.arg("knxSetpointArea").toInt(),
      server.arg("knxSetpointLine").toInt(),
      server.arg("knxSetpointMember").toInt()
    );
  }
  
  if (server.hasArg("knxValveArea") && server.hasArg("knxValveLine") && server.hasArg("knxValveMember")) {
    configManager->setKnxValveGA(
      server.arg("knxValveArea").toInt(),
      server.arg("knxValveLine").toInt(),
      server.arg("knxValveMember").toInt()
    );
  }
  
  if (server.hasArg("knxModeArea") && server.hasArg("knxModeLine") && server.hasArg("knxModeMember")) {
    configManager->setKnxModeGA(
      server.arg("knxModeArea").toInt(),
      server.arg("knxModeLine").toInt(),
      server.arg("knxModeMember").toInt()
    );
  }
  
  // Handle MQTT settings
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
  
  // Handle PID settings
  if (server.hasArg("kp")) {
    configManager->setKp(server.arg("kp").toFloat());
    if (pidController) {
      pidController->setTunings(
        server.arg("kp").toFloat(),
        pidController->getKi(),
        pidController->getKd()
      );
    }
  }
  
  if (server.hasArg("ki")) {
    configManager->setKi(server.arg("ki").toFloat());
    if (pidController) {
      pidController->setTunings(
        pidController->getKp(),
        server.arg("ki").toFloat(),
        pidController->getKd()
      );
    }
  }
  
  if (server.hasArg("kd")) {
    configManager->setKd(server.arg("kd").toFloat());
    if (pidController) {
      pidController->setTunings(
        pidController->getKp(),
        pidController->getKi(),
        server.arg("kd").toFloat()
      );
    }
  }
  
  if (server.hasArg("setpoint")) {
    float setpoint = server.arg("setpoint").toFloat();
    configManager->setSetpoint(setpoint);
    if (thermostatState) {
      thermostatState->setTargetTemperature(setpoint);
    }
  }
  
  // Save configuration to file
  configManager->saveConfig();
  
  // Redirect back to main page
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "Saved. Redirecting...");
}

void WebInterface::handleReboot() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  server.send(200, "text/html", 
    "<html><head><meta http-equiv='refresh' content='10;URL=/'></head><body>"
    "<h1>Device is rebooting</h1>"
    "<p>Please wait, the device will reboot in a few seconds.</p>"
    "<p>You will be redirected back to the home page in 10 seconds.</p>"
    "</body></html>"
  );
  
  delay(1000);
  ESP.restart();
}

void WebInterface::handleFactoryReset() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  if (configManager) {
    configManager->factoryReset();
  }
  
  server.send(200, "text/html", 
    "<html><head><meta http-equiv='refresh' content='10;URL=/'></head><body>"
    "<h1>Factory reset completed</h1>"
    "<p>The device has been reset to factory defaults and will reboot now.</p>"
    "<p>You will be redirected back to the home page in 10 seconds.</p>"
    "</body></html>"
  );
  
  delay(1000);
  ESP.restart();
}

void WebInterface::handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}