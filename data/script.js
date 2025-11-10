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

    // Manual override elements
    const overrideSlider = document.getElementById('override-slider');
    const overrideValue = document.getElementById('override-value');
    const toggleOverrideButton = document.getElementById('toggle-override');
    const overrideStatusElement = document.getElementById('override-status');
    const overrideInfoElement = document.getElementById('override-info');
    const overrideTimerElement = document.getElementById('override-timer');

    // Manual override state
    let manualOverrideEnabled = false;

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

    // Update override value display when slider changes
    if (overrideSlider) {
        overrideSlider.addEventListener('input', function() {
            if (overrideValue) {
                overrideValue.textContent = `${overrideSlider.value}%`;
            }
        });
    }

    // Toggle manual override
    if (toggleOverrideButton) {
        toggleOverrideButton.addEventListener('click', function() {
            if (manualOverrideEnabled) {
                // Disable override
                disableManualOverride();
            } else {
                // Enable override
                enableManualOverride();
            }
        });
    }

    function enableManualOverride() {
        const position = overrideSlider ? overrideSlider.value : 0;

        fetch('/api/manual-override', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: `enabled=true&position=${position}`
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                manualOverrideEnabled = true;
                if (overrideSlider) overrideSlider.disabled = false;
                if (toggleOverrideButton) {
                    toggleOverrideButton.textContent = 'Disable Manual Control';
                    toggleOverrideButton.style.backgroundColor = '#e74c3c';
                }
                if (overrideStatusElement) {
                    overrideStatusElement.textContent = 'Manual control active';
                    overrideStatusElement.style.color = '#e74c3c';
                }
                if (overrideInfoElement) {
                    overrideInfoElement.style.display = 'block';
                }
                updateOverrideStatus();
            } else {
                if (overrideStatusElement) {
                    overrideStatusElement.textContent = `Error: ${data.message}`;
                }
            }
        })
        .catch(error => {
            console.error('Error enabling manual override:', error);
            if (overrideStatusElement) {
                overrideStatusElement.textContent = `Error: ${error.message}`;
            }
        });
    }

    function disableManualOverride() {
        fetch('/api/manual-override', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: 'enabled=false'
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                manualOverrideEnabled = false;
                if (overrideSlider) overrideSlider.disabled = true;
                if (toggleOverrideButton) {
                    toggleOverrideButton.textContent = 'Enable Manual Control';
                    toggleOverrideButton.style.backgroundColor = '';
                }
                if (overrideStatusElement) {
                    overrideStatusElement.textContent = '';
                }
                if (overrideInfoElement) {
                    overrideInfoElement.style.display = 'none';
                }
            } else {
                if (overrideStatusElement) {
                    overrideStatusElement.textContent = `Error: ${data.message}`;
                }
            }
        })
        .catch(error => {
            console.error('Error disabling manual override:', error);
            if (overrideStatusElement) {
                overrideStatusElement.textContent = `Error: ${error.message}`;
            }
        });
    }

    function updateOverrideStatus() {
        fetch('/api/manual-override')
        .then(response => response.json())
        .then(data => {
            if (data.enabled) {
                manualOverrideEnabled = true;
                if (overrideSlider) {
                    overrideSlider.value = data.position;
                    overrideSlider.disabled = false;
                }
                if (overrideValue) {
                    overrideValue.textContent = `${data.position}%`;
                }
                if (toggleOverrideButton) {
                    toggleOverrideButton.textContent = 'Disable Manual Control';
                    toggleOverrideButton.style.backgroundColor = '#e74c3c';
                }
                if (overrideInfoElement) {
                    overrideInfoElement.style.display = 'block';
                }
                if (overrideTimerElement && data.remaining_seconds !== undefined) {
                    const minutes = Math.floor(data.remaining_seconds / 60);
                    const seconds = data.remaining_seconds % 60;
                    overrideTimerElement.textContent =
                        `Auto-disable in: ${minutes}m ${seconds}s`;
                }
            } else {
                manualOverrideEnabled = false;
                if (overrideSlider) overrideSlider.disabled = true;
                if (toggleOverrideButton) {
                    toggleOverrideButton.textContent = 'Enable Manual Control';
                    toggleOverrideButton.style.backgroundColor = '';
                }
                if (overrideInfoElement) {
                    overrideInfoElement.style.display = 'none';
                }
            }
        })
        .catch(error => {
            console.error('Error fetching override status:', error);
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
    updateOverrideStatus();

    // Fetch data every 30 seconds
    setInterval(fetchSensorData, 30000);

    // Update override status every 5 seconds (for countdown timer)
    setInterval(updateOverrideStatus, 5000);

    // Temperature History Graph
    let historyChart = null;

    function fetchHistoryData() {
        fetch('/api/history?maxPoints=100')
            .then(response => response.json())
            .then(data => {
                updateHistoryChart(data);
            })
            .catch(error => {
                console.error('Error fetching history data:', error);
            });
    }

    function updateHistoryChart(data) {
        const ctx = document.getElementById('historyChart');
        if (!ctx) return;

        // Convert timestamps to time labels (HH:MM format)
        const labels = data.timestamps.map(ts => {
            const date = new Date(ts);
            return date.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit' });
        });

        // Destroy existing chart if it exists
        if (historyChart) {
            historyChart.destroy();
        }

        // Create new chart
        historyChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [
                    {
                        label: 'Temperature (°C)',
                        data: data.temperatures,
                        borderColor: 'rgb(255, 99, 132)',
                        backgroundColor: 'rgba(255, 99, 132, 0.1)',
                        yAxisID: 'y',
                        tension: 0.3
                    },
                    {
                        label: 'Humidity (%)',
                        data: data.humidities,
                        borderColor: 'rgb(54, 162, 235)',
                        backgroundColor: 'rgba(54, 162, 235, 0.1)',
                        yAxisID: 'y1',
                        tension: 0.3
                    },
                    {
                        label: 'Valve Position (%)',
                        data: data.valvePositions,
                        borderColor: 'rgb(75, 192, 192)',
                        backgroundColor: 'rgba(75, 192, 192, 0.1)',
                        yAxisID: 'y1',
                        tension: 0.3
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                interaction: {
                    mode: 'index',
                    intersect: false,
                },
                plugins: {
                    legend: {
                        position: 'top',
                    },
                    title: {
                        display: false
                    }
                },
                scales: {
                    x: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Time'
                        }
                    },
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: {
                            display: true,
                            text: 'Temperature (°C)'
                        }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Humidity / Valve (%)'
                        },
                        grid: {
                            drawOnChartArea: false,
                        }
                    }
                }
            }
        });
    }

    // Refresh history button
    const refreshHistoryButton = document.getElementById('refresh-history');
    if (refreshHistoryButton) {
        refreshHistoryButton.addEventListener('click', fetchHistoryData);
    }

    // Load history on page load
    fetchHistoryData();

    // Refresh history every 5 minutes
    setInterval(fetchHistoryData, 300000);
});