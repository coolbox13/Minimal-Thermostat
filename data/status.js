// JavaScript for System Status Dashboard

document.addEventListener('DOMContentLoaded', function() {
    // Load status on page load
    loadSystemStatus();
    loadHealthStatus();

    // Refresh button
    document.getElementById('refresh-status').addEventListener('click', function() {
        loadSystemStatus();
        loadHealthStatus();
    });

    // Auto-refresh every 10 seconds
    setInterval(function() {
        loadSystemStatus();
        loadHealthStatus();
    }, 10000);
});

function loadSystemStatus() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            updateSystemStatus(data);

            // Update last update timestamp
            const now = new Date();
            document.getElementById('last-update').textContent =
                `Last updated: ${now.toLocaleTimeString()}`;
        })
        .catch(error => {
            console.error('Error loading system status:', error);
        });
}

function updateSystemStatus(data) {
    // System Information
    if (data.system) {
        // Uptime
        const uptimeSeconds = data.system.uptime;
        const uptimeStr = formatUptime(uptimeSeconds);
        document.getElementById('uptime-value').textContent = uptimeStr;
        document.getElementById('uptime-subtitle').textContent =
            `${uptimeSeconds.toLocaleString()} seconds`;

        // Memory
        const freeHeapKB = Math.round(data.system.free_heap / 1024);
        const totalHeapKB = Math.round(data.system.total_heap / 1024);
        const memoryPercent = Math.round((data.system.free_heap / data.system.total_heap) * 100);

        document.getElementById('memory-value').textContent = freeHeapKB.toLocaleString();
        document.getElementById('memory-subtitle').textContent =
            `${memoryPercent}% free (${totalHeapKB.toLocaleString()} KB total)`;

        // Color code memory
        const memoryStatus = document.getElementById('memory-status');
        if (memoryPercent > 50) {
            memoryStatus.className = 'status-item good';
        } else if (memoryPercent > 25) {
            memoryStatus.className = 'status-item warning';
        } else {
            memoryStatus.className = 'status-item error';
        }

        // Flash memory - use pre-calculated values from backend
        document.getElementById('flash-value').textContent = data.system.free_flash_kb.toLocaleString();
        document.getElementById('flash-subtitle').textContent =
            `${data.system.flash_percent_free}% free (${data.system.used_flash_kb.toLocaleString()} KB used / ${data.system.ota_partition_kb.toLocaleString()} KB partition)`;

        // Color code flash (stricter thresholds for OTA partition)
        const flashStatus = document.getElementById('flash-status');
        if (data.system.flash_percent_free > 15) {
            flashStatus.className = 'status-item good';
        } else if (data.system.flash_percent_free > 5) {
            flashStatus.className = 'status-item warning';
        } else {
            flashStatus.className = 'status-item error';
        }

        // Chip info
        document.getElementById('chip-model').textContent = data.system.chip_model;
        document.getElementById('chip-details').textContent =
            `Revision ${data.system.chip_revision} @ ${data.system.cpu_freq_mhz} MHz`;
    }

    // WiFi Information
    if (data.wifi) {
        const wifiStatus = document.getElementById('wifi-status');
        if (data.wifi.connected) {
            document.getElementById('wifi-connected').textContent = 'Connected';
            document.getElementById('wifi-ssid').textContent = `Network: ${data.wifi.ssid}`;
            wifiStatus.className = 'status-item good';

            // Signal strength
            const rssi = data.wifi.rssi;
            const quality = data.wifi.quality;
            document.getElementById('wifi-rssi').textContent = rssi;
            document.getElementById('wifi-quality').textContent =
                `Signal quality: ${quality}%`;

            const wifiSignalStatus = document.getElementById('wifi-signal-status');
            if (rssi >= -60) {
                wifiSignalStatus.className = 'status-item good';
            } else if (rssi >= -75) {
                wifiSignalStatus.className = 'status-item warning';
            } else {
                wifiSignalStatus.className = 'status-item error';
            }

            // IP and MAC
            document.getElementById('wifi-ip').textContent = data.wifi.ip;
            document.getElementById('wifi-mac').textContent = `MAC: ${data.wifi.mac}`;
        } else {
            document.getElementById('wifi-connected').textContent = 'Disconnected';
            document.getElementById('wifi-ssid').textContent = 'Not connected to any network';
            wifiStatus.className = 'status-item error';

            document.getElementById('wifi-rssi').textContent = '--';
            document.getElementById('wifi-quality').textContent = '--';
            document.getElementById('wifi-ip').textContent = '--';
            document.getElementById('wifi-mac').textContent = '--';
        }
    }

    // Sensor Information
    if (data.sensor) {
        document.getElementById('sensor-temp').textContent = data.sensor.temperature.toFixed(1);
        document.getElementById('sensor-humidity').textContent = data.sensor.humidity.toFixed(1);
        document.getElementById('sensor-pressure').textContent = data.sensor.pressure.toFixed(1);
    }

    // PID Controller
    if (data.pid) {
        document.getElementById('pid-setpoint').textContent = data.pid.setpoint.toFixed(1);
        document.getElementById('pid-valve').textContent = Math.round(data.pid.valve_position);
        document.getElementById('pid-params').textContent =
            `Kp: ${data.pid.kp.toFixed(2)}, Ki: ${data.pid.ki.toFixed(3)}, Kd: ${data.pid.kd.toFixed(3)}\nDeadband: ${data.pid.deadband.toFixed(1)}°C`;
    }

    // Diagnostics
    if (data.diagnostics) {
        const rebootStatus = document.getElementById('reboot-status');
        const reason = data.diagnostics.last_reboot_reason || 'Unknown';
        document.getElementById('reboot-reason').textContent = reason;
        document.getElementById('reboot-count').textContent =
            `Total reboots: ${data.diagnostics.reboot_count}`;

        // Color code based on reboot reason
        if (reason.toLowerCase().includes('watchdog') ||
            reason.toLowerCase().includes('crash') ||
            reason.toLowerCase().includes('panic')) {
            rebootStatus.className = 'status-item error';
        } else if (reason.toLowerCase().includes('update') ||
                   reason.toLowerCase().includes('user')) {
            rebootStatus.className = 'status-item good';
        } else {
            rebootStatus.className = 'status-item warning';
        }

        // Watchdog reboots
        const watchdogCount = data.diagnostics.consecutive_watchdog_reboots;
        document.getElementById('watchdog-count').textContent = watchdogCount;

        const watchdogStatus = document.getElementById('watchdog-status');
        if (watchdogCount === 0) {
            watchdogStatus.className = 'status-item good';
        } else if (watchdogCount < 3) {
            watchdogStatus.className = 'status-item warning';
        } else {
            watchdogStatus.className = 'status-item error';
        }
    }

    // Configuration
    if (data.mqtt) {
        document.getElementById('mqtt-server').textContent = data.mqtt.server;
        document.getElementById('mqtt-port').textContent = `Port: ${data.mqtt.port}`;
    }

    if (data.knx) {
        document.getElementById('knx-address').textContent = data.knx.address;
    }
}

