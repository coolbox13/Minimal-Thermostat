// JavaScript for the ESP32 KNX Thermostat configuration page

document.addEventListener('DOMContentLoaded', function() {
    // Tab switching
    document.querySelectorAll('.tab-button').forEach(button => {
        button.addEventListener('click', function() {
            // Remove active class from all buttons and tabs
            document.querySelectorAll('.tab-button').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            
            // Add active class to clicked button and corresponding tab
            this.classList.add('active');
            const tabId = this.getAttribute('data-tab');
            document.getElementById(tabId).classList.add('active');
            
            // If system tab is selected, update system status
            if (tabId === 'system') {
                updateSystemStatus();
            }
        });
    });
    
    // Load current configuration
    loadConfiguration();
    
    // Save configuration form submit handler
    document.getElementById('config-form').addEventListener('submit', saveConfiguration);
    
    // Reboot device button handler
    document.getElementById('reboot-device').addEventListener('click', rebootDevice);
    
    // Add input blur handlers to format values properly when user finishes editing
    document.getElementById('pid_kp').addEventListener('blur', function() {
        this.value = formatNumberWithPrecision(parseFloat(this.value), 2);
    });
    
    document.getElementById('pid_ki').addEventListener('blur', function() {
        this.value = formatNumberWithPrecision(parseFloat(this.value), 3);
    });
    
    document.getElementById('pid_kd').addEventListener('blur', function() {
        this.value = formatNumberWithPrecision(parseFloat(this.value), 3);
    });
    
    document.getElementById('setpoint').addEventListener('blur', function() {
        this.value = formatNumberWithPrecision(parseFloat(this.value), 1);
    });
    
    // Add event listeners for WiFi action buttons
    document.getElementById('btn-reconnect').addEventListener('click', reconnectWiFi);
    document.getElementById('btn-config-portal').addEventListener('click', startConfigPortal);
});

/**
 * Format a number with a specific number of decimal places,
 * ensuring consistent display of trailing zeros.
 * 
 * @param {number} value - The number to format
 * @param {number} decimals - Number of decimal places to display
 * @returns {string} Formatted number with specified decimal places
 */
function formatNumberWithPrecision(value, decimals) {
    if (isNaN(value)) return '';
    
    // Use toFixed to get the right number of decimal places with trailing zeros
    return value.toFixed(decimals);
}

// Function to load the current configuration from the API
function loadConfiguration() {
    const statusElement = document.getElementById('config-status');
    
    fetch('/api/config')
        .then(response => response.json())
        .then(data => {
            // Network settings
            if (data.network) {
                document.getElementById('wifi_ssid').value = data.network.wifi_ssid || '';
                // Don't set password for security reasons
            }
            
            // MQTT settings
            if (data.mqtt) {
                document.getElementById('mqtt_server').value = data.mqtt.server || '';
                document.getElementById('mqtt_port').value = data.mqtt.port || 1883;
            }
            
            // KNX settings
            if (data.knx) {
                document.getElementById('knx_area').value = data.knx.area || 1;
                document.getElementById('knx_line').value = data.knx.line || 1;
                document.getElementById('knx_member').value = data.knx.member || 1;
                document.getElementById('knx_test').checked = data.knx.use_test || false;
            }
            
            // BME280 settings - these are served from the API but not stored in ConfigManager yet
            if (data.bme280) {
                document.getElementById('bme280_address').value = data.bme280.address || '0x76';
                document.getElementById('bme280_sda').value = data.bme280.sda_pin || 21;
                document.getElementById('bme280_scl').value = data.bme280.scl_pin || 22;
                document.getElementById('bme280_interval').value = data.bme280.interval || 30;
            }
            
            // PID settings - Apply proper formatting for floating point values
            if (data.pid) {
                // Format each value with the appropriate precision
                document.getElementById('pid_kp').value = formatNumberWithPrecision(data.pid.kp || 2.0, 2);
                document.getElementById('pid_ki').value = formatNumberWithPrecision(data.pid.ki || 0.1, 3);
                document.getElementById('pid_kd').value = formatNumberWithPrecision(data.pid.kd || 0.5, 3);
                document.getElementById('setpoint').value = formatNumberWithPrecision(data.pid.setpoint || 22.0, 1);
            }
        })
        .catch(error => {
            console.error('Error loading configuration:', error);
            if (statusElement) {
                statusElement.textContent = 'Error loading configuration';
                statusElement.className = 'status status-error';
            }
        });
}

