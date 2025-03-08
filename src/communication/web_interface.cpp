#include "web_interface.h"

WebInterface::WebInterface() : 
  server(80),
  thermostatState(nullptr),
  configManager(nullptr),
  sensorInterface(nullptr),
  knxInterface(nullptr),
  mqttInterface(nullptr),
  pidController(nullptr),
  lastRequestTime(0),
  requestCount(0) {
}

bool WebInterface::begin(ThermostatState* state, 
                        ConfigManager* config, 
                        SensorInterface* sensor,
                        KNXInterface* knx,
                        MQTTInterface* mqtt,
                        PIDController* pid) {
  
  thermostatState = state;
  configManager = config;
  sensorInterface = sensor;
  knxInterface = knx;
  mqttInterface = mqtt;
  pidController = pid;
  
  // Set up web server routes
  server.on("/", HTTP_GET, [this](){ this->handleRoot(); });
  server.on("/status", HTTP_GET, [this](){ this->handleGetStatus(); });
  server.on("/setpoint", HTTP_POST, [this](){ this->handleSetpoint(); });
  server.on("/save", HTTP_POST, [this](){ this->handleSaveConfig(); });
  server.on("/reboot", HTTP_ANY, [this](){ this->handleReboot(); });
  server.on("/reset", HTTP_ANY, [this](){ this->handleFactoryReset(); });
  
  // Handle file read from LittleFS for static files
  server.onNotFound([this](){ 
    if (!this->handleFileRead(server.uri())) {
      this->handleNotFound();
    }
  });
  
  // Start the web server
  server.begin();
  
  // Set up mDNS
  setupMDNS();
  
  Serial.println("Web server started");
  return true;
}

void WebInterface::handle() {
  // Rate limiting to prevent DoS attacks
  unsigned long currentTime = millis();
  if (currentTime - lastRequestTime > 60000) { // Reset counter every minute
    lastRequestTime = currentTime;
    requestCount = 0;
  }
  
  if (requestCount < MAX_REQUESTS_PER_MINUTE) {
    server.handleClient();
    requestCount++;
  }
}

