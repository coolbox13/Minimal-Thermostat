# Frontend Audit Prompt - ESP32 KNX Thermostat Web Interface

**Date:** 2025-11-22
**Scope:** Complete audit of vanilla JavaScript frontend implementation
**Focus:** Code execution, data flows, memory leaks, graph visualization failures, and functional bugs

---

## Executive Summary

Conduct a comprehensive technical audit of the ESP32 KNX Thermostat web frontend to identify:
1. **Critical bugs** causing frontend failures
2. **Graph visualization failures** (not displaying on any device)
3. **Memory leaks** and performance issues
4. **Dead code** and unused functions
5. **Event handler issues** and race conditions
6. **API communication problems** and error handling gaps
7. **Mobile responsiveness** and UX issues
8. **Security vulnerabilities** (XSS, injection, etc.)

---

## Audit Scope

### Files to Audit

#### Core Frontend Files
- `/data/index.html` - Main dashboard HTML
- `/data/script.js` - Main dashboard JavaScript (579 lines)
- `/data/status.html` - System status page HTML
- `/data/status.js` - System status JavaScript (287 lines)
- `/data/config.html` - Configuration page HTML
- `/data/config.js` - Configuration JavaScript (574 lines)
- `/data/logs.html` - Event logs page (inline JavaScript)
- `/data/style.css` - Global styles (490 lines)
- `/data/serial.html` - Serial monitor page (if exists)

#### Supporting Files
- API endpoints documentation (`/docs/API.md`)
- Backend implementation for context

---

## Critical Issues to Investigate

### 1. Graph Visualization Failure ⚠️ HIGH PRIORITY

**Symptoms:**
- Graph not displaying on any device (desktop or mobile)
- User reports "we can not get that to show properly at all"

**Investigation Areas:**

#### A. Chart.js Loading and Initialization
```javascript
// Location: /data/index.html:142
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
```

**Check:**
- [ ] Is Chart.js CDN accessible and loading successfully?
- [ ] Is there a network error blocking the CDN?
- [ ] Browser console errors related to Chart.js?
- [ ] Is the script loaded before `script.js` executes?
- [ ] Does `Chart` global exist when `script.js` runs?

#### B. Canvas Element Availability
```javascript
// Location: /data/script.js:451
const ctx = document.getElementById('historyChart');
```

**Check:**
- [ ] Does the `<canvas id="historyChart">` element exist in DOM?
- [ ] Is the element accessible when JavaScript tries to query it?
- [ ] Does the element have proper dimensions (not 0x0)?
- [ ] Is the canvas hidden by CSS?

#### C. Data Fetching and Structure
```javascript
// Location: /data/script.js:400-411
function fetchHistoryData() {
    fetch('/api/history?maxPoints=0')
        .then(response => response.json())
        .then(data => {
            historyData = data;
            updateHistoryChart();
        })
}
```

**Check:**
- [ ] Does `/api/history` endpoint return valid JSON?
- [ ] What is the structure of returned data?
- [ ] Are `timestamps`, `temperatures`, `humidities`, `valvePositions` arrays present?
- [ ] Are arrays empty or malformed?
- [ ] Network tab: is the request succeeding (200 OK)?
- [ ] Response time: is the API timing out?
- [ ] CORS errors preventing data fetch?

#### D. Data Processing and Filtering
```javascript
// Location: /data/script.js:413-448
function filterDataByTimeRange(data, hours) {
    if (!data || !data.timestamps || data.timestamps.length === 0) {
        return data;
    }
    // ...filtering logic
}
```

**Check:**
- [ ] Are timestamps in correct format (Unix seconds)?
- [ ] Is time range calculation correct?
- [ ] Does filtering return empty arrays unexpectedly?
- [ ] Are indices correctly mapped across all arrays?
- [ ] Timezone issues causing all data to be filtered out?