// Function to save the configuration
function saveConfiguration(e) {
    e.preventDefault();
    
    // Show loading status
    const statusElement = document.getElementById('config-status');
    statusElement.textContent = 'Saving configuration...';
    statusElement.className = 'status status-loading';
    
    // Get form data
    const formData = new FormData(document.getElementById('config-form'));
    
    // Build configuration object
    const config = {
        network: {
            wifi_ssid: formData.get('wifi_ssid'),
            wifi_pass: formData.get('wifi_pass')
        },
        mqtt: {
            server: formData.get('mqtt_server'),
            port: parseInt(formData.get('mqtt_port'))
        },
        knx: {
            area: parseInt(formData.get('knx_area')),
            line: parseInt(formData.get('knx_line')),
            member: parseInt(formData.get('knx_member')),
            use_test: formData.get('knx_test') === 'on'
        },
        bme280: {
            address: formData.get('bme280_address'),
            sda_pin: parseInt(formData.get('bme280_sda')),
            scl_pin: parseInt(formData.get('bme280_scl')),
            interval: parseInt(formData.get('bme280_interval'))
        },
        pid: {
            // Format PID values with proper precision before sending
            kp: parseFloat(formatNumberWithPrecision(parseFloat(formData.get('pid_kp')), 2)),
            ki: parseFloat(formatNumberWithPrecision(parseFloat(formData.get('pid_ki')), 3)),
            kd: parseFloat(formatNumberWithPrecision(parseFloat(formData.get('pid_kd')), 3)),
            setpoint: parseFloat(formatNumberWithPrecision(parseFloat(formData.get('setpoint')), 1))
        }
    };
    
    // Reset any previous validation errors
    document.querySelectorAll('.error-highlight').forEach(el => {
        el.classList.remove('error-highlight');
    });
    
    // Send to server
    fetch('/api/config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(config)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            // Show success message
            statusElement.textContent = 'Configuration saved successfully';
            statusElement.className = 'status status-success';
            
            // Reload the configuration after a short delay
            setTimeout(() => {
                loadConfiguration();
            }, 1000);
        } else {
            // Show error message
            statusElement.textContent = 'Error saving configuration: ' + data.message;
            statusElement.className = 'status status-error';
            
            // Highlight the field with the error if we can identify it
            highlightErrorField(data.message);
        }
    })
    .catch(error => {
        console.error('Error saving configuration:', error);
        statusElement.textContent = 'Error connecting to server. Check your network connection.';
        statusElement.className = 'status status-error';
    });
}

// Function to highlight fields with errors
function highlightErrorField(errorMessage) {
    if (!errorMessage) return;
    
    const errorMsg = errorMessage.toLowerCase();
    
    // Network settings
    if (errorMsg.includes('wifi')) {
        document.querySelector('[name="wifi_ssid"]').classList.add('error-highlight');
        document.querySelector('[name="wifi_pass"]').classList.add('error-highlight');
    }
    
    // MQTT settings
    else if (errorMsg.includes('mqtt')) {
        document.querySelector('[name="mqtt_server"]').classList.add('error-highlight');
        document.querySelector('[name="mqtt_port"]').classList.add('error-highlight');
    }
    
    // KNX settings
    else if (errorMsg.includes('knx area')) {
        document.querySelector('[name="knx_area"]').classList.add('error-highlight');
    }
    else if (errorMsg.includes('knx line')) {
        document.querySelector('[name="knx_line"]').classList.add('error-highlight');
    }
    else if (errorMsg.includes('knx member')) {
        document.querySelector('[name="knx_member"]').classList.add('error-highlight');
    }
    
    // BME280 settings
    else if (errorMsg.includes('bme280') || errorMsg.includes('i2c')) {
        if (errorMsg.includes('address')) {
            document.querySelector('[name="bme280_address"]').classList.add('error-highlight');
        }
        else if (errorMsg.includes('sda')) {
            document.querySelector('[name="bme280_sda"]').classList.add('error-highlight');
        }
        else if (errorMsg.includes('scl')) {
            document.querySelector('[name="bme280_scl"]').classList.add('error-highlight');
        }
        else if (errorMsg.includes('interval')) {
            document.querySelector('[name="bme280_interval"]').classList.add('error-highlight');
        }
        else {
            // If specific field isn't identified, highlight all BME280 fields
            document.querySelector('[name="bme280_address"]').classList.add('error-highlight');
            document.querySelector('[name="bme280_sda"]').classList.add('error-highlight');
            document.querySelector('[name="bme280_scl"]').classList.add('error-highlight');
            document.querySelector('[name="bme280_interval"]').classList.add('error-highlight');
        }
    }
    
    // PID settings
    else if (errorMsg.includes('kp')) {
        document.querySelector('[name="pid_kp"]').classList.add('error-highlight');
    }
    else if (errorMsg.includes('ki')) {
        document.querySelector('[name="pid_ki"]').classList.add('error-highlight');
    }
    else if (errorMsg.includes('kd')) {
        document.querySelector('[name="pid_kd"]').classList.add('error-highlight');
    }
    else if (errorMsg.includes('setpoint')) {
        document.querySelector('[name="setpoint"]').classList.add('error-highlight');
    }
}