void WebInterface::handleRoot() {
  if (!isAuthenticated()) {
    requestAuthentication();
    return;
  }

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>KNX Thermostat</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; }";
  html += "label { display: inline-block; width: 120px; margin-bottom: 10px; }";
  html += "input[type='number'] { width: 60px; }";
  html += "input[type='checkbox'] { width: auto; }";
  html += ".form-group { margin-bottom: 15px; border-bottom: 1px solid #ccc; padding-bottom: 15px; }";
  html += "h2 { color: #333; }";
  html += "button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; }";
  html += "button:hover { background-color: #45a049; }";
  html += ".protocol-toggle { margin-bottom: 10px; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>KNX Thermostat Configuration</h1>";
  
  html += "<form action='/save' method='post'>";
  
  // Device Settings
  html += "<div class='form-group'>";
  html += "<h2>Device Settings</h2>";
  html += "<label for='deviceName'>Device Name:</label>";
  html += "<input type='text' id='deviceName' name='deviceName' value='" + 
          String(configManager ? configManager->getDeviceName() : "KNX-Thermostat") + "'><br>";
  
  html += "<label for='sendInterval'>Send Interval (ms):</label>";
  html += "<input type='number' id='sendInterval' name='sendInterval' min='1000' step='1000' value='" + 
          String(configManager ? configManager->getSendInterval() : 10000) + "'><br>";
  
  html += "<label for='pidInterval'>PID Interval (ms):</label>";
  html += "<input type='number' id='pidInterval' name='pidInterval' min='1000' step='1000' value='" + 
          String(configManager ? configManager->getPidInterval() : 30000) + "'><br>";
  html += "</div>";
  
  // KNX Settings
  html += "<div class='form-group'>";
  html += "<h2>KNX Settings</h2>";
  
  // KNX Toggle
  html += "<div class='protocol-toggle'>";
  html += "<label for='knxEnabled'>Enable KNX:</label>";
  html += "<input type='checkbox' id='knxEnabled' name='knxEnabled' " + 
          String(configManager && configManager->getKnxEnabled() ? "checked" : "") + "><br>";
  html += "</div>";
  
  // KNX Physical Address
  html += "<h3>KNX Physical Address</h3>";
  html += "<label for='knxPhysicalArea'>Area:</label>";
  html += "<input type='number' id='knxPhysicalArea' name='knxPhysicalArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxPhysicalArea() : 1) + "'>";
  
  html += "<label for='knxPhysicalLine'>Line:</label>";
  html += "<input type='number' id='knxPhysicalLine' name='knxPhysicalLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxPhysicalLine() : 1) + "'>";
  
  html += "<label for='knxPhysicalMember'>Member:</label>";
  html += "<input type='number' id='knxPhysicalMember' name='knxPhysicalMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxPhysicalMember() : 201) + "'><br>";
  
  // Temperature Group Address
  html += "<h3>Temperature Group Address</h3>";
  html += "<label for='knxTempArea'>Area:</label>";
  html += "<input type='number' id='knxTempArea' name='knxTempArea' min='0' max='31' value='" + 
          String(configManager ? configManager->getKnxTempArea() : 3) + "'>";
  
  html += "<label for='knxTempLine'>Line:</label>";
  html += "<input type='number' id='knxTempLine' name='knxTempLine' min='0' max='7' value='" + 
          String(configManager ? configManager->getKnxTempLine() : 1) + "'>";
  
  html += "<label for='knxTempMember'>Member:</label>";
  html += "<input type='number' id='knxTempMember' name='knxTempMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxTempMember() : 0) + "'><br>";
  
  // Setpoint Group Address
  html += "<h3>Setpoint Group Address</h3>";
  html += "<label for='knxSetpointArea'>Area:</label>";
  html += "<input type='number' id='knxSetpointArea' name='knxSetpointArea' min='0' max='31' value='" + 
          String(configManager ? configManager->getKnxSetpointArea() : 3) + "'>";
  
  html += "<label for='knxSetpointLine'>Line:</label>";
  html += "<input type='number' id='knxSetpointLine' name='knxSetpointLine' min='0' max='7' value='" + 
          String(configManager ? configManager->getKnxSetpointLine() : 2) + "'>";
  
  html += "<label for='knxSetpointMember'>Member:</label>";
  html += "<input type='number' id='knxSetpointMember' name='knxSetpointMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxSetpointMember() : 0) + "'><br>";
  
  // Valve Group Address
  html += "<h3>Valve Group Address</h3>";
  html += "<label for='knxValveArea'>Area:</label>";
  html += "<input type='number' id='knxValveArea' name='knxValveArea' min='0' max='31' value='" + 
          String(configManager ? configManager->getKnxValveArea() : 3) + "'>";
  
  html += "<label for='knxValveLine'>Line:</label>";
  html += "<input type='number' id='knxValveLine' name='knxValveLine' min='0' max='7' value='" + 
          String(configManager ? configManager->getKnxValveLine() : 3) + "'>";
  
  html += "<label for='knxValveMember'>Member:</label>";
  html += "<input type='number' id='knxValveMember' name='knxValveMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxValveMember() : 0) + "'><br>";
  
  // Mode Group Address
  html += "<h3>Mode Group Address</h3>";
  html += "<label for='knxModeArea'>Area:</label>";
  html += "<input type='number' id='knxModeArea' name='knxModeArea' min='0' max='31' value='" + 
          String(configManager ? configManager->getKnxModeArea() : 3) + "'>";
  
  html += "<label for='knxModeLine'>Line:</label>";
  html += "<input type='number' id='knxModeLine' name='knxModeLine' min='0' max='7' value='" + 
          String(configManager ? configManager->getKnxModeLine() : 4) + "'>";
  
  html += "<label for='knxModeMember'>Member:</label>";
  html += "<input type='number' id='knxModeMember' name='knxModeMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxModeMember() : 0) + "'><br>";
  
  html += "</div>";
  
  // MQTT Settings
  html += "<div class='form-group'>";
  html += "<h2>MQTT Settings</h2>";
  
  // MQTT Toggle
  html += "<div class='protocol-toggle'>";
  html += "<label for='mqttEnabled'>Enable MQTT:</label>";
  html += "<input type='checkbox' id='mqttEnabled' name='mqttEnabled' " + 
          String(configManager && configManager->getMqttEnabled() ? "checked" : "") + "><br>";
  html += "</div>";
  
  html += "<label for='mqttServer'>MQTT Server:</label>";
  html += "<input type='text' id='mqttServer' name='mqttServer' value='" + 
          String(configManager ? configManager->getMqttServer() : "192.168.178.32") + "'><br>";
  
  html += "<label for='mqttPort'>MQTT Port:</label>";
  html += "<input type='number' id='mqttPort' name='mqttPort' value='" + 
          String(configManager ? configManager->getMqttPort() : 1883) + "'><br>";
  
  html += "<label for='mqttUser'>MQTT User:</label>";
  html += "<input type='text' id='mqttUser' name='mqttUser' value='" + 
          String(configManager ? configManager->getMqttUser() : "") + "'><br>";
  
  html += "<label for='mqttPassword'>MQTT Password:</label>";
  html += "<input type='password' id='mqttPassword' name='mqttPassword' value='" + 
          String(configManager ? configManager->getMqttPassword() : "") + "'><br>";
  
  html += "<label for='mqttClientId'>Client ID:</label>";
  html += "<input type='text' id='mqttClientId' name='mqttClientId' value='" + 
          String(configManager ? configManager->getMqttClientId() : "ESP32Thermostat") + "'><br>";
  html += "</div>";
  
  // PID Settings
  html += "<div class='form-group'>";
  html += "<h2>PID Settings</h2>";
  html += "<label for='kp'>Proportional (Kp):</label>";
  html += "<input type='number' id='kp' name='kp' step='0.1' value='" + 
          String(configManager ? configManager->getKp() : 1.0) + "'><br>";
  
  html += "<label for='ki'>Integral (Ki):</label>";
  html += "<input type='number' id='ki' name='ki' step='0.01' value='" + 
          String(configManager ? configManager->getKi() : 0.1) + "'><br>";
  
  html += "<label for='kd'>Derivative (Kd):</label>";
  html += "<input type='number' id='kd' name='kd' step='0.001' value='" + 
          String(configManager ? configManager->getKd() : 0.01) + "'><br>";
  
  html += "<label for='setpoint'>Setpoint (°C):</label>";
  html += "<input type='number' id='setpoint' name='setpoint' step='0.5' value='" + 
          String(configManager ? configManager->getSetpoint() : 21.0) + "'><br>";
  html += "</div>";
  
  // Current Status
  html += "<div class='form-group'>";
  html += "<h2>Current Status</h2>";
  html += "<p>Temperature: " + String(thermostatState ? thermostatState->currentTemperature : 0.0) + " °C</p>";
  html += "<p>Humidity: " + String(thermostatState ? thermostatState->currentHumidity : 0.0) + " %</p>";
  html += "<p>Pressure: " + String(thermostatState ? thermostatState->currentPressure : 0.0) + " hPa</p>";
  html += "<p>Valve Position: " + String(thermostatState ? thermostatState->valvePosition : 0.0) + " %</p>";
  html += "<p>Heating Active: " + String(thermostatState && thermostatState->heatingActive ? "Yes" : "No") + "</p>";
  html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
  html += "<p>RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
  
  // Status indicators for protocols
  if (configManager) {
    html += "<p>KNX Status: " + 
            String(configManager->getKnxEnabled() ? "Connected" : "Disabled") + "</p>";
    html += "<p>MQTT Status: " + 
            String(configManager->getMqttEnabled() ? 
                 (mqttInterface && mqttInterface->isConnected() ? "Connected" : "Disconnected") : 
                 "Disabled") + "</p>";
  }
  html += "</div>";
  
  html += "<button type='submit'>Save Configuration</button>";
  html += "</form>";
  
  html += "<div style='margin-top: 20px;'>";
  html += "<a href='/reboot'><button type='button'>Reboot Device</button></a>";
  html += "<a href='/reset'><button type='button' style='background-color: #f44336; margin-left: 10px;'>Factory Reset</button></a>";
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