#### E. Chart Rendering Logic
```javascript
// Location: /data/script.js:450-548
function updateHistoryChart() {
    const ctx = document.getElementById('historyChart');
    if (!ctx || !historyData) return;

    // Filter data
    const filteredData = filterDataByTimeRange(historyData, selectedTimeRange);

    // Destroy existing chart
    if (historyChart) {
        historyChart.destroy();
    }

    // Create new chart
    historyChart = new Chart(ctx, { /* config */ });
}
```

**Check:**
- [ ] Is `ctx` null or undefined?
- [ ] Is `historyData` null or undefined?
- [ ] Does `filterDataByTimeRange()` return valid data?
- [ ] Are `labels` array and dataset arrays the same length?
- [ ] Chart.js throwing errors during initialization?
- [ ] Does `destroy()` fail if chart doesn't exist?
- [ ] Canvas context issues (`getContext('2d')` failing)?

#### F. Chart Configuration
```javascript
// Location: /data/script.js:470-547
historyChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: labels,  // Time labels
        datasets: [
            { /* temperature */ },
            { /* humidity */ },
            { /* valve position */ }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: true,
        // ... scales configuration
    }
});
```

**Check:**
- [ ] Are `labels` properly formatted?
- [ ] Do all datasets have valid data arrays?
- [ ] Are Y-axis configurations correct?
- [ ] Does dual Y-axis setup work?
- [ ] Responsive options causing rendering issues?
- [ ] Canvas size constraints causing failures?

#### G. Time Range Selector
```javascript
// Location: /data/script.js:556-572
const timeRangeButtons = document.querySelectorAll('.time-range-btn');
timeRangeButtons.forEach(button => {
    button.addEventListener('click', function() {
        selectedTimeRange = parseInt(this.getAttribute('data-range'));
        updateHistoryChart();
    });
});
```

**Check:**
- [ ] Do time range buttons exist in DOM?
- [ ] Are event listeners properly attached?
- [ ] Does clicking buttons trigger chart update?
- [ ] Is `selectedTimeRange` variable properly initialized?

---

### 2. Memory Leaks and Performance Issues

#### A. Interval Timers and Cleanup
```javascript
// /data/script.js:390, 393, 578
setInterval(fetchSensorData, 30000);        // Every 30 seconds
setInterval(updateOverrideStatus, 5000);    // Every 5 seconds
setInterval(fetchHistoryData, 300000);      // Every 5 minutes
```

**Check:**
- [ ] Are intervals cleared on page unload?
- [ ] Multiple intervals accumulating if page is soft-reloaded?
- [ ] Memory usage growing over time?

```javascript
// /data/status.js:15-18
setInterval(function() {
    loadSystemStatus();
    loadHealthStatus();
}, 10000);  // Every 10 seconds
```

**Check:**
- [ ] Same interval leak issues on status page?

```javascript
// /data/logs.html:299
setInterval(fetchLogs, 10000);
```

**Check:**
- [ ] Logs page accumulating memory?

#### B. Chart Destruction and Recreation
```javascript
// /data/script.js:465-467
if (historyChart) {
    historyChart.destroy();
}
historyChart = new Chart(ctx, {...});
```

**Check:**
- [ ] Is previous chart instance properly destroyed?
- [ ] Canvas memory released after destroy()?
- [ ] Event listeners removed by Chart.js destroy()?
- [ ] Memory leaks from repeated chart creation?

