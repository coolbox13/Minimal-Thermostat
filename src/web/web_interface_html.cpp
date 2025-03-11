#include <Arduino.h>
#include "web/web_interface.h"
#include <ESPmDNS.h>

String WebInterface::generateHtml() {
    return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>ESP32 KNX Thermostat</title>
    <link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css'>
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
        .button.danger {
            background-color: #dc3545;
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
        .form-group {
            margin-bottom: 1rem;
        }
        .form-label {
            display: block;
            margin-bottom: 0.5rem;
        }
        .alert {
            padding: 15px;
            margin-bottom: 20px;
            border: 1px solid transparent;
            border-radius: 4px;
        }
        .alert-danger {
            color: #721c24;
            background-color: #f8d7da;
            border-color: #f5c6cb;
        }
        .form-select {
            width: 100%;
            padding: 0.375rem 2.25rem 0.375rem 0.75rem;
            font-size: 1rem;
            font-weight: 400;
            line-height: 1.5;
            color: #212529;
            background-color: #fff;
            border: 1px solid #ced4da;
            border-radius: 0.25rem;
            appearance: none;
        }
        .section { margin-bottom: 30px; }
        .navbar { margin-bottom: 20px; }
    </style>
</head>
<body>
    <nav class="navbar navbar-expand-lg navbar-dark bg-dark">
        <div class="container-fluid">
            <a class="navbar-brand" href="#">ESP32 KNX Thermostat</a>
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav">
                <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav">
                    <li class="nav-item">
                        <a class="nav-link" href="#status">Status</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="#control">Control</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="#config">Configuration</a>
                    </li>
                </ul>
            </div>
        </div>
    </nav>

    <div class='container'>
        <div id='status' class='section'>
            <h2>Status</h2>
            <div class='row'>
                <div class='col-md-4'>
                    <div class='card'>
                        <div class='card-body'>
                            <h5 class='card-title'>Temperature</h5>
                            <p class="temperature">--</p>
                            <h5 class='card-title'>Humidity</h5>
                            <p class="humidity">--</p>
                            <h5 class='card-title'>Pressure</h5>
                            <p class="pressure">--</p>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div id='control' class='section'>
            <h2>Control</h2>
            <div class='card'>
                <div class='card-body'>
                    <div class='row'>
                        <div class='col-md-6'>
                            <label class='form-label'>Mode</label>
                            <select class='form-select' id='mode'>
                                <option value='off' selected>Off</option>
                                <option value='comfort'>Comfort</option>
                                <option value='eco'>Eco</option>
                            </select>
                            <button onclick="setMode()" class="button">Set Mode</button>
                        </div>
                        <div class='col-md-6'>
                            <label class='form-label'>Setpoint</label>
                            <input type='number' class='form-control' id='setpoint' value='21.0' min='5' max='30' step='0.5'>
                            <button onclick="setSetpoint()" class="button">Set Temperature</button>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div id='config' class='section'>
            <h2>Configuration</h2>
            <div class='card'>
                <div class='card-body'>
                    <form id='configForm'>
                        <div class='mb-3'>
                            <label class='form-label'>Device Name</label>
                            <input type='text' class='form-control' name='deviceName' value='ESP32 Thermostat'>
                        </div>
                        <div class='mb-3'>
                            <label class='form-label'>Update Interval (ms)</label>
                            <input type='number' class='form-control' name='updateInterval' value='10000' min='1000' step='1000'>
                        </div>
                        <div class='mb-3'>
                            <label class='form-label'>KNX Address</label>
                            <input type='text' class='form-control' name='knxAddress' pattern='\d{1,2}\.\d{1,2}\.\d{1,3}' value='1.1.1'>
                        </div>
                        <button type="button" onclick="saveConfig()" class="button">Save Configuration</button>
                    </form>
                </div>
            </div>
        </div>

        <div id='system' class='section'>
            <h2>System</h2>
            <div class='card'>
                <div class='card-body'>
                    <button onclick="factoryReset()" class="button danger">Factory Reset</button>
                    <button onclick="location.reload()" class="button">Refresh Page</button>
                </div>
            </div>
        </div>
    </div>

    <script src='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js'></script>
    <script>
    async function setSetpoint() {
        const value = document.getElementById('setpoint').value;
        await fetch('/setpoint', {
            method: 'POST',
            body: `value=${value}`
        });
        location.reload();
    }

    async function setMode() {
        const mode = document.getElementById('mode').value;
        await fetch('/mode', {
            method: 'POST',
            body: `mode=${mode}`
        });
        location.reload();
    }

    async function saveConfig() {
        const form = document.getElementById('configForm');
        const data = new FormData(form);
        const json = Object.fromEntries(data.entries());
        
        await fetch('/save', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(json)
        });
        location.reload();
    }

    async function factoryReset() {
        if (confirm('Are you sure you want to reset to factory defaults?')) {
            await fetch('/reset', { method: 'POST' });
            location.reload();
        }
    }

    // Auto-refresh status every 10 seconds
    function updateStatus() {
        fetch('/status')
            .then(response => response.json())
            .then(data => {
                // Update status values
                document.querySelector('#status .temperature').textContent = data.temperature.toFixed(1) + '°C';
                document.querySelector('#status .humidity').textContent = data.humidity.toFixed(1) + '%';
                document.querySelector('#status .pressure').textContent = data.pressure.toFixed(1) + ' hPa';
            })
            .catch(error => {
                console.error('Error fetching status:', error);
            });
    }

    // Initial status update
    updateStatus();
    
    // Set interval for updates
    setInterval(updateStatus, 10000);
    </script>
</body>
</html>
)rawliteral";
}