void WebInterface::setupMDNS() {
  // Set up mDNS responder with hostname from config or default
  String hostname = (configManager ? String(configManager->getDeviceName()) : "knx-thermostat");
  hostname.toLowerCase();
  hostname.replace(" ", "-");
  
  // Remove special characters from hostname
  String validHostname = "";
  for (unsigned int i = 0; i < hostname.length(); i++) {
    char c = hostname.charAt(i);
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
      validHostname += c;
    }
  }
  
  // Ensure hostname is not empty
  if (validHostname.length() == 0) {
    validHostname = "knx-thermostat";
  }
  
  if (MDNS.begin(validHostname.c_str())) {
    Serial.print("mDNS responder started: http://");
    Serial.print(validHostname);
    Serial.println(".local");
    
    // Add service to mDNS
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder");
  }
}

bool WebInterface::handleFileRead(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  String contentType = "text/plain";
  if(path.endsWith(".html")) contentType = "text/html";
  else if(path.endsWith(".css")) contentType = "text/css";
  else if(path.endsWith(".js")) contentType = "application/javascript";
  else if(path.endsWith(".ico")) contentType = "image/x-icon";
  else if(path.endsWith(".json")) contentType = "application/json";
  
  // Check if file exists in LittleFS
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    if (file) {
      server.streamFile(file, contentType);
      file.close();
      return true;
    }
  }
  
  return false;
}

bool WebInterface::isAuthenticated() {
  // Basic authentication - implement if needed
  return true;
}

void WebInterface::requestAuthentication() {
  server.sendHeader("WWW-Authenticate", "Basic realm=\"KNX Thermostat\"");
  server.send(401, "text/plain", "Authentication required");
}