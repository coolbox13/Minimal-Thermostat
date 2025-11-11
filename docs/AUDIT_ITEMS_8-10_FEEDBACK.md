# Feedback on "Nice to Have" Improvements (Items 8-10)

This document provides detailed analysis and recommendations for the three "Nice to Have" improvements from the PID audit report. These improvements are NOT required for production deployment but could enhance system robustness.

---

## Item 8: Add Concurrent Access Protection (Mutex)

### Current Status
The PID controller uses global static variables that are **not protected** by mutexes or critical sections:

```cpp
// In adaptive_pid_controller.cpp
static float prev_error = 0.0f;
static float integral_error = 0.0f;
static float prev_temp = 0.0f;
// etc.
```

### Why This Matters

**Race Condition Risk**: If PID functions are called from multiple tasks or interrupts simultaneously, race conditions could occur:

1. **Read-Modify-Write Issues**: One task reads a variable, another modifies it, first task writes back old value
2. **Torn Reads/Writes**: On some architectures, float operations aren't atomic (though ESP32 is 32-bit, so single float reads/writes are usually atomic)
3. **Inconsistent State**: Multiple related variables updated non-atomically could leave the system in an inconsistent state

### Current Architecture Analysis

**Current Call Pattern** (from code review):
```cpp
// In main.cpp - main loop only
void updatePIDControl() {
    // ...
    updatePIDController(currentTemp, valvePosition);
    float finalValvePosition = getPIDOutput();
    // ...
}
```

**Key Observation**: The PID functions appear to be called **only from the main loop**, which is single-threaded. This means:
- ✅ **Low Risk** in current implementation
- ⚠️ **Future Risk** if architecture changes

### When Protection Is Needed

Protection would be necessary if:
1. PID functions called from interrupt service routines (ISRs)
2. PID functions called from FreeRTOS tasks other than main
3. Web server callbacks directly call PID functions
4. MQTT callbacks directly call PID functions

### Current Web/MQTT Callback Analysis

```cpp
// Web server (web_server.cpp:300)
_server->on("/api/setpoint", HTTP_POST, [](AsyncWebServerRequest *request) {
    setTemperatureSetpoint(setpoint); // ✅ Only sets a variable
});

// MQTT typically runs in its own task with AsyncWebServer
// But callbacks appear to only set values, not call PID_Update
```

**Finding**: Current callbacks only **set input values** (setpoint, Kp, Ki, Kd), they don't call `AdaptivePID_Update()` directly. The actual PID calculation happens in the main loop.

### Risk Assessment

| Scenario | Current Risk | Future Risk |
|----------|-------------|-------------|
| Current single-threaded main loop | **MINIMAL** | N/A |
| If ISRs added for PID | N/A | **HIGH** |
| If FreeRTOS tasks added | N/A | **MEDIUM-HIGH** |
| If callbacks call PID_Update directly | N/A | **HIGH** |

### Implementation Approach (If Needed)

**Option 1: FreeRTOS Mutex (Recommended for ESP32)**
```cpp
// In adaptive_pid_controller.cpp
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static SemaphoreHandle_t pidMutex = NULL;

void initializePIDController(void) {
    // Create mutex
    pidMutex = xSemaphoreCreateMutex();
    if (pidMutex == NULL) {
        LOG_E(TAG, "Failed to create PID mutex");
    }
    // ... rest of initialization
}

void AdaptivePID_Update(AdaptivePID_Input *input, AdaptivePID_Output *output) {
    // Try to take mutex with timeout
    if (xSemaphoreTake(pidMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // ... PID calculation code ...
        xSemaphoreGive(pidMutex);
    } else {
        LOG_W(TAG, "PID mutex timeout - skipping update");
    }
}
```

**Option 2: Disable Interrupts (Simpler, but blocks system)**
```cpp
void AdaptivePID_Update(AdaptivePID_Input *input, AdaptivePID_Output *output) {
    portDISABLE_INTERRUPTS();
    // ... PID calculation code ...
    portENABLE_INTERRUPTS();
}
```

### Recommendation

**DO NOT IMPLEMENT NOW** because:
1. Current architecture is single-threaded - no risk
2. Adding mutex adds complexity and overhead (~100-200 CPU cycles per lock/unlock)
3. Could reduce PID update frequency if poorly implemented
4. No observable issue to solve

**IMPLEMENT LATER IF**:
1. You add FreeRTOS tasks that call PID functions
2. You add ISRs that call PID functions
3. You observe unexplained PID instability
4. You want to allow web/MQTT callbacks to trigger immediate PID updates

