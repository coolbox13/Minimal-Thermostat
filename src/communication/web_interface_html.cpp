#include "web/web_interface.h"

String WebInterface::generateHtml() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Thermostat</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        .card { background: #fff; border-radius: 5px; padding: 20px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; }
        input[type="text"], input[type="number"], input[type="password"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; }
        button:hover { background: #0056b3; }
        .status { font-size: 1.2em; margin-bottom: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <h2>Current Status</h2>
            <div class="status">
                Temperature: <span id="currentTemp">--</span>°C<br>
                Target: <span id="targetTemp">--</span>°C<br>
                Mode: <span id="mode">--</span><br>
                Output: <span id="output">--</span>%
            </div>
            <button onclick="updateStatus()">Refresh</button>
        </div>

        <div class="card">
            <h2>Device Settings</h2>
            <div class="form-group">
                <label>Device Name:</label>
                <input type="text" id="deviceName" value=")" + 
                String(configManager ? configManager->getDeviceName() : "") + R"(">
            </div>
            <div class="form-group">
                <label>Send Interval (ms):</label>
                <input type="number" id="sendInterval" value=")" + 
                String(configManager ? configManager->getSendInterval() : 10000) + R"(">
            </div>
            <div class="form-group">
                <label>PID Interval (ms):</label>
                <input type="number" id="pidInterval" value=")" + 
                String(configManager ? configManager->getPidInterval() : 10000) + R"(">
            </div>
        </div>

        <div class="card">
            <h2>KNX Settings</h2>
            <div class="form-group">
                <label>KNX Enabled:</label>
                <input type="checkbox" id="knxEnabled")" + 
                String(configManager && configManager->getKnxEnabled() ? " checked" : "") + R"(>
            </div>
            <div class="form-group">
                <label>Physical Address:</label>
                <input type="number" min="0" max="15" id="knxArea" value=")" + 
                String(configManager ? configManager->getKnxPhysicalArea() : 1) + R"(">.
                <input type="number" min="0" max="15" id="knxLine" value=")" + 
                String(configManager ? configManager->getKnxPhysicalLine() : 1) + R"(">.
                <input type="number" min="0" max="255" id="knxMember" value=")" + 
                String(configManager ? configManager->getKnxPhysicalMember() : 1) + R"(">
            </div>
        </div>

        <div class="card">
            <h2>MQTT Settings</h2>
            <div class="form-group">
                <label>MQTT Enabled:</label>
                <input type="checkbox" id="mqttEnabled")" + 
                String(configManager && configManager->getMQTTEnabled() ? " checked" : "") + R"(>
            </div>
            <div class="form-group">
                <label>MQTT Server:</label>
                <input type="text" id="mqttServer" value=")" + 
                String(configManager ? configManager->getMQTTServer() : "") + R"(">
            </div>
            <div class="form-group">
                <label>MQTT Port:</label>
                <input type="number" id="mqttPort" value=")" + 
                String(configManager ? configManager->getMQTTPort() : 1883) + R"(">
            </div>
            <div class="form-group">
                <label>MQTT User:</label>
                <input type="text" id="mqttUser" value=")" + 
                String(configManager ? configManager->getMQTTUser() : "") + R"(">
            </div>
            <div class="form-group">
                <label>MQTT Password:</label>
                <input type="password" id="mqttPassword">
            </div>
            <div class="form-group">
                <label>MQTT Client ID:</label>
                <input type="text" id="mqttClientId" value=")" + 
                String(configManager ? configManager->getMQTTClientId() : "") + R"(">
            </div>
        </div>

        <div class="card">
            <h2>PID Settings</h2>
            <div class="form-group">
                <label>Kp:</label>
                <input type="number" step="0.1" id="pidKp" value=")" + 
                String(pidController ? pidController->getKp() : 1.0) + R"(">
            </div>
            <div class="form-group">
                <label>Ki:</label>
                <input type="number" step="0.1" id="pidKi" value=")" + 
                String(pidController ? pidController->getKi() : 0.1) + R"(">
            </div>
            <div class="form-group">
                <label>Kd:</label>
                <input type="number" step="0.1" id="pidKd" value=")" + 
                String(pidController ? pidController->getKd() : 0.0) + R"(">
            </div>
        </div>

        <div class="card">
            <h2>Thermostat Settings</h2>
            <div class="form-group">
                <label>Setpoint:</label>
                <input type="number" step="0.5" id="setpoint" value=")" + 
                String(configManager ? configManager->getSetpoint() : 21.0) + R"(">
            </div>
        </div>

        <button onclick="saveSettings()">Save Settings</button>
    </div>

    <script>
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('currentTemp').textContent = data.currentTemp.toFixed(1);
                    document.getElementById('targetTemp').textContent = data.targetTemp.toFixed(1);
                    document.getElementById('mode').textContent = data.mode;
                    document.getElementById('output').textContent = data.output.toFixed(1);
                });
        }

        function saveSettings() {
            const data = {
                deviceName: document.getElementById('deviceName').value,
                sendInterval: parseInt(document.getElementById('sendInterval').value),
                pidInterval: parseInt(document.getElementById('pidInterval').value),
                knxEnabled: document.getElementById('knxEnabled').checked,
                knxArea: parseInt(document.getElementById('knxArea').value),
                knxLine: parseInt(document.getElementById('knxLine').value),
                knxMember: parseInt(document.getElementById('knxMember').value),
                mqttEnabled: document.getElementById('mqttEnabled').checked,
                mqttServer: document.getElementById('mqttServer').value,
                mqttPort: parseInt(document.getElementById('mqttPort').value),
                mqttUser: document.getElementById('mqttUser').value,
                mqttPassword: document.getElementById('mqttPassword').value,
                mqttClientId: document.getElementById('mqttClientId').value,
                pidKp: parseFloat(document.getElementById('pidKp').value),
                pidKi: parseFloat(document.getElementById('pidKi').value),
                pidKd: parseFloat(document.getElementById('pidKd').value),
                setpoint: parseFloat(document.getElementById('setpoint').value)
            };

            fetch('/save', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data)
            })
            .then(response => {
                if (response.ok) {
                    alert('Settings saved successfully');
                    updateStatus();
                } else {
                    alert('Failed to save settings');
                }
            });
        }

        // Update status on page load
        updateStatus();
        // Update status every 10 seconds
        setInterval(updateStatus, 10000);
    </script>
</body>
</html>
)";

    return html;
}