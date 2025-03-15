// Main JavaScript for ESP32 KNX Thermostat

document.addEventListener('DOMContentLoaded', function() {
    // Get DOM elements
    const temperatureElement = document.getElementById('temperature');
    const humidityElement = document.getElementById('humidity');
    const pressureElement = document.getElementById('pressure');
    const valveElement = document.getElementById('valve');
    const setpointSlider = document.getElementById('setpoint-slider');
    const setpointValue = document.getElementById('setpoint-value');
    const setTemperatureButton = document.getElementById('set-temperature');
    const refreshButton = document.getElementById('refresh-data');
    const statusElement = document.getElementById('status');
    
    // Safe update function for elements
    function updateElement(element, value) {
        if (element) {
            element.textContent = value;
        }
    }
    
    // Update setpoint display when slider changes
    if (setpointSlider) {
        setpointSlider.addEventListener('input', function() {
            if (setpointValue) {
                setpointValue.textContent = `${setpointSlider.value}°C`;
            }
        });
    }
    
    // Set temperature when button is clicked
    if (setTemperatureButton) {
        setTemperatureButton.addEventListener('click', function() {
            const setpoint = setpointSlider ? setpointSlider.value : 22.0;
            
            if (statusElement) {
                statusElement.textContent = `Setting temperature to ${setpoint}°C...`;
            }
            
            fetch('/api/setpoint', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded'
                },
                body: `value=${setpoint}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    if (statusElement) {
                        statusElement.textContent = `Temperature set to ${data.setpoint}°C`;
                    }
                    setTimeout(() => {
                        fetchSensorData();
                    }, 500);
                } else {
                    if (statusElement) {
                        statusElement.textContent = `Error: ${data.message}`;
                    }
                }
            })
            .catch(error => {
                console.error('Error setting temperature:', error);
                if (statusElement) {
                    statusElement.textContent = `Error: ${error.message}`;
                }
            });
        });
    }
    
    // Refresh data when button is clicked
    if (refreshButton) {
        refreshButton.addEventListener('click', fetchSensorData);
    }
    
    // Function to fetch sensor data
    function fetchSensorData() {
        if (statusElement) {
            statusElement.textContent = 'Fetching data...';
        }
        
        fetch('/api/sensor-data')
        .then(response => response.json())
        .then(data => {
            if (temperatureElement) {
                temperatureElement.textContent = `${data.temperature.toFixed(1)}°C`;
            }
            if (humidityElement) {
                humidityElement.textContent = `${data.humidity.toFixed(1)}%`;
            }
            if (pressureElement) {
                pressureElement.textContent = `${data.pressure.toFixed(1)} hPa`;
            }
            if (valveElement) {
                valveElement.textContent = `${data.valve.toFixed(0)}%`;
            }
            
            // Update slider if setpoint has changed
            if (data.setpoint && setpointSlider) {
                setpointSlider.value = data.setpoint;
                if (setpointValue) {
                    setpointValue.textContent = `${data.setpoint.toFixed(1)}°C`;
                }
            }
            
            if (statusElement) {
                statusElement.textContent = `Data updated at ${new Date().toLocaleTimeString()}`;
            }
        })
        .catch(error => {
            console.error('Error fetching sensor data:', error);
            if (statusElement) {
                statusElement.textContent = `Error: ${error.message}`;
            }
        });
    }
    
    // Fetch data on page load
    fetchSensorData();
    
    // Fetch data every 30 seconds
    setInterval(fetchSensorData, 30000);
});