function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = Math.floor(seconds % 60);

    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m`;
    } else if (minutes > 0) {
        return `${minutes}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

// Health Monitoring Functions (Items #9 & #10)
function loadHealthStatus() {
    // Load sensor health
    fetch('/api/sensor-health')
        .then(response => response.json())
        .then(data => {
            updateSensorHealth(data);
        })
        .catch(error => {
            console.error('Error loading sensor health:', error);
        });

    // Load valve health
    fetch('/api/valve-health')
        .then(response => response.json())
        .then(data => {
            updateValveHealth(data);
        })
        .catch(error => {
            console.error('Error loading valve health:', error);
        });
}

function updateSensorHealth(data) {
    const statusElement = document.getElementById('sensor-health-status');
    const valueElement = document.getElementById('sensor-health-value');
    const detailsElement = document.getElementById('sensor-health-details');

    if (data.healthy) {
        statusElement.className = 'status-item good';
        valueElement.textContent = '✓ Healthy';
        detailsElement.textContent = `Failure rate: ${data.failure_rate.toFixed(1)}% | ${data.total_readings} readings`;
    } else {
        const consecutiveFailures = data.consecutive_failures;
        if (consecutiveFailures >= 10) {
            statusElement.className = 'status-item error';
            valueElement.textContent = '✗ CRITICAL';
            detailsElement.textContent = `${consecutiveFailures} consecutive failures!`;
        } else {
            statusElement.className = 'status-item warning';
            valueElement.textContent = '⚠ Warning';
            detailsElement.textContent = `${consecutiveFailures} consecutive failures`;
        }
    }

    // Add last good reading info if available
    if (data.last_good_value !== null && !isNaN(data.last_good_value)) {
        const timeSince = data.seconds_since_good_reading;
        if (timeSince < 60) {
            detailsElement.textContent += ` | Last: ${data.last_good_value.toFixed(1)}°C (${timeSince}s ago)`;
        } else {
            const minAgo = Math.floor(timeSince / 60);
            detailsElement.textContent += ` | Last: ${data.last_good_value.toFixed(1)}°C (${minAgo}m ago)`;
        }
    }
}

function updateValveHealth(data) {
    const statusElement = document.getElementById('valve-health-status');
    const valueElement = document.getElementById('valve-health-value');
    const detailsElement = document.getElementById('valve-health-details');

    if (data.healthy) {
        statusElement.className = 'status-item good';
        valueElement.textContent = '✓ Healthy';
        detailsElement.textContent = `Avg error: ${data.average_error.toFixed(1)}% | Max: ${data.max_error.toFixed(1)}%`;
    } else {
        const consecutiveStuck = data.consecutive_stuck;
        if (consecutiveStuck >= 5) {
            statusElement.className = 'status-item error';
            valueElement.textContent = '✗ STUCK';
            detailsElement.textContent = `Valve non-responsive! Error: ${data.last_error.toFixed(1)}%`;
        } else {
            statusElement.className = 'status-item warning';
            valueElement.textContent = '⚠ Warning';
            detailsElement.textContent = `Position error: ${data.last_error.toFixed(1)}% | Stuck events: ${data.stuck_count}`;
        }
    }

    // Add commanded vs actual if available
    if (data.last_commanded !== undefined) {
        detailsElement.textContent += ` | Cmd: ${data.last_commanded.toFixed(0)}%, Act: ${data.last_actual.toFixed(0)}%`;
    }
}