// Function to reboot the device
function rebootDevice() {
    if (confirm('Are you sure you want to reboot the device?')) {
        const statusElement = document.getElementById('config-status');
        statusElement.textContent = 'Rebooting...';
        statusElement.className = 'status status-loading';
        
        fetch('/api/reboot', { method: 'POST' })
            .then(() => {
                // Keep the "Rebooting..." message
            })
            .catch(error => {
                console.error('Error rebooting device:', error);
                statusElement.textContent = 'Error rebooting device';
                statusElement.className = 'status status-error';
            });
    }
}

// System status update functions
function updateSystemStatus() {
    fetch('/api/system-status')
        .then(response => response.json())
        .then(data => {
            // Update WiFi status
            document.getElementById('wifi-status').textContent = data.wifi.state;
            document.getElementById('wifi-status').className = 'status-' + data.wifi.stateClass;
            
            // Update signal strength
            const signalBar = document.getElementById('wifi-signal');
            signalBar.style.width = data.wifi.signalQuality + '%';
            signalBar.textContent = data.wifi.signalQuality + '%';
            signalBar.className = 'progress-bar ' + getSignalBarClass(data.wifi.signalQuality);
            
            // Update connection quality
            document.getElementById('wifi-quality').textContent = data.wifi.connectionQuality;
            
            // Update connected time
            document.getElementById('wifi-connected-time').textContent = data.wifi.connectedSince;
            
            // Update watchdog status
            document.getElementById('system-watchdog-status').textContent = data.watchdog.systemStatus;
            document.getElementById('system-watchdog-status').className = 'status-' + data.watchdog.systemStatusClass;
            
            document.getElementById('wifi-watchdog-status').textContent = data.watchdog.wifiStatus;
            document.getElementById('wifi-watchdog-status').className = 'status-' + data.watchdog.wifiStatusClass;
            
            // Update reboot history
            const rebootHistoryElement = document.getElementById('reboot-history');
            if (data.rebootHistory.length === 0) {
                rebootHistoryElement.innerHTML = '<p>No reboot history available</p>';
            } else {
                let historyHtml = '<table class="status-table"><thead><tr><th>Time</th><th>Reason</th></tr></thead><tbody>';
                data.rebootHistory.forEach(entry => {
                    historyHtml += `<tr><td>${entry.time}</td><td>${entry.reason}</td></tr>`;
                });
                historyHtml += '</tbody></table>';
                rebootHistoryElement.innerHTML = historyHtml;
            }
        })
        .catch(error => {
            console.error('Error fetching system status:', error);
            document.getElementById('wifi-status').textContent = 'Error fetching data';
            document.getElementById('wifi-status').className = 'status-error';
        });
}

function getSignalBarClass(quality) {
    if (quality >= 80) return 'bg-success';
    if (quality >= 50) return 'bg-info';
    if (quality >= 30) return 'bg-warning';
    return 'bg-danger';
}

function reconnectWiFi() {
    if (confirm('Are you sure you want to reconnect WiFi?')) {
        document.getElementById('wifi-status').textContent = 'Reconnecting...';
        document.getElementById('wifi-status').className = 'status-warning';
        
        fetch('/api/wifi-reconnect', { method: 'POST' })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
                setTimeout(updateSystemStatus, 5000); // Update status after 5 seconds
            })
            .catch(error => {
                console.error('Error reconnecting WiFi:', error);
                alert('Error reconnecting WiFi. See console for details.');
                updateSystemStatus();
            });
    }
}

function startConfigPortal() {
    if (confirm('Are you sure you want to start the WiFi configuration portal? This will disconnect from the current network.')) {
        document.getElementById('wifi-status').textContent = 'Starting config portal...';
        document.getElementById('wifi-status').className = 'status-warning';
        
        fetch('/api/start-config-portal', { method: 'POST' })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
            })
            .catch(error => {
                console.error('Error starting config portal:', error);
                alert('Error starting config portal. See console for details.');
                updateSystemStatus();
            });
    }
}