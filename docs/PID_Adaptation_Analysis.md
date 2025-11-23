# PID Adaptation System Analysis

## Date: 2025-01-23

## Overview

Analysis of the PID controller's auto-tuning and continuous adaptation system, identifying issues with configurability and control over adaptive behavior.

## Current State of Auto-tune vs Manual PID

### The Problem

The PID adaptation system has a disconnect between what's configurable and what's actually used by the controller. Users cannot disable adaptive behavior, making manual PID tuning and calibration difficult.

## System Architecture

### Two Separate Adaptation Mechanisms

#### 1. One-Time Auto-Tune (Ziegler-Nichols)

**Location:** `src/adaptive_pid_controller.cpp:168-172`

```cpp
// Try auto-tuning if we have enough data (once when the buffer is full)
static int auto_tuned = 0;
if (!auto_tuned && g_history_index == 0) {
    AdaptivePID_AutoTune(&g_pid_input, g_temperature_history, HISTORY_SIZE);
    auto_tuned = 1;
}
```

**Behavior:**
- Runs automatically once when temperature history buffer fills (300 samples = 5 minutes after startup)
- Uses simplified Ziegler-Nichols relay method
- Analyzes oscillation period and amplitude to calculate PID parameters
- Applies conservative HVAC adjustments (50% Kp, 30% Ki, 70% Kd reduction)
- **Cannot be disabled or retriggered without reboot**

#### 2. Continuous Adaptation (Rule-Based)

**Location:** `src/adaptive_pid_controller.cpp:330-336`

```cpp
if (input->adaptation_enabled) {
    trackPerformanceMetrics(input, error);
    if (adaptation_timer >= adaptation_interval_sec) {
        adaptParameters(input, output, oscillation_count, max_overshoot, error_sum / (samples_count > 0 ? samples_count : 1));
        adaptation_timer = 0.0f;
    }
}
```

**Behavior:**
- Runs periodically at configurable interval (default: 60 seconds)
- Adjusts Kp, Ki, Kd based on observed performance:
  - Oscillations → reduce Kp, increase Kd
  - Overshoot → reduce Kp, increase Kd
  - Steady-state error → increase Ki
  - Slow response → increase Kp
- Changes are conservative (5% adaptation rate)
- **Always enabled despite stored preference**

## Configuration System Issues

### Issue 1: Hardcoded Adaptation Enable

**File:** `src/adaptive_pid_controller.cpp:74`

```cpp
// Adaptation parameters - enable by default
g_pid_input.adaptation_enabled = 1;   // Enable self-learning
```

**Problem:** Value is hardcoded to 1 (enabled), ignoring any stored preference.

### Issue 2: Disconnected Storage System

Two separate persistence systems exist but don't communicate:

#### PersistenceManager (Has the Setting)

**File:** `include/persistence_manager.h:42-43`

```cpp
bool setAdaptationEnabled(bool enabled);
bool getAdaptationEnabled();
```

**Implementation:** `src/persistence_manager.cpp:55-60`

```cpp
bool PersistenceManager::setAdaptationEnabled(bool enabled) {
    return preferences.putBool(KEY_ADAPT_ENABLED, enabled);
}

bool PersistenceManager::getAdaptationEnabled() {
    return preferences.getBool(KEY_ADAPT_ENABLED, true);  // Defaults to TRUE
}
```

**Storage Key:** `"adapt_en"` in ESP32 Preferences

**JSON Export:** Included in `persistence_manager.cpp:104`

```cpp
root["adaptation_enabled"] = preferences.getBool(KEY_ADAPT_ENABLED, true);
```

#### ConfigManager (Missing the Setting)

**File:** `include/config_manager.h:396-397`

```cpp
// Only has interval, NOT the enabled/disabled flag
float getPidAdaptationInterval();
void setPidAdaptationInterval(float interval);
```

**Problem:** ConfigManager is what `initializePIDController()` uses, but it doesn't have adaptation_enabled methods.

### Issue 3: Not Exposed in Web API

**File:** `src/web_server.cpp:640-641`

```cpp
// PID configuration - adaptation interval only (no enabled flag exists)
doc["pid"]["adaptation_interval"] = configManager->getPidAdaptationInterval();
```

**Comment explicitly acknowledges the missing flag.**

## What's Currently Editable

### Via Config Page

✅ **PID Parameters:**
- Kp (Proportional gain): 0-100
- Ki (Integral gain): 0-10
- Kd (Derivative gain): 0-10
- These are saved and loaded correctly via ConfigManager

✅ **Adaptation Interval:**
- How often continuous adaptation runs (seconds)
- Stored in ConfigManager
- Exposed in web API

❌ **Adaptation Enabled/Disabled:**
- NOT in ConfigManager
- NOT in web API
- NOT in config page UI
- Stored in PersistenceManager but never used

❌ **Auto-Tune Control:**
- Cannot disable one-time auto-tune
- Cannot trigger auto-tune on demand
- No way to re-run without reboot

## Impact on Calibration

### Problems for Manual PID Tuning

1. **Cannot test fixed PID values** - continuous adaptation will change them over time
2. **Cannot compare before/after** - parameters drift during observation period
3. **Cannot validate manual calculations** - auto-tune overwrites values after 5 minutes
4. **Cannot isolate variables** - can't tell if behavior is from PID values or adaptation logic

