#include "web_interface.h"

String WebInterface::generateHtml() {
  String html = "<!DOCTYPE html><html lang='en'>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta charset='UTF-8'>";
  html += "<title>KNX Thermostat Configuration</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; }";
  html += "label { display: inline-block; width: 120px; margin-bottom: 10px; }";
  html += "input[type='number'] { width: 60px; }";
  html += ".form-group { margin-bottom: 15px; border-bottom: 1px solid #ccc; padding-bottom: 15px; }";
  html += "h2 { color: #333; }";
  html += ".btn { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; }";
  html += ".btn:hover { background-color: #45a049; }";
  html += ".btn-danger { background-color: #f44336; }";
  html += ".btn-danger:hover { background-color: #da190b; }";
  html += ".status-value { font-weight: bold; }";
  html += "</style></head><body>";
  html += "<div class='container' role='main'>";
  html += "<h1>KNX Thermostat Configuration</h1>";
  
  // Main configuration form
  html += "<form action='/save' method='post' id='configForm'>";
  
  // Device Settings
  html += "<div class='form-group' role='group' aria-labelledby='deviceSettings'>";
  html += "<h2 id='deviceSettings'>Device Settings</h2>";
  html += "<div><label for='deviceName'>Device Name:</label>";
  html += "<input type='text' id='deviceName' name='deviceName' required value='" + 
          String(configManager ? configManager->getDeviceName() : "KNX-Thermostat") + 
          "' aria-describedby='deviceNameHelp'></div>";
  html += "<small id='deviceNameHelp'>Enter a unique name for this device</small><br><br>";
  
  html += "<div><label for='sendInterval'>Send Interval (ms):</label>";
  html += "<input type='number' id='sendInterval' name='sendInterval' required min='1000' step='1000' value='" + 
          String(configManager ? configManager->getSendInterval() : 10000) + 
          "' aria-describedby='sendIntervalHelp'></div>";
  html += "<small id='sendIntervalHelp'>Minimum 1000ms (1 second)</small><br><br>";
  
  html += "<div><label for='pidInterval'>PID Interval (ms):</label>";
  html += "<input type='number' id='pidInterval' name='pidInterval' required min='1000' step='1000' value='" + 
          String(configManager ? configManager->getPidInterval() : 10000) + 
          "' aria-describedby='pidIntervalHelp'></div>";
  html += "<small id='pidIntervalHelp'>Minimum 1000ms (1 second)</small>";
  html += "</div>";
  
  // KNX Settings
  html += "<div class='form-group' role='group' aria-labelledby='knxSettings'>";
  html += "<h2 id='knxSettings'>KNX Settings</h2>";
  html += "<div><label for='knxEnabled'>KNX Enabled:</label>";
  html += "<input type='checkbox' id='knxEnabled' name='knxEnabled'" + 
          String(configManager && configManager->getKnxEnabled() ? " checked" : "") + "></div>";
  
  // KNX Physical Address
  html += "<div><label>Physical Address:</label>";
  html += "<input type='number' name='knxPhysicalArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxPhysicalArea() : 1) + "'>.";
  html += "<input type='number' name='knxPhysicalLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxPhysicalLine() : 1) + "'>.";
  html += "<input type='number' name='knxPhysicalMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxPhysicalMember() : 1) + "'></div>";
  html += "</div>";
  
  // MQTT Settings
  html += "<div class='form-group' role='group' aria-labelledby='mqttSettings'>";
  html += "<h2 id='mqttSettings'>MQTT Settings</h2>";
  html += "<div><label for='mqttEnabled'>MQTT Enabled:</label>";
  html += "<input type='checkbox' id='mqttEnabled' name='mqttEnabled'" + 
          String(configManager && configManager->getMqttEnabled() ? " checked" : "") + "></div>";
  
  html += "<div><label for='mqttServer'>MQTT Server:</label>";
  html += "<input type='text' id='mqttServer' name='mqttServer' value='" + 
          String(configManager ? configManager->getMqttServer() : "") + "'></div>";
  
  html += "<div><label for='mqttPort'>MQTT Port:</label>";
  html += "<input type='number' id='mqttPort' name='mqttPort' value='" + 
          String(configManager ? configManager->getMqttPort() : 1883) + "'></div>";
  
  html += "<div><label for='mqttUser'>MQTT Username:</label>";
  html += "<input type='text' id='mqttUser' name='mqttUser' value='" + 
          String(configManager ? configManager->getMqttUser() : "") + "'></div>";
  
  html += "<div><label for='mqttPassword'>MQTT Password:</label>";
  html += "<input type='password' id='mqttPassword' name='mqttPassword'></div>";
  
  html += "<div><label for='mqttClientId'>MQTT Client ID:</label>";
  html += "<input type='text' id='mqttClientId' name='mqttClientId' value='" + 
          String(configManager ? configManager->getMqttClientId() : "") + "'></div>";
  html += "</div>";
  
  // PID Settings
  html += "<div class='form-group' role='group' aria-labelledby='pidSettings'>";
  html += "<h2 id='pidSettings'>PID Settings</h2>";
  html += "<div><label for='kp'>Proportional (Kp):</label>";
  html += "<input type='number' id='kp' name='kp' step='0.1' required value='" + 
          String(configManager ? configManager->getKp() : 1.0) + "'></div>";
  
  html += "<div><label for='ki'>Integral (Ki):</label>";
  html += "<input type='number' id='ki' name='ki' step='0.01' required value='" + 
          String(configManager ? configManager->getKi() : 0.1) + "'></div>";
  
  html += "<div><label for='kd'>Derivative (Kd):</label>";
  html += "<input type='number' id='kd' name='kd' step='0.001' required value='" + 
          String(configManager ? configManager->getKd() : 0.01) + "'></div>";
  
  html += "<div><label for='setpoint'>Setpoint (°C):</label>";
  html += "<input type='number' id='setpoint' name='setpoint' step='0.5' required min='5' max='30' value='" + 
          String(configManager ? configManager->getSetpoint() : 21.0) + "'></div>";
  html += "</div>";
  
  // Current Status
  html += "<div class='form-group' role='group' aria-labelledby='currentStatus'>";
  html += "<h2 id='currentStatus'>Current Status</h2>";
  if (thermostatState && sensorInterface) {
    html += "<p>Temperature: <span class='status-value'>" + String(sensorInterface->getTemperature()) + " °C</span></p>";
    html += "<p>Humidity: <span class='status-value'>" + String(sensorInterface->getHumidity()) + " %</span></p>";
    html += "<p>Target Temperature: <span class='status-value'>" + String(thermostatState->getTargetTemperature()) + " °C</span></p>";
    html += "<p>Heating: <span class='status-value'>" + String(thermostatState->isHeating() ? "Active" : "Inactive") + "</span></p>";
  } else {
    html += "<p>Status information not available</p>";
  }
  html += "<p>IP Address: <span class='status-value'>" + WiFi.localIP().toString() + "</span></p>";
  html += "<p>Signal Strength: <span class='status-value'>" + String(WiFi.RSSI()) + " dBm</span></p>";
  if (mqttInterface) {
    html += "<p>MQTT Status: <span class='status-value'>" + String(mqttInterface->isConnected() ? "Connected" : "Disconnected") + "</span></p>";
  }
  html += "</div>";
  
  // Submit button
  html += "<button type='submit' class='btn'>Save Configuration</button>";
  html += "</form>";
  
  // Additional action buttons with POST forms
  html += "<div style='margin-top: 20px;'>";
  html += "<form action='/reboot' method='post' style='display: inline-block;'>";
  html += "<button type='submit' class='btn'>Reboot Device</button>";
  html += "</form>";
  
  html += "<form action='/factory_reset' method='post' style='display: inline-block; margin-left: 10px;'>";
  html += "<button type='submit' class='btn btn-danger' onclick='return confirm(\"Are you sure? This will reset all settings!\");'>";
  html += "Factory Reset</button>";
  html += "</form>";
  html += "</div>";
  
  html += "</div>";
  
  // Add simple JavaScript for form validation
  html += "<script>";
  html += "document.getElementById('configForm').onsubmit = function(e) {";
  html += "  var sendInterval = document.getElementById('sendInterval').value;";
  html += "  var pidInterval = document.getElementById('pidInterval').value;";
  html += "  if (sendInterval < 1000 || pidInterval < 1000) {";
  html += "    alert('Intervals must be at least 1000ms');";
  html += "    e.preventDefault();";
  html += "    return false;";
  html += "  }";
  html += "  return true;";
  html += "};";
  html += "</script>";
  
  html += "</body></html>";
  
  return html;
}