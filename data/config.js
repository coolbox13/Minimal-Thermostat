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
        });
    });
    
    // Load current configuration
    loadConfiguration();
    
    // Save configuration form submit handler
    document.getElementById('config-form').addEventListener('submit', saveConfiguration);
    
    // Reboot device button handler
    document.getElementById('reboot-device').addEventListener('click', rebootDevice);
});

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
            
            // PID settings
            if (data.pid) {
                document.getElementById('pid_kp').value = data.pid.kp || 2.0;
                document.getElementById('pid_ki').value = data.pid.ki || 0.1;
                document.getElementById('pid_kd').value = data.pid.kd || 0.5;
                document.getElementById('setpoint').value = data.pid.setpoint || 22.0;
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
            kp: parseFloat(formData.get('pid_kp')),
            ki: parseFloat(formData.get('pid_ki')),
            kd: parseFloat(formData.get('pid_kd')),
            setpoint: parseFloat(formData.get('setpoint'))
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