### Problems for System Comparison

From `docs/calibration.md`:
- Need to run both ESP32 and reference thermostat in parallel
- Need stable, known PID parameters for fair comparison
- Continuous adaptation introduces moving target
- Auto-tune timing interferes with warm-start testing

## Required Fix

### Phase 1: Add ConfigManager Support

**File:** `include/config_manager.h`

Add methods:
```cpp
bool getAdaptationEnabled();
void setAdaptationEnabled(bool enabled);
```

**File:** `src/config_manager.cpp`

Implement by delegating to PersistenceManager:
```cpp
bool ConfigManager::getAdaptationEnabled() {
    return PersistenceManager::getAdaptationEnabled();
}

void ConfigManager::setAdaptationEnabled(bool enabled) {
    PersistenceManager::setAdaptationEnabled(enabled);
}
```

### Phase 2: Use the Setting

**File:** `src/adaptive_pid_controller.cpp:74`

Change from:
```cpp
g_pid_input.adaptation_enabled = 1;
```

To:
```cpp
g_pid_input.adaptation_enabled = configManager->getAdaptationEnabled();
```

### Phase 3: Expose in Web API

**File:** `src/web_server.cpp` (GET /api/config)

Add to PID section:
```cpp
doc["pid"]["adaptation_enabled"] = configManager->getAdaptationEnabled();
doc["pid"]["adaptation_interval"] = configManager->getPidAdaptationInterval();
```

**File:** `src/web_server.cpp` (POST /api/config)

Add parameter handling:
```cpp
if (request->hasParam("adaptation_enabled", true)) {
    bool enabled = request->getParam("adaptation_enabled", true)->value() == "true";
    configManager->setAdaptationEnabled(enabled);
}
```

### Phase 4: Update Frontend Config Page

**File:** `frontend/src/pages/Config.jsx`

Add toggle in PID section:
```jsx
<label>
  <input
    type="checkbox"
    checked={formData.pid?.adaptation_enabled ?? true}
    onChange={(e) => updateFormData('pid', {
      ...formData.pid,
      adaptation_enabled: e.target.checked
    })}
  />
  Enable Continuous Adaptation
</label>
```

### Phase 5: Optional - Manual Auto-Tune Trigger

Add button to re-run one-time auto-tune:

**Backend:** Add API endpoint `/api/pid/autotune` (POST)

**Frontend:** Add button in config page to trigger auto-tune

**Implementation:** Reset `auto_tuned` static flag and force re-run

## Recommended Configuration for Calibration

### Week 1-2: Baseline with Adaptation OFF

```json
{
  "pid": {
    "kp": 2.0,
    "ki": 0.1,
    "kd": 0.5,
    "adaptation_enabled": false,
    "adaptation_interval": 60
  }
}
```

**Purpose:** Collect baseline data with known, fixed PID values

### Week 3: Test Auto-Tune

1. Trigger auto-tune manually
2. Disable adaptation immediately after
3. Test with auto-tuned values (fixed)

### Week 4: Test Continuous Adaptation

```json
{
  "pid": {
    "adaptation_enabled": true,
    "adaptation_interval": 300  // 5 minutes for slower adaptation
  }
}
```

**Purpose:** Compare adaptive vs fixed tuning

## System Behavior Summary

### Current (Broken) Behavior

```
Startup → Load PID from ConfigManager (e.g., Kp=2.0)
        ↓
After 5 min → Auto-tune runs (overwrites to Kp=1.5)
        ↓
Every 60s → Continuous adaptation (Kp drifts to 1.7, 1.6, etc.)
        ↓
User edits Kp=2.0 in config page
        ↓
Next adaptation cycle → Back to Kp=1.6 (change lost)
```

### Fixed Behavior (After Implementation)

```
Startup → Load PID from ConfigManager
        ↓
        → Load adaptation_enabled flag
        ↓
If adaptation_enabled = false:
   → PID values stay at configured values
   → No auto-tune
   → No continuous changes
   → User has full control

If adaptation_enabled = true:
   → Auto-tune runs after 5 min
   → Continuous adaptation every N seconds
   → Self-optimizing behavior
   → Good for production, bad for calibration
```

## Testing Checklist

After implementing the fix:

- [ ] ConfigManager methods work (get/set adaptation_enabled)
- [ ] PID controller respects the flag
- [ ] Web API exposes the setting
- [ ] Config page shows toggle
- [ ] Setting persists across reboots
- [ ] Disabled adaptation prevents auto-tune
- [ ] Disabled adaptation prevents continuous changes
- [ ] Manual PID edits are preserved when disabled
- [ ] Enabling adaptation allows auto-tune to run
- [ ] Enabling adaptation allows continuous tuning

## References

**Related Files:**
- `src/adaptive_pid_controller.cpp` - Main PID implementation
- `src/adaptive_pid_controller.h` - PID interface
- `src/config_manager.cpp` - Configuration management
- `src/persistence_manager.cpp` - Low-level storage
- `src/web_server.cpp` - HTTP API
- `frontend/src/pages/Config.jsx` - Configuration UI
- `docs/calibration.md` - Calibration methodology

**Related Issues:**
- Need for manual PID calibration against reference thermostat
- Inability to test fixed PID parameters
- Hidden adaptation interfering with troubleshooting
