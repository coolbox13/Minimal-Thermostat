#ifndef SKIP_WEB_INTERFACE_EXTRA
// implementation here
#endif

#include "web/web_interface.h"
#include <ESPmDNS.h>

String WebInterface::generateHtml() {
    return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 KNX Thermostat</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .card {
            background-color: white;
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .button {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 15px 32px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 4px;
        }
        .input {
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }
    </style>
</head>
<body>
    <div class="card">
        <h2>Current Status</h2>
        <div id="status">Loading...</div>
        <button onclick="updateStatus()">Refresh</button>
    </div>

    <div class="card">
        <h2>Settings</h2>
        <form id="settingsForm">
            <label for="deviceName">Device Name:</label><br>
            <input type="text" id="deviceName" name="deviceName" class="input"><br>
            <label for="sendInterval">Send Interval (ms):</label><br>
            <input type="number" id="sendInterval" name="sendInterval" class="input"><br>
            <label for="pidInterval">PID Interval (ms):</label><br>
            <input type="number" id="pidInterval" name="pidInterval" class="input"><br>
            <label for="knxAddress">KNX Physical Address:</label><br>
            <input type="text" id="knxAddress" name="knxAddress" class="input" pattern="\d{1,2}\.\d{1,2}\.\d{1,3}" title="Format: x.x.x (e.g., 1.1.1)"><br>
            <input type="submit" value="Save" class="button">
        </form>
    </div>

    <div class="card">
        <h2>System</h2>
        <button onclick="reboot()" class="button">Reboot</button>
        <button onclick="factoryReset()" class="button">Factory Reset</button>
    </div>

    <script>
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    let statusHtml = `
                        <p>Temperature: ${data.temperature}°C</p>
                        <p>Humidity: ${data.humidity}%</p>
                        <p>Pressure: ${data.pressure} hPa</p>
                        <p>Setpoint: ${data.setpoint}°C</p>
                        <p>Mode: ${data.mode}</p>
                        <p>Valve Position: ${data.valve}%</p>
                        <p>Heating: ${data.heating ? 'On' : 'Off'}</p>
                    `;
                    if (data.error) {
                        statusHtml += `<p>Error: ${data.error}</p>`;
                    }
                    document.getElementById('status').innerHTML = statusHtml;
                })
                .catch(error => {
                    console.error('Error:', error);
                    document.getElementById('status').innerHTML = 'Error loading status';
                });
        }

        document.getElementById('settingsForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const formData = new FormData(this);
            const data = {};
            for (let [key, value] of formData.entries()) {
                data[key] = value;
            }

            fetch('/save', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'X-CSRF-Token': getCsrfToken()
                },
                body: JSON.stringify(data)
            })
            .then(response => {
                if (response.ok) {
                    alert('Settings saved successfully');
                } else {
                    throw new Error('Failed to save settings');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                alert('Error saving settings');
            });
        });

        function reboot() {
            if (confirm('Are you sure you want to reboot the device?')) {
                fetch('/reboot', {
                    method: 'POST',
                    headers: {
                        'X-CSRF-Token': getCsrfToken()
                    }
                })
                .then(response => {
                    if (response.ok) {
                        alert('Device is rebooting...');
                        setTimeout(() => {
                            window.location.reload();
                        }, 5000);
                    } else {
                        throw new Error('Failed to reboot device');
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    alert('Error rebooting device');
                });
            }
        }

        function factoryReset() {
            if (confirm('Are you sure you want to reset the device to factory settings? This will erase all configuration!')) {
                fetch('/reset', {
                    method: 'POST',
                    headers: {
                        'X-CSRF-Token': getCsrfToken()
                    }
                })
                .then(response => {
                    if (response.ok) {
                        alert('Device is resetting to factory settings and will reboot...');
                        setTimeout(() => {
                            window.location.reload();
                        }, 5000);
                    } else {
                        throw new Error('Failed to reset device');
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    alert('Error resetting device');
                });
            }
        }

        function getCsrfToken() {
            return document.cookie.replace(/(?:(?:^|.*;\s*)XSRF-TOKEN\s*=\s*([^;]*).*$)|^.*$/, '$1');
        }

        updateStatus();
        setInterval(updateStatus, 10000);
    </script>
</body>
</html>
)rawliteral";
}