**Cost-Benefit Analysis**:
- **Benefit**: Protection against future race conditions
- **Cost**: 5-10 hours implementation + testing, slight performance overhead
- **Value**: Insurance policy, not urgent need

---

## Item 9: Add Sensor Health Monitoring with Alerts

### Current Status

After implementing **Audit Fix #3**, we now have:
- ✅ BME280 returns NaN on failure
- ✅ `isHealthy()` method available
- ✅ `updatePIDControl()` validates sensor readings

**But we lack**:
- ❌ Persistent health tracking
- ❌ User alerts/notifications
- ❌ Health history/statistics
- ❌ Automatic recovery mechanisms

### Why This Matters

**Silent Failures are Dangerous**: Without active monitoring, a sensor failure might go unnoticed:

**Failure Scenario**:
1. BME280 communication fails (I2C bus error, power issue, etc.)
2. System skips PID updates (valve stays at last position)
3. Temperature drifts away from setpoint
4. User doesn't notice until room is too hot/cold
5. No log of when/why failure occurred

**Real-World Triggers**:
- Loose wiring
- I2C bus contention
- Power supply noise
- Sensor aging/degradation
- Electromagnetic interference

### What Monitoring Should Track

1. **Failure Count**: Number of consecutive bad readings
2. **Failure Duration**: How long sensor has been unhealthy
3. **Failure Rate**: Percentage of failed readings over time window
4. **Last Good Reading**: Timestamp and value of last valid reading
5. **Recovery Events**: When sensor comes back online

### Implementation Approach

**Step 1: Add Health Tracking Class**

```cpp
// include/sensor_health_monitor.h
class SensorHealthMonitor {
public:
    static SensorHealthMonitor* getInstance();

    void recordReading(bool isValid, float value);
    bool isSensorHealthy();
    uint32_t getConsecutiveFailures();
    unsigned long getLastGoodReadingTime();
    float getLastGoodValue();
    float getFailureRate();  // Over last 5 minutes

private:
    uint32_t consecutiveFailures;
    uint32_t totalReadings;
    uint32_t failedReadings;
    unsigned long lastGoodReadingTime;
    float lastGoodValue;

    // Circular buffer for failure rate calculation
    bool readingHistory[300];  // 5 min at 1 sec intervals
    int historyIndex;
};
```

**Step 2: Integrate into main.cpp**

```cpp
void updatePIDControl() {
    float currentTemp = bme280.readTemperature();

    SensorHealthMonitor* healthMonitor = SensorHealthMonitor::getInstance();

    bool isValid = !isnan(currentTemp) && currentTemp >= -40.0f && currentTemp <= 85.0f;
    healthMonitor->recordReading(isValid, currentTemp);

    if (!isValid) {
        LOG_E(TAG_PID, "Invalid sensor reading: %.2f°C - skipping PID update", currentTemp);

        // Alert user if failures exceed threshold
        if (healthMonitor->getConsecutiveFailures() == 3) {
            // First alert at 3 failures
            sendSensorFailureAlert("Sensor may be failing");
        } else if (healthMonitor->getConsecutiveFailures() >= 10) {
            // Critical alert at 10 consecutive failures
            sendSensorFailureAlert("CRITICAL: Sensor failure detected");
        }
        return;
    }

    // ... rest of PID control
}
```

**Step 3: Add Alert Mechanisms**

```cpp
void sendSensorFailureAlert(const char* message) {
    // 1. Log to event system
    EventLog::getInstance().addEntry(LOG_ERROR, "SENSOR", message);

    // 2. Publish to MQTT for Home Assistant notification
    mqttManager.publishAlert("sensor_failure", message);

    // 3. Send webhook if configured (for push notifications)
    WebhookManager* webhookManager = WebhookManager::getInstance();
    if (webhookManager->isEnabled()) {
        webhookManager->sendEvent("sensor_failure", message);
    }

    // 4. Set system status flag
    systemStatus.setSensorHealthy(false);
}
```

**Step 4: Add Recovery Detection**

```cpp
void updatePIDControl() {
    // ... sensor reading and validation ...

    if (isValid) {
        // Check if this is a recovery from failure
        if (healthMonitor->getConsecutiveFailures() >= 3) {
            LOG_I(TAG_PID, "Sensor recovered after %lu failures",
                  healthMonitor->getConsecutiveFailures());
            sendSensorRecoveryAlert();
        }
        healthMonitor->recordReading(true, currentTemp);
    }
}
```

