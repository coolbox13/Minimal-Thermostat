#include "web_interface.h"

String WebInterface::generateHtml() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>KNX Thermostat Configuration</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; }";
  html += "label { display: inline-block; width: 120px; margin-bottom: 10px; }";
  html += "input[type='number'] { width: 60px; }";
  html += ".form-group { margin-bottom: 15px; border-bottom: 1px solid #ccc; padding-bottom: 15px; }";
  html += "h2 { color: #333; }";
  html += "button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; }";
  html += "button:hover { background-color: #45a049; }";
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
  
  // KNX Physical Address
  html += "<div class='form-group'>";
  html += "<h2>KNX Physical Address</h2>";
  html += "<label for='knxEnabled'>Enabled:</label>";
  html += "<input type='checkbox' id='knxEnabled' name='knxEnabled' " + 
          String(configManager && configManager->getKnxEnabled() ? "checked" : "") + "><br>";
          
  html += "<label for='knxPhysicalArea'>Area:</label>";
  html += "<input type='number' id='knxPhysicalArea' name='knxPhysicalArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxPhysicalArea() : 1) + "'>";
  
  html += "<label for='knxPhysicalLine'>Line:</label>";
  html += "<input type='number' id='knxPhysicalLine' name='knxPhysicalLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxPhysicalLine() : 1) + "'>";
  
  html += "<label for='knxPhysicalMember'>Member:</label>";
  html += "<input type='number' id='knxPhysicalMember' name='knxPhysicalMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxPhysicalMember() : 201) + "'><br>";
  html += "</div>";
  
  // Temperature Group Address
  html += "<div class='form-group'>";
  html += "<h2>Temperature Group Address</h2>";
  html += "<label for='knxTempArea'>Area:</label>";
  html += "<input type='number' id='knxTempArea' name='knxTempArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxTempArea() : 3) + "'>";
  
  html += "<label for='knxTempLine'>Line:</label>";
  html += "<input type='number' id='knxTempLine' name='knxTempLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxTempLine() : 1) + "'>";
  
  html += "<label for='knxTempMember'>Member:</label>";
  html += "<input type='number' id='knxTempMember' name='knxTempMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxTempMember() : 0) + "'><br>";
  html += "</div>";
  
  // Setpoint Group Address
  html += "<div class='form-group'>";
  html += "<h2>Setpoint Group Address</h2>";
  html += "<label for='knxSetpointArea'>Area:</label>";
  html += "<input type='number' id='knxSetpointArea' name='knxSetpointArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxSetpointArea() : 3) + "'>";
  
  html += "<label for='knxSetpointLine'>Line:</label>";
  html += "<input type='number' id='knxSetpointLine' name='knxSetpointLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxSetpointLine() : 2) + "'>";
  
  html += "<label for='knxSetpointMember'>Member:</label>";
  html += "<input type='number' id='knxSetpointMember' name='knxSetpointMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxSetpointMember() : 0) + "'><br>";
  html += "</div>";
  
  // Valve Group Address
  html += "<div class='form-group'>";
  html += "<h2>Valve Group Address</h2>";
  html += "<label for='knxValveArea'>Area:</label>";
  html += "<input type='number' id='knxValveArea' name='knxValveArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxValveArea() : 3) + "'>";
  
  html += "<label for='knxValveLine'>Line:</label>";
  html += "<input type='number' id='knxValveLine' name='knxValveLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxValveLine() : 3) + "'>";
  
  html += "<label for='knxValveMember'>Member:</label>";
  html += "<input type='number' id='knxValveMember' name='knxValveMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxValveMember() : 0) + "'><br>";
  html += "</div>";
  
  // Mode Group Address
  html += "<div class='form-group'>";
  html += "<h2>Mode Group Address</h2>";
  html += "<label for='knxModeArea'>Area:</label>";
  html += "<input type='number' id='knxModeArea' name='knxModeArea' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxModeArea() : 3) + "'>";
  
  html += "<label for='knxModeLine'>Line:</label>";
  html += "<input type='number' id='knxModeLine' name='knxModeLine' min='0' max='15' value='" + 
          String(configManager ? configManager->getKnxModeLine() : 4) + "'>";
  
  html += "<label for='knxModeMember'>Member:</label>";
  html += "<input type='number' id='knxModeMember' name='knxModeMember' min='0' max='255' value='" + 
          String(configManager ? configManager->getKnxModeMember() : 0) + "'><br>";
  html += "</div>";
  
  // MQTT Settings
  html += "<div class='form-group'>";
  html += "<h2>MQTT Settings</h2>";
  html += "<label for='mqttEnabled'>Enabled:</label>";
  html += "<input type='checkbox' id='mqttEnabled' name='mqttEnabled' " + 
          String(configManager && configManager->getMqttEnabled() ? "checked" : "") + "><br>";
  
  html += "<label for='mqttServer'>MQTT Server:</label>";
  html += "<input type='text' id='mqttServer' name='mqttServer' value='" + 
          String(configManager ? configManager->getMqttServer()