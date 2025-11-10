// JavaScript for System Status Dashboard

document.addEventListener('DOMContentLoaded', function() {
    // Load status on page load
    loadSystemStatus();

    // Refresh button
    document.getElementById('refresh-status').addEventListener('click', loadSystemStatus);

    // Auto-refresh every 10 seconds
    setInterval(loadSystemStatus, 10000);
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
        const sensorStatus = document.getElementById('sensor-status');
        if (data.sensor.healthy) {
            document.getElementById('sensor-health').textContent = 'Healthy';
            sensorStatus.className = 'status-item good';
        } else {
            document.getElementById('sensor-health').textContent = 'Error';
            sensorStatus.className = 'status-item error';
        }

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