**Step 5: Add Web UI Indicator**

```javascript
// In status.html or dashboard
function updateSensorHealth() {
    fetch('/api/sensor-health')
        .then(response => response.json())
        .then(data => {
            const indicator = document.getElementById('sensor-health');
            if (data.healthy) {
                indicator.className = 'status-good';
                indicator.textContent = '✓ Sensor OK';
            } else {
                indicator.className = 'status-error';
                indicator.textContent = `⚠ Sensor Issue (${data.consecutiveFailures} failures)`;
            }
        });
}
```

### Estimated Implementation

| Task | Time Estimate |
|------|--------------|
| Create SensorHealthMonitor class | 2-3 hours |
| Integrate into main.cpp | 1 hour |
| Add alert mechanisms | 2-3 hours |
| Add web UI indicators | 2 hours |
| Testing and debugging | 3-4 hours |
| **Total** | **10-13 hours** |

### Benefits

1. **Early Warning**: Detect sensor degradation before complete failure
2. **Diagnostics**: Know when and how often failures occur
3. **User Confidence**: Visible health status on dashboard
4. **Remote Monitoring**: Alerts via MQTT/webhooks for off-site awareness
5. **Maintenance Planning**: Failure history helps schedule sensor replacement

### Recommendation

**IMPLEMENT IF**:
- System will be deployed remotely (vacation home, rental property)
- HVAC system is critical (elderly/infant occupants, temperature-sensitive equipment)
- You want professional-grade monitoring

**SKIP IF**:
- System used in occupied home with frequent observation
- Sensor failures expected to be obvious (e.g., display shows NaN)
- Budget/timeline is tight

**Priority**: Medium - valuable but not critical

---

## Item 10: Add Valve Position Feedback Validation

### Current Status

The system **assumes** the valve responds correctly to commands:

```cpp
// In main.cpp
knxManager.setValvePosition(finalValvePosition);
// No check if valve actually moved to that position
```

```cpp
// In adaptive_pid_controller.cpp
g_pid_input.valve_feedback = valvePosition;  // Assumes this is accurate
```

### Why This Matters

**Actuator Failures Can Occur**:

1. **Mechanical Issues**:
   - Valve stuck due to sediment/debris
   - Motor burned out
   - Gearbox failure
   - Obstruction preventing movement

2. **Communication Issues**:
   - KNX bus error (command not received)
   - Wrong address configured
   - Network congestion (command delayed/dropped)

3. **Power Issues**:
   - Valve actuator unpowered
   - Insufficient voltage for movement
   - Wiring problems

**Consequences of Undetected Valve Failure**:
- PID controller thinks valve is responding, but it's not
- Temperature error grows larger
- PID increases output to maximum (100%)
- Still no response from valve
- System appears "broken" but gives no indication why
- User troubleshoots wrong component (sensor, controller)

### What Is Valve Feedback?

**Two Types of Valves**:

1. **Open-Loop Valves** (cheaper):
   - Receive position command
   - Move to position (theoretically)
   - **No feedback** about actual position
   - Example: "Set to 50%" → valve tries to open 50%, but you don't know if it did

2. **Closed-Loop Valves** (more expensive):
   - Receive position command
   - Move to position
   - **Report back** actual position
   - Example: "Set to 50%" → valve reports "Currently at 48%"

**Current System**: Based on code, appears to support **closed-loop valves** (has valve_feedback variable), but doesn't validate it.

### Implementation Approach

**Step 1: Add Validation Logic**

```cpp
// In main.cpp
void updatePIDControl() {
    // ... PID calculation ...

    // Apply final valve position to KNX
    knxManager.setValvePosition(finalValvePosition);

    // Wait for valve to respond (valves are slow - mechanical movement)
    delay(500);  // 500ms is typical actuator response time

    // Get actual valve position from KNX feedback
    float actualValvePosition = knxManager.getValvePosition();

    // Validate valve actually moved
    float positionError = fabs(finalValvePosition - actualValvePosition);

    if (positionError > 10.0f) {  // More than 10% deviation
        LOG_W(TAG_PID, "Valve position mismatch: commanded=%.1f%%, actual=%.1f%%",
              finalValvePosition, actualValvePosition);

        // Increment failure counter
        static uint8_t valveFailureCount = 0;
        valveFailureCount++;

        if (valveFailureCount >= 5) {
            // 5 consecutive failures - likely stuck valve
            LOG_E(TAG_PID, "CRITICAL: Valve appears to be stuck or non-responsive");
            sendValveFailureAlert();
            // Could enter safe mode here
        }
    } else {
        // Valve responding correctly, reset counter
        static uint8_t valveFailureCount = 0;
        valveFailureCount = 0;
    }
}
```

