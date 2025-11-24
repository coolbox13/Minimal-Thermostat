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

    // Preset elements
    const presetSelector = document.getElementById('preset-selector');
    const presetTempDisplay = document.getElementById('preset-temp');

    // Manual override elements
    const overrideSlider = document.getElementById('override-slider');
    const overrideValue = document.getElementById('override-value');
    const toggleOverrideButton = document.getElementById('toggle-override');
    const overrideStatusElement = document.getElementById('override-status');
    const overrideInfoElement = document.getElementById('override-info');
    const overrideTimerElement = document.getElementById('override-timer');

    // Manual override state
    let manualOverrideEnabled = false;

    // Preset configuration cache
    let presetConfig = {
        current: 'none',
        eco: 18.0,
        comfort: 22.0,
        away: 16.0,
        sleep: 19.0,
        boost: 24.0
    };

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

    // Load preset configuration
    function loadPresetConfig() {
        fetch('/api/config')
            .then(response => response.json())
            .then(data => {
                if (data.presets) {
                    presetConfig.current = data.presets.current || 'none';
                    presetConfig.eco = data.presets.eco || 18.0;
                    presetConfig.comfort = data.presets.comfort || 22.0;
                    presetConfig.away = data.presets.away || 16.0;
                    presetConfig.sleep = data.presets.sleep || 19.0;
                    presetConfig.boost = data.presets.boost || 24.0;

                    // Update UI
                    if (presetSelector) {
                        presetSelector.value = presetConfig.current;
                    }
                    updatePresetDisplay();
                }
            })
            .catch(error => {
                console.error('Error loading preset config:', error);
            });
    }

    // Update preset temperature display
    function updatePresetDisplay() {
        if (!presetTempDisplay) return;

        const selectedPreset = presetSelector ? presetSelector.value : 'none';
        if (selectedPreset === 'none') {
            presetTempDisplay.textContent = '--';
        } else {
            const temp = presetConfig[selectedPreset];
            presetTempDisplay.textContent = temp ? `${temp.toFixed(1)}°C` : '--';
        }
    }

    // Handle preset selection
    if (presetSelector) {
        presetSelector.addEventListener('change', function() {
            const selectedPreset = presetSelector.value;
            updatePresetDisplay();

            if (selectedPreset !== 'none') {
                const temp = presetConfig[selectedPreset];
                if (temp !== undefined) {
                    // Update slider and apply setpoint
                    if (setpointSlider) {
                        setpointSlider.value = temp;
                    }
                    if (setpointValue) {
                        setpointValue.textContent = `${temp.toFixed(1)}°C`;
                    }

                    // Automatically set the temperature
                    if (statusElement) {
                        statusElement.textContent = `Applying ${selectedPreset} preset (${temp.toFixed(1)}°C)...`;
                    }

                    fetch('/api/setpoint', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/x-www-form-urlencoded'
                        },
                        body: `value=${temp}`
                    })
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            if (statusElement) {
                                statusElement.textContent = `${selectedPreset.charAt(0).toUpperCase() + selectedPreset.slice(1)} preset applied: ${data.setpoint}°C`;
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
                        console.error('Error setting preset:', error);
                        if (statusElement) {
                            statusElement.textContent = `Error: ${error.message}`;
                        }
                    });
                }
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
        console.log('Enabling manual override with position:', position);

        fetch('/api/manual-override', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: `enabled=true&position=${position}`
        })
        .then(response => {
            console.log('Response status:', response.status);
            return response.json();
        })
        .then(data => {
            console.log('Response data:', data);
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
    loadPresetConfig();

    // Fetch data every 30 seconds
    setInterval(fetchSensorData, 30000);

    // Update override status every 5 seconds (for countdown timer)
    setInterval(updateOverrideStatus, 5000);

    // Temperature History Graph
    let historyChart = null;
    let historyData = null;
    let selectedTimeRange = 1; // Default to 1 hour

    function fetchHistoryData() {
        // Fetch all available data
        fetch('/api/history?maxPoints=0')
            .then(response => response.json())
            .then(data => {
                historyData = data;
                updateHistoryChart();
            })
            .catch(error => {
                console.error('Error fetching history data:', error);
            });
    }

    function filterDataByTimeRange(data, hours) {
        if (!data || !data.timestamps || data.timestamps.length === 0) {
            return data;
        }

        const now = Date.now() / 1000; // Current time in seconds
        const cutoffTime = now - (hours * 3600); // Hours ago in seconds

        // Find indices within the time range
        const indices = [];
        for (let i = 0; i < data.timestamps.length; i++) {
            if (data.timestamps[i] >= cutoffTime) {
                indices.push(i);
            }
        }

        // If no data in range, return empty arrays
        if (indices.length === 0) {
            return {
                timestamps: [],
                temperatures: [],
                humidities: [],
                pressures: [],
                valvePositions: []
            };
        }

        // Filter all arrays
        return {
            timestamps: indices.map(i => data.timestamps[i]),
            temperatures: indices.map(i => data.temperatures[i]),
            humidities: indices.map(i => data.humidities[i]),
            pressures: indices.map(i => data.pressures[i]),
            valvePositions: indices.map(i => data.valvePositions[i])
        };
    }

    function updateHistoryChart() {
        const ctx = document.getElementById('historyChart');
        if (!ctx || !historyData) return;

        // Filter data based on selected time range
        const filteredData = filterDataByTimeRange(historyData, selectedTimeRange);

        // Convert timestamps to time labels (HH:MM format in 24-hour notation)
        // Note: timestamps from backend are Unix time in seconds, JavaScript Date expects milliseconds
        const labels = filteredData.timestamps.map(ts => {
            const date = new Date(ts * 1000);
            return date.toLocaleTimeString('nl-NL', { hour: '2-digit', minute: '2-digit', hour12: false });
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
                        data: filteredData.temperatures,
                        borderColor: 'rgb(255, 99, 132)',
                        backgroundColor: 'rgba(255, 99, 132, 0.1)',
                        yAxisID: 'y',
                        tension: 0.3
                    },
                    {
                        label: 'Humidity (%)',
                        data: filteredData.humidities,
                        borderColor: 'rgb(54, 162, 235)',
                        backgroundColor: 'rgba(54, 162, 235, 0.1)',
                        yAxisID: 'y1',
                        tension: 0.3
                    },
                    {
                        label: 'Valve Position (%)',
                        data: filteredData.valvePositions,
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

    // Time range buttons
    const timeRangeButtons = document.querySelectorAll('.time-range-btn');
    timeRangeButtons.forEach(button => {
        button.addEventListener('click', function() {
            // Remove active class from all buttons
            timeRangeButtons.forEach(btn => btn.classList.remove('active'));

            // Add active class to clicked button
            this.classList.add('active');

            // Update selected time range
            selectedTimeRange = parseInt(this.getAttribute('data-range'));

            // Re-render chart with new time range
            updateHistoryChart();
        });
    });

    // Load history on page load
    fetchHistoryData();

    // Refresh history every 5 minutes
    setInterval(fetchHistoryData, 300000);
});