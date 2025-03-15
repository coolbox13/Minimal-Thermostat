// Highlight active tab based on URL
const tabs = document.querySelectorAll(".tab");
const currentPath = window.location.pathname;

// Show/hide content based on current path
const dashboardContent = document.getElementById('dashboard-content');
const persistenceContent = document.getElementById('persistence-content');

function updateContentVisibility() {
    if (currentPath === '/') {
        dashboardContent.style.display = 'block';
        persistenceContent.style.display = 'none';
    } else if (currentPath === '/persistence') {
        dashboardContent.style.display = 'none';
        persistenceContent.style.display = 'block';
    }
}

tabs.forEach(tab => {
    if ((currentPath === "/" && tab.getAttribute("href") === "/") || 
        (currentPath !== "/" && tab.getAttribute("href") === currentPath)) {
        tab.classList.add("active");
    }
});

updateContentVisibility();

// Fetch sensor data for dashboard
function updateSensorData() {
    fetch('/api/sensor-data')
        .then(response => response.json())
        .then(data => {
            document.getElementById('temperature').textContent = data.temperature.toFixed(2) + '°C';
            document.getElementById('humidity').textContent = data.humidity.toFixed(2) + '%';
            document.getElementById('pressure').textContent = data.pressure.toFixed(2) + ' hPa';
            document.getElementById('valve').textContent = data.valve + '%';
            document.getElementById('setpoint').textContent = data.setpoint.toFixed(2) + '°C';
        })
        .catch(error => console.error('Error fetching sensor data:', error));
}

// Load persistence data
function loadPersistenceData() {
    fetch('/api/persistence')
        .then(response => response.json())
        .then(data => {
            document.getElementById('setpoint_temp').value = data.setpoint_temp;
            document.getElementById('kp').value = data.kp;
            document.getElementById('ki').value = data.ki;
            document.getElementById('kd').value = data.kd;
        })
        .catch(error => console.error('Error loading persistence data:', error));
}

// Handle persistence form submission
document.getElementById('persistenceForm').addEventListener('submit', function(e) {
    e.preventDefault();
    const formData = {
        setpoint_temp: parseFloat(document.getElementById('setpoint_temp').value),
        kp: parseFloat(document.getElementById('kp').value),
        ki: parseFloat(document.getElementById('ki').value),
        kd: parseFloat(document.getElementById('kd').value)
    };

    fetch('/api/persistence', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(formData)
    })
    .then(response => response.json())
    .then(data => {
        alert('Settings saved successfully!');
    })
    .catch(error => {
        console.error('Error saving settings:', error);
        alert('Error saving settings. Please try again.');
    });
});

// Update sensor data every 5 seconds (only if we're on the dashboard)
if (currentPath === "/") {
    setInterval(updateSensorData, 5000);
    updateSensorData();
}

// Load persistence data if we're on the persistence page
if (currentPath === "/persistence") {
    loadPersistenceData();
}