**Step 2: Add Diagnostic Tracking**

```cpp
class ValveHealthMonitor {
public:
    void recordCommand(float commanded, float actual);
    bool isValveHealthy();
    float getAverageError();  // Average position error
    float getMaxError();      // Worst position error
    uint32_t getStuckCount(); // Number of times valve didn't move

private:
    float errorHistory[100];
    int historyIndex;
    uint32_t stuckCount;
};
```

**Step 3: Add Alerts**

```cpp
void sendValveFailureAlert() {
    EventLog::getInstance().addEntry(LOG_ERROR, "VALVE", "Valve appears stuck or non-responsive");
    mqttManager.publishAlert("valve_failure", "Check valve actuator");

    // Stop aggressive control to prevent damage
    // Switch to conservative mode or manual control
    systemStatus.setValveHealthy(false);
}
```

**Step 4: Add Web UI Diagnostic**

```html
<!-- In status.html -->
<div class="diagnostic-card">
    <h3>Valve Health</h3>
    <div id="valve-status">Unknown</div>
    <div>Average Error: <span id="valve-avg-error">0%</span></div>
    <div>Max Error: <span id="valve-max-error">0%</span></div>
    <div>Stuck Events: <span id="valve-stuck-count">0</span></div>
</div>
```

### Challenges and Considerations

**1. Timing Issues**:
- Valves are slow (mechanical actuation takes 1-5 seconds)
- Need to wait before checking feedback
- Waiting blocks main loop (could use async approach)

**2. Tolerance Selection**:
- What deviation is "acceptable"?
- 5% error = OK (mechanical tolerances)
- 10% error = Warning
- 20% error = Failure

**3. Open-Loop Valves**:
- If valve has no feedback, this feature is **impossible**
- Need to detect if feedback is available
- Could use "expected vs. temperature response" as indirect feedback

**4. False Alarms**:
- Slow-moving valve might trigger false alarm if checked too soon
- Network delays on KNX bus might make feedback stale
- Need debouncing logic

### Alternative: Temperature-Based Validation

**For Open-Loop Valves** (no position feedback):

```cpp
// Track if valve commands are having expected temperature effect
void validateValveEffectiveness() {
    static float lastValveCommand = 0.0f;
    static float lastTemperature = 0.0f;
    static unsigned long lastCommandTime = 0;

    // Wait 5 minutes after valve command change
    if (millis() - lastCommandTime < 300000) {
        return;
    }

    // Check if temperature moved in expected direction
    float valveChange = g_pid_input.valve_command - lastValveCommand;
    float tempChange = g_pid_input.current_temp - lastTemperature;

    if (valveChange > 10.0f && tempChange < 0.1f) {
        // Valve increased significantly but no temperature change
        LOG_W(TAG_PID, "Valve may be ineffective - no temperature response");
    }

    lastValveCommand = g_pid_input.valve_command;
    lastTemperature = g_pid_input.current_temp;
    lastCommandTime = millis();
}
```

### Estimated Implementation

| Task | Time Estimate |
|------|--------------|
| Add position validation logic | 2-3 hours |
| Create ValveHealthMonitor class | 2 hours |
| Add timing/debouncing logic | 2-3 hours |
| Add alerts and logging | 1-2 hours |
| Add web UI diagnostics | 2 hours |
| Testing with actual valve | 4-5 hours |
| **Total** | **13-17 hours** |

### Benefits

1. **Early Failure Detection**: Know immediately when valve stops responding
2. **Root Cause Diagnosis**: Distinguish between sensor failure and valve failure
3. **Prevent Damage**: Detect stuck valve before motor burns out
4. **Maintenance Planning**: Track valve wear over time

### Recommendation

**IMPLEMENT IF**:
- Using expensive/critical HVAC equipment
- Valve failures have occurred in the past
- System deployed remotely with minimal supervision
- Using closed-loop valves with position feedback

**SKIP IF**:
- Using simple open-loop valves (no feedback available)
- Budget/timeline is constrained
- Valve failures are rare/obvious
- Manual monitoring is sufficient

**Priority**: Low-Medium - useful for professional installations but not essential

---

## Summary Comparison

