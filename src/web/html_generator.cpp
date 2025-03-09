#include "web/html_generator.h"

String HtmlGenerator::generatePage(
    ThermostatState* state,
    ConfigInterface* config,
    ControlInterface* control) {
    String html = generateHeader();
    html += generateNavigation();
    html += "<div class='container'>";
    html += generateStatusSection(state);
    html += generateControlSection(state);
    html += generateConfigSection(config);
    if (control) {
        html += generatePIDSection(control);
    }
    html += "</div>";
    html += generateFooter();
    return html;
}

String HtmlGenerator::generateHeader() {
    return R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>ESP32 KNX Thermostat</title>
    <link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css'>
)" + generateStyles() + R"(
</head>
<body>
)";
}

String HtmlGenerator::generateNavigation() {
    return R"(
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
)";
}

String HtmlGenerator::generateStatusSection(ThermostatState* state) {
    if (!state) return "";

    char buf[256];
    snprintf(buf, sizeof(buf), R"(
<div id='status' class='section'>
    <h2>Status</h2>
    <div class='row'>
        <div class='col-md-4'>
            <div class='card'>
                <div class='card-body'>
                    <h5 class='card-title'>Temperature</h5>
                    <p class='card-text'>%.1f°C</p>
                </div>
            </div>
        </div>
        <div class='col-md-4'>
            <div class='card'>
                <div class='card-body'>
                    <h5 class='card-title'>Humidity</h5>
                    <p class='card-text'>%.1f%%</p>
                </div>
            </div>
        </div>
        <div class='col-md-4'>
            <div class='card'>
                <div class='card-body'>
                    <h5 class='card-title'>Pressure</h5>
                    <p class='card-text'>%.1f hPa</p>
                </div>
            </div>
        </div>
    </div>
</div>
)", 
        state->getCurrentTemperature(),
        state->getCurrentHumidity(),
        state->getCurrentPressure()
    );
    return String(buf);
}

String HtmlGenerator::generateControlSection(ThermostatState* state) {
    if (!state) return "";

    char buf[512];
    snprintf(buf, sizeof(buf), R"(
<div id='control' class='section'>
    <h2>Control</h2>
    <div class='row'>
        <div class='col-md-6'>
            <div class='card'>
                <div class='card-body'>
                    <h5 class='card-title'>Setpoint</h5>
                    <div class='input-group'>
                        <input type='number' class='form-control' id='setpoint' value='%.1f' step='0.5'>
                        <button class='btn btn-primary' onclick='setSetpoint()'>Set</button>
                    </div>
                </div>
            </div>
        </div>
        <div class='col-md-6'>
            <div class='card'>
                <div class='card-body'>
                    <h5 class='card-title'>Mode</h5>
                    <select class='form-select' id='mode' onchange='setMode()'>
                        <option value='off' %s>Off</option>
                        <option value='comfort' %s>Comfort</option>
                        <option value='eco' %s>Eco</option>
                        <option value='away' %s>Away</option>
                        <option value='boost' %s>Boost</option>
                        <option value='antifreeze' %s>Anti-freeze</option>
                    </select>
                </div>
            </div>
        </div>
    </div>
</div>
)",
        state->getTargetTemperature(),
        state->getMode() == MODE_OFF ? "selected" : "",
        state->getMode() == MODE_COMFORT ? "selected" : "",
        state->getMode() == MODE_ECO ? "selected" : "",
        state->getMode() == MODE_AWAY ? "selected" : "",
        state->getMode() == MODE_BOOST ? "selected" : "",
        state->getMode() == MODE_ANTIFREEZE ? "selected" : ""
    );
    return String(buf);
}

String HtmlGenerator::generateConfigSection(ConfigInterface* config) {
    if (!config) return "";

    char buf[512];
    snprintf(buf, sizeof(buf), R"(
<div id='config' class='section'>
    <h2>Configuration</h2>
    <div class='card'>
        <div class='card-body'>
            <form id='configForm'>
                <div class='mb-3'>
                    <label class='form-label'>Device Name</label>
                    <input type='text' class='form-control' name='deviceName' value='%s'>
                </div>
                <div class='mb-3'>
                    <label class='form-label'>Update Interval (ms)</label>
                    <input type='number' class='form-control' name='interval' value='%lu'>
                </div>
                <div class='mb-3'>
                    <label class='form-label'>Web Username</label>
                    <input type='text' class='form-control' name='username' value='%s'>
                </div>
                <div class='mb-3'>
                    <label class='form-label'>Web Password</label>
                    <input type='password' class='form-control' name='password'>
                </div>
                <button type='button' class='btn btn-primary' onclick='saveConfig()'>Save</button>
                <button type='button' class='btn btn-danger' onclick='factoryReset()'>Factory Reset</button>
            </form>
        </div>
    </div>
</div>
)",
        config->getDeviceName(),
        config->getSendInterval(),
        config->getWebUsername()
    );
    return String(buf);
}

String HtmlGenerator::generatePIDSection(ControlInterface* control) {
    if (!control) return "";

    char buf[512];
    snprintf(buf, sizeof(buf), R"(
<div id='pid' class='section'>
    <h2>PID Control</h2>
    <div class='card'>
        <div class='card-body'>
            <div class='row'>
                <div class='col-md-3'>
                    <label class='form-label'>Kp</label>
                    <input type='number' class='form-control' id='kp' value='%.2f' step='0.1'>
                </div>
                <div class='col-md-3'>
                    <label class='form-label'>Ki</label>
                    <input type='number' class='form-control' id='ki' value='%.2f' step='0.01'>
                </div>
                <div class='col-md-3'>
                    <label class='form-label'>Kd</label>
                    <input type='number' class='form-control' id='kd' value='%.2f' step='0.01'>
                </div>
                <div class='col-md-3'>
                    <label class='form-label'>Output</label>
                    <input type='number' class='form-control' value='%.1f' readonly>
                </div>
            </div>
            <div class='form-check mt-3'>
                <input class='form-check-input' type='checkbox' id='pidActive' %s>
                <label class='form-check-label'>PID Active</label>
            </div>
            <button class='btn btn-primary mt-3' onclick='updatePID()'>Update</button>
        </div>
    </div>
</div>
)",
        control->getKp(),
        control->getKi(),
        control->getKd(),
        control->getOutput(),
        control->isActive() ? "checked" : ""
    );
    return String(buf);
}

String HtmlGenerator::generateFooter() {
    return R"(
    <script src='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js'></script>
)" + generateScripts() + R"(
</body>
</html>
)";
}

String HtmlGenerator::generateStyles() {
    return R"(
<style>
    body { padding-top: 20px; }
    .section { margin-bottom: 30px; }
    .card { margin-bottom: 20px; }
    .navbar { margin-bottom: 20px; }
</style>
)";
}

String HtmlGenerator::generateScripts() {
    return R"(
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

async function updatePID() {
    const data = {
        kp: document.getElementById('kp').value,
        ki: document.getElementById('ki').value,
        kd: document.getElementById('kd').value,
        active: document.getElementById('pidActive').checked
    };
    
    await fetch('/pid', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
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
setInterval(() => {
    fetch('/status')
        .then(response => response.json())
        .then(data => {
            // Update status values
            document.querySelector('#status .temperature').textContent = data.temperature.toFixed(1) + '°C';
            document.querySelector('#status .humidity').textContent = data.humidity.toFixed(1) + '%';
            document.querySelector('#status .pressure').textContent = data.pressure.toFixed(1) + ' hPa';
        });
}, 10000);
</script>
)";
} 