#### C. Event Listener Management
**Check:**
- [ ] Are event listeners added multiple times?
- [ ] Listeners added in loops without cleanup?
- [ ] Anonymous function listeners (can't be removed)?
- [ ] `removeEventListener()` called anywhere?

#### D. DOM Manipulation and References
**Check:**
- [ ] Detached DOM nodes being held in memory?
- [ ] Large data structures retained unnecessarily?
- [ ] Closures capturing large objects?

---

### 3. Data Flow and API Communication

#### A. Fetch Error Handling
```javascript
// /data/script.js:348-381
fetch('/api/sensor-data')
    .then(response => response.json())
    .then(data => {
        // Update UI
    })
    .catch(error => {
        console.error('Error fetching sensor data:', error);
        if (statusElement) {
            statusElement.textContent = `Error: ${error.message}`;
        }
    });
```

**Check:**
- [ ] Are HTTP error codes handled (404, 500, 503)?
- [ ] Is `response.ok` checked before parsing JSON?
- [ ] Malformed JSON responses caught?
- [ ] Network timeout handling?
- [ ] User feedback for all error cases?

#### B. Race Conditions
```javascript
// /data/script.js:129-131
setTimeout(() => {
    fetchSensorData();
}, 500);
```

**Check:**
- [ ] Can multiple API calls overlap?
- [ ] Is data overwritten by out-of-order responses?
- [ ] Are fetch promises tracked and cancelled?

#### C. API Endpoint Dependencies
**Verify all API endpoints exist and return expected data:**
- [ ] `GET /api/sensor-data` - Current sensor readings
- [ ] `POST /api/setpoint` - Set temperature
- [ ] `GET /api/config` - Configuration data
- [ ] `POST /api/config` - Save configuration
- [ ] `GET /api/history?maxPoints=0` - Historical data
- [ ] `GET /api/status` - System status
- [ ] `GET /api/sensor-health` - Sensor health
- [ ] `GET /api/valve-health` - Valve health
- [ ] `GET /api/manual-override` - Override status
- [ ] `POST /api/manual-override` - Set override
- [ ] `POST /api/reboot` - Reboot device
- [ ] `POST /api/factory-reset` - Factory reset
- [ ] `GET /api/logs` - Event logs
- [ ] `POST /api/logs/clear` - Clear logs
- [ ] `GET /api/config/export` - Export config
- [ ] `POST /api/config/import` - Import config
- [ ] `POST /api/webhook/test` - Test webhook

---

### 4. Code Execution Flow Analysis

#### A. Page Load Sequence
```javascript
// /data/script.js:3-4
document.addEventListener('DOMContentLoaded', function() {
    // ... initialization code
});
```

**Verify execution order:**
1. [ ] DOMContentLoaded fires correctly
2. [ ] All DOM elements are available
3. [ ] Chart.js is loaded before use
4. [ ] Initial data fetch completes
5. [ ] UI updates reflect loaded data

#### B. Function Call Chains
**Trace execution paths:**
```
Page Load → DOMContentLoaded
         → fetchSensorData()
         → updateOverrideStatus()
         → loadPresetConfig()
         → fetchHistoryData()
         → updateHistoryChart()
```

**Check for:**
- [ ] Circular dependencies
- [ ] Missing function calls
- [ ] Undefined function references
- [ ] Asynchronous timing issues

#### C. State Management
**Track state variables:**
```javascript
let historyChart = null;
let historyData = null;
let selectedTimeRange = 1;
let manualOverrideEnabled = false;
let presetConfig = { /* ... */ };
```

**Check:**
- [ ] Are variables initialized correctly?
- [ ] State updates in correct order?
- [ ] State inconsistencies between functions?
- [ ] Global variable pollution?

---

### 5. Dead Code and Unused Features

#### A. Unreachable Code
**Check for:**
- [ ] Functions defined but never called
- [ ] Event listeners for non-existent elements
- [ ] Commented-out code blocks
- [ ] Conditional branches never executed

#### B. Unused Variables
**Check:**
- [ ] Variables declared but never read
- [ ] Function parameters never used
- [ ] Imports/dependencies not utilized

#### C. Redundant DOM Queries
```javascript
document.getElementById('temperature')
document.getElementById('temperature')  // Called again
```

**Check:**
- [ ] Repeated DOM queries (cache in variables)
- [ ] querySelector vs getElementById efficiency

---

### 6. Mobile and Responsive Issues

#### A. Touch Events
```javascript
// /data/script.js - No touch event handling detected
```

**Check:**
- [ ] Are sliders usable on touch devices?
- [ ] Buttons have adequate touch targets (44x44px)?
- [ ] Gestures work on charts?
- [ ] Zoom/pinch properly handled or disabled?

#### B. Viewport and Scaling
```html
<!-- /data/index.html:5 -->
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=5.0, user-scalable=yes">
```

**Check:**
- [ ] Is layout broken on small screens?
- [ ] Does content overflow horizontally?
- [ ] Are font sizes readable on mobile?

#### C. CSS Media Queries
```css
/* /data/style.css:304-441 */
@media (max-width: 650px) {
    /* Mobile styles */
}
```

**Check:**
- [ ] Do mobile styles actually apply?
- [ ] Breakpoint at 650px appropriate?
- [ ] Graph rendering on mobile?
- [ ] Flex container wrapping correctly?

---

### 7. Security Vulnerabilities

#### A. XSS (Cross-Site Scripting)
```javascript
// /data/script.js:352
temperatureElement.textContent = `${data.temperature.toFixed(1)}°C`;
```

**Check:**
- [ ] Is `textContent` used (safe) vs `innerHTML` (unsafe)?
- [ ] User input sanitized before display?
- [ ] Configuration values escaped properly?

#### B. CSRF (Cross-Site Request Forgery)
```javascript
fetch('/api/setpoint', {
    method: 'POST',
    headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
    },
    body: `value=${setpoint}`
})
```

**Check:**
- [ ] Are POST requests protected with CSRF tokens?
- [ ] Origin/Referer headers validated on backend?
- [ ] Critical actions (reboot, factory reset) protected?

#### C. Injection Attacks
```javascript
// /data/config.js:310-316
fetch('/api/config', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(config)
})
```

**Check:**
- [ ] Is user input validated before sending to backend?
- [ ] WiFi passwords, MQTT credentials handled safely?
- [ ] No code injection possible through config?

---

### 8. Error Handling and User Feedback

#### A. Silent Failures
**Check for:**
- [ ] Errors logged to console but not shown to user
- [ ] Failed API calls with no user notification
- [ ] Invalid configuration saved without warning

#### B. Error Messages Quality
```javascript
statusElement.textContent = `Error: ${error.message}`;
```

**Check:**
- [ ] Are error messages helpful to users?
- [ ] Generic "Error loading data" vs specific issues?
- [ ] Suggested actions for common errors?

#### C. Loading States
```javascript
statusElement.textContent = 'Fetching data...';
```

**Check:**
- [ ] Loading indicators for all async operations?
- [ ] Spinners or visual feedback during long operations?
- [ ] Disable buttons during processing?

---

### 9. Configuration Page Complexity

#### A. Tab System
```javascript
// /data/config.js:6-15
document.querySelectorAll('.tab-button').forEach(button => {
    button.addEventListener('click', function() {
        // Switch tabs
    });
});
```

**Check:**
- [ ] Do tabs switch correctly?
- [ ] Tab content visibility logic working?
- [ ] Active state management correct?
- [ ] Can user get lost in 8 tabs?

#### B. Form Validation
```javascript
// /data/config.js:50-72 - Input blur handlers for formatting
```

**Check:**
- [ ] Is real-time validation present?
- [ ] Are invalid values prevented or caught?
- [ ] Can user submit broken configuration?
- [ ] Number inputs have min/max constraints?

#### C. KNX Address Mode Toggle
```javascript
// /data/config.js:96-109
function toggleKnxAddressMode(useTest) {
    const testDisplay = document.getElementById('test_addresses_display');
    const prodConfig = document.getElementById('production_addresses_config');
    // ... toggle logic
}
```

**Check:**
- [ ] Does toggle work correctly?
- [ ] Are production addresses hidden in test mode?
- [ ] State persisted across page loads?

---

### 10. Specific Bug Investigations

#### A. Preset Selector Issue
```javascript
// /data/script.js:96-147
presetSelector.addEventListener('change', function() {
    // Automatically set temperature
    fetch('/api/setpoint', { /* ... */ })
});
```

**Check:**
- [ ] Does preset change trigger temperature update?
- [ ] Is preset temp applied correctly?
- [ ] Race condition with manual slider changes?
- [ ] Preset display (`preset-temp`) updating?

#### B. Manual Override Timer
```javascript
// /data/script.js:314-319
if (overrideTimerElement && data.remaining_seconds !== undefined) {
    const minutes = Math.floor(data.remaining_seconds / 60);
    const seconds = data.remaining_seconds % 60;
    overrideTimerElement.textContent = `Auto-disable in: ${minutes}m ${seconds}s`;
}
```

**Check:**
- [ ] Does countdown update every 5 seconds?
- [ ] Timer accurate or drifting?
- [ ] What happens when timer reaches 0?
- [ ] Override disabled automatically?

#### C. Status Page Health Checks
```javascript
// /data/status.js:204-286
function updateSensorHealth(data) { /* ... */ }
function updateValveHealth(data) { /* ... */ }
```

**Check:**
- [ ] Are health endpoints returning data?
- [ ] Color coding (good/warning/error) working?
- [ ] Are calculations correct?
- [ ] Edge cases handled (null values, NaN)?

---

## Audit Methodology

### Phase 1: Static Code Analysis (2-3 hours)
1. **Read all JavaScript files** line-by-line
2. **Map data flows** from API to UI
3. **Identify potential bugs** (type errors, null checks, edge cases)
4. **List dead code** and unused functions
5. **Check coding patterns** for anti-patterns

### Phase 2: Dynamic Testing (2-3 hours)
1. **Load pages in browser** with DevTools open
2. **Monitor console** for errors and warnings
3. **Test all user interactions** (buttons, sliders, forms)
4. **Verify API calls** in Network tab
5. **Profile memory usage** over time
6. **Test on mobile device** or emulator

### Phase 3: Graph Debugging (1-2 hours)
1. **Verify Chart.js loading** - Check Network tab
2. **Inspect canvas element** - Check DOM
3. **Test API endpoint** - `curl /api/history?maxPoints=0`
4. **Log data structure** - `console.log(historyData)`
5. **Validate timestamps** - Check format and values
6. **Test filtering logic** - Isolated function test
7. **Minimal chart test** - Create simple test chart
8. **Browser compatibility** - Test in Chrome, Firefox, Safari

### Phase 4: Integration Testing (1 hour)
1. **Test full user flows:**
   - Set temperature → verify API call → check feedback
   - Change preset → verify setpoint updates
   - Enable manual override → verify timer starts
   - Reboot device → verify confirmation and execution
   - Import/export config → verify data integrity

### Phase 5: Security Audit (1 hour)
1. **Test XSS:** Try injecting `<script>alert('XSS')</script>` in inputs
2. **Test CSRF:** Send POST requests from external site
3. **Test injection:** Try SQL/command injection in config fields
4. **Check HTTPS:** Verify secure connection on production
5. **Review secrets:** Ensure no hardcoded credentials

---

## Expected Deliverables

### 1. Bug Report
**Format:**
```markdown
## Critical Bugs
1. **Graph Not Rendering**
   - Severity: HIGH
   - Location: /data/script.js:451
   - Issue: Canvas element is null
   - Root Cause: DOMContentLoaded fires before Chart.js loads
   - Fix: Move Chart.js script before script.js or add load event listener

2. **Memory Leak in Intervals**
   - Severity: MEDIUM
   - Location: /data/script.js:390,393,578
   - Issue: Intervals never cleared
   - Root Cause: No cleanup on page unload
   - Fix: Add window.addEventListener('beforeunload', clearIntervals)
```

### 2. Code Quality Report
- Unused functions and dead code
- Inconsistent coding patterns
- Missing error handling
- Performance bottlenecks

### 3. Security Findings
- Vulnerabilities found (if any)
- Recommendations for hardening
- Best practices to implement

### 4. Recommendations
**High Priority:**
- Fix critical bugs preventing functionality
- Implement proper memory management
- Add comprehensive error handling

**Medium Priority:**
- Remove dead code
- Improve mobile UX
- Add loading states

**Low Priority:**
- Code refactoring for maintainability
- Consider Preact migration (per existing docs)
- Implement progressive enhancement

---

## Graph Debugging Decision Tree

```
Graph not showing?
│
├─ Is Chart.js loaded?
│  ├─ NO → Check CDN URL, network errors
│  └─ YES ↓
│
├─ Does canvas element exist?
│  ├─ NO → Check HTML, element ID
│  └─ YES ↓
│
├─ Does /api/history return data?
│  ├─ NO → Check backend, API endpoint
│  └─ YES ↓
│
├─ Is data structure valid?
│  ├─ NO → Fix data format, add validation
│  └─ YES ↓
│
├─ Does filterDataByTimeRange return data?
│  ├─ NO → Check filtering logic, timestamps
│  └─ YES ↓
│
├─ Are labels and datasets populated?
│  ├─ NO → Check data mapping, toLocaleTimeString
│  └─ YES ↓
│
├─ Does Chart constructor throw error?
│  ├─ YES → Check console, Chart.js config
│  └─ NO ↓
│
└─ Chart exists but not visible?
   └─ Check CSS (display, visibility, opacity, z-index, canvas size)
```

---

## Testing Checklist

### Manual Testing
- [ ] Dashboard loads without errors
- [ ] All sensor readings display correctly
- [ ] Temperature slider works
- [ ] Set temperature button functional
- [ ] Preset selector changes temperature
- [ ] Manual override toggle works
- [ ] Override timer counts down
- [ ] Refresh button updates data
- [ ] **Graph displays (ALL TIME RANGES)**
- [ ] Time range buttons switch views
- [ ] Graph auto-refreshes every 5 minutes
- [ ] Status page loads
- [ ] Health indicators show correct states
- [ ] Config page tabs work
- [ ] Config save/load functional
- [ ] Reboot/reset have confirmations
- [ ] Logs page loads and refreshes
- [ ] Mobile layout works
- [ ] Touch interactions smooth

### Automated Testing (if applicable)
- [ ] Unit tests for filtering functions
- [ ] Integration tests for API calls
- [ ] E2E tests for critical user flows

---

## Priority Focus Areas

Based on user report, prioritize in this order:

1. **CRITICAL: Graph not displaying**
   - 40% of audit time
   - Must identify root cause
   - Test on multiple browsers/devices

2. **HIGH: Frontend failures**
   - 30% of audit time
   - JavaScript errors in console
   - Failed API calls
   - Broken user interactions

3. **MEDIUM: Code quality**
   - 20% of audit time
   - Memory leaks
   - Dead code
   - Performance issues

4. **LOW: Future improvements**
   - 10% of audit time
   - Mobile UX enhancements
   - Security hardening
   - Preact migration notes

---

## Tools and Resources

### Browser DevTools
- **Console:** Errors, warnings, logs
- **Network:** API calls, response times
- **Sources:** Breakpoints, step debugging
- **Memory:** Heap snapshots, allocation profiling
- **Performance:** Recording, frame rate, scripting time

### External Tools
- **Lighthouse:** Performance and best practices audit
- **WebPageTest:** Load time analysis
- **BrowserStack:** Cross-browser testing
- **Chrome DevTools Device Mode:** Mobile simulation

### Code Quality Tools
- **ESLint:** JavaScript linting
- **JSHint:** Alternative linter
- **SonarQube:** Code quality analysis

---

## Graph Removal Consideration

If graph proves unfixable or too problematic, document:
1. **Reasons for removal**
2. **Alternative solutions** (link to external Grafana/InfluxDB)
3. **Code cleanup required**
4. **User impact assessment**

However, attempt comprehensive fix BEFORE recommending removal.

---

## Conclusion

This audit should result in:
- **Identified root cause** of graph failure
- **List of all bugs** found in frontend
- **Actionable fixes** prioritized by severity
- **Code quality improvements** for maintainability
- **Security recommendations** for hardening

The frontend is relatively small (~1,900 lines total), so a thorough audit is achievable in 6-8 hours.

---

**End of Audit Prompt**