| Item | Complexity | Time | Value | Priority |
|------|-----------|------|-------|----------|
| **8. Mutex Protection** | Low-Medium | 5-10 hrs | Insurance policy | Defer |
| **9. Sensor Health Monitoring** | Medium | 10-13 hrs | High for remote systems | Medium |
| **10. Valve Feedback Validation** | Medium-High | 13-17 hrs | Medium for pro installs | Low-Medium |

## Overall Recommendation

**For Typical Home Installation**:
- Skip #8 (not needed in current architecture)
- Consider #9 if system will be unattended for long periods
- Skip #10 unless valve failures are a known issue

**For Professional/Commercial Installation**:
- Implement #9 (sensor monitoring) - essential for remote management
- Implement #10 if using closed-loop valves with feedback
- Defer #8 unless architecture changes to use multiple tasks

**Implementation Order** (if doing multiple):
1. #9 first (most value, moderate effort)
2. #10 second (if applicable to valve type)
3. #8 last (only if architecture changes)



Added the feedback from Home Assitant audit:


Based on my audit of the Home Assistant MQTT integration, I've identified the root cause of why some sensors show "Unknown". Here's my comprehensive analysis:

  HOME ASSISTANT MQTT INTEGRATION AUDIT REPORT

  Current Status

  Working Sensors (5/10):
  - ✅ Temperature: 20.3°C
  - ✅ Humidity: 53%
  - ✅ Pressure: 1,013 hPa
  - ✅ Valve Position: 0%
  - ✅ Heating Status: Not running

  Not Working Sensors (5/10):
  - ❌ PID Kp: Unknown
  - ❌ PID Ki: Unknown
  - ❌ PID Kd: Unknown
  - ❌ Uptime: Unknown
  - ❌ WiFi Signal: Unknown

  ---
  ROOT CAUSE ANALYSIS

  1. Discovery vs State Updates

  The code has two distinct phases:

  Phase 1: Discovery (Registration) - home_assistant.cpp:35-274
  - ✅ ALL sensors are properly registered via MQTT discovery
  - ✅ Discovery configs published to topics like homeassistant/sensor/esp32_thermostat_pid_kp/config
  - ✅ All 10 entities are visible in Home Assistant

  Phase 2: State Updates - home_assistant.cpp:277-344
  - ❌ CRITICAL ISSUE: Not all sensors receive regular state updates
  - Only updateStates() is called regularly from main loop
  - updatePIDParameters() and updateDiagnostics() are NEVER called

  2. Code Flow Analysis

  What's being called regularly:

  From main.cpp:298-346 and mqtt_manager.cpp:51-58:

  updateSensorReadings()
    └─> mqttManager.publishSensorData(temp, humidity, pressure)
        └─> homeAssistant->updateStates(temp, humidity, pressure, valvePosition)

  This only updates 5 sensors: temperature, humidity, pressure, valve position, heating status.

  What's NOT being called:

  - updatePIDParameters(Kp, Ki, Kd) - src/home_assistant.cpp:310-322
  - updateDiagnostics(wifiRSSI, uptime) - src/home_assistant.cpp:325-333

  These functions exist but are orphaned - no code path calls them!

  3. Home Assistant MQTT Requirements

  According to HA documentation, sensors need:

  1. ✅ Discovery config (one-time) - DONE
  2. ✅ State topic defined - DONE (e.g., esp32_thermostat/pid/kp)
  3. ❌ Regular state updates published - MISSING for 5 sensors
  4. ✅ Availability topic - DONE (all sensors share esp32_thermostat/status)

  Key Finding: Home Assistant shows "Unknown" when:
  - Sensor is discovered (registered) ✅
  - But no state message has ever been published to its state_topic ❌

  ---
  DETAILED FINDINGS

  Discovery Configuration (Lines 186-273)

  All sensors are correctly configured with:
  - ✅ device_class (e.g., "signal_strength", "duration")
  - ✅ state_topic (where ESP32 should publish values)
  - ✅ unit_of_measurement (dBm, s, etc.)
  - ✅ unique_id for entity registry
  - ✅ availability_topic for online/offline status

  Example for WiFi Signal:
  {
    "name": "WiFi Signal",
    "device_class": "signal_strength",
    "state_topic": "esp32_thermostat/wifi/rssi",
    "unit_of_measurement": "dBm",
    "unique_id": "esp32_thermostat_wifi_rssi"
  }

  State Update Functions

  1. updateStates() - Lines 277-307 ✅ WORKING
  - Called from: mqtt_manager.cpp:56
  - Frequency: Every 30 seconds (sensor update interval)
  - Publishes: temperature, humidity, pressure, valve position, heating status

  2. updatePIDParameters() - Lines 310-322 ❌ NEVER CALLED
  - Expected to publish: PID Kp, Ki, Kd values
  - Topics: esp32_thermostat/pid/kp, /ki, /kd
  - Issue: No caller in main.cpp or mqtt_manager.cpp

  3. updateDiagnostics() - Lines 325-333 ❌ NEVER CALLED
  - Expected to publish: WiFi RSSI, uptime
  - Topics: esp32_thermostat/wifi/rssi, esp32_thermostat/uptime
  - Issue: No caller in main.cpp or mqtt_manager.cpp

  ---
  BIDIRECTIONAL COMMUNICATION CHECK

  ESP32 → Home Assistant (State Updates)

  - ✅ Working for 5 sensors
  - ❌ Missing for 5 sensors (not called)

  Home Assistant → ESP32 (Commands)

  Checked mqtt_manager.cpp:93-151:

  ✅ Temperature Setpoint (Line 129-142):
  - Topic: esp32_thermostat/temperature/set
  - Action: Calls setTemperatureSetpoint()
  - Status: Properly subscribed (line 170)

  ✅ Valve Position Command (Line 107-126):
  - Topics: MQTT_TOPIC_VALVE_COMMAND, esp32_thermostat/valve/set
  - Action: Updates valve via KNXManager
  - Status: Properly subscribed (lines 168-169)

  ✅ System Restart (Line 144-150):
  - Topic: esp32_thermostat/restart
  - Status: Properly subscribed (line 171)

  Finding: Bidirectional communication is properly implemented for climate control.

  ---
  COMPARISON WITH HA BEST PRACTICES

  ✅ Following Best Practices:

  1. Using MQTT discovery protocol correctly
  2. Proper device registration with identifiers
  3. Availability topic for online/offline status
  4. Retained messages for discovery configs
  5. Appropriate device_class for sensor types
  6. Unique IDs for entity registry

  ⚠️ Missing Best Practices:

  1. State updates on change: PID parameters should publish when changed
  2. Periodic diagnostics: WiFi/uptime should update regularly (every 60s recommended)
  3. Initial state publish: After discovery, should immediately publish current values

  Home Assistant Expectations:

  According to HA MQTT sensor documentation:
  - Sensors need at least one state message after discovery to show a value
  - State updates should be:
    - On change for parameters that change infrequently (PID values)
    - Periodic for diagnostics (WiFi RSSI every 60s, uptime every 60s)
    - Real-time for sensor readings (temperature every 30s) ✅

  ---
  RECOMMENDATIONS

  1. Add Regular Diagnostic Updates (High Priority)

  Add to main loop in main.cpp:
  // Update diagnostics every 60 seconds
  static unsigned long lastDiagnosticsUpdate = 0;
  if (millis() - lastDiagnosticsUpdate > 60000) {
      int rssi = WiFi.RSSI();
      unsigned long uptime = millis() / 1000;
      mqttManager.updateDiagnostics(rssi, uptime);
      lastDiagnosticsUpdate = millis();
  }

  2. Publish PID Parameters on Change (High Priority)

  In main.cpp:416-436 where PID params are saved:
  if (pendingConfigWrite && configWriteElapsed > ...) {
      // ... existing code ...
      // Add MQTT update after saving
      mqttManager.updatePIDParameters(g_pid_input.Kp, g_pid_input.Ki, g_pid_input.Kd);
  }

  3. Add MQTTManager Methods (Required)

  In mqtt_manager.h and mqtt_manager.cpp:
  void updatePIDParameters(float kp, float ki, float kd);
  void updateDiagnostics(int wifiRSSI, unsigned long uptime);

  4. Publish Initial State After Discovery (Medium Priority)

  After _homeAssistant->begin() in mqtt_manager.cpp:35, publish current values immediately.

  ---
  SUMMARY

  The issue is not with Home Assistant or the discovery protocol. The ESP32 firmware correctly registers all sensors, but forgets to send state updates for half of them. The missing sensors (PID params, WiFi signal, uptime) are "orphaned" - their
  update functions exist but are never called from the main loop.

  Fix Strategy:
  1. Call updateDiagnostics() every 60 seconds from main loop
  2. Call updatePIDParameters() when PID values change
  3. Publish initial values immediately after MQTT connection

  This is a simple integration issue - the plumbing exists, it's just not connected to the water supply.