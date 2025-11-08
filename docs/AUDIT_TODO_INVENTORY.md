# Audit TODO Inventory
**Date:** 2025-11-08
**Project:** ESP32 KNX Thermostat
**Source:** Comprehensive Code Audit

This document consolidates all actionable items identified during the code audit. Items are organized by priority and category.

---

## PRIORITY LEGEND
- 🔴 **P0 - Critical:** Must fix, violates standards or causes issues
- 🟠 **P1 - High:** Should fix soon, impacts code quality significantly
- 🟡 **P2 - Medium:** Should fix, but not urgent
- 🟢 **P3 - Low:** Nice to have, minor improvements
- 🔵 **P4 - Optional:** Future consideration, not essential

---

## 🔴 P0 - CRITICAL PRIORITY

### Code Standards Violations

- [ ] **TODO-001:** Break down `setup()` function in main.cpp (103 lines → max 20)
  - **File:** src/main.cpp:72-175
  - **Violation:** 5x over function length limit
  - **Action:** Extract helper functions:
    - `initializeLogger()`
    - `initializeConfig()`
    - `initializeWatchdog()`
    - `initializeWiFi()`
    - `initializeWebServer()`
    - `initializeKNXAndMQTT()`
    - `initializePID()`
  - **Estimated Effort:** 2-3 hours

- [ ] **TODO-002:** Break down `AdaptivePID_Update()` function (108 lines → max 20)
  - **File:** src/adaptive_pid_controller.cpp:218-326
  - **Violation:** 5x over function length limit
  - **Action:** Extract functions:
    - `calculateError()`
    - `checkDeadband()`
    - `updateIntegralError()`
    - `calculateDerivativeError()`
    - `computePIDOutput()`
    - `updatePerformanceMetrics()`
    - `attemptAdaptation()`
  - **Estimated Effort:** 3-4 hours

- [ ] **TODO-003:** Break down `setFromJson()` in ConfigManager (192 lines → max 20)
  - **File:** src/config_manager.cpp:201-393
  - **Violation:** 9.6x over function length limit
  - **Action:** Extract validation functions:
    - `validateNetworkSettings()`
    - `validateMQTTSettings()`
    - `validateKNXSettings()`
    - `validateBME280Settings()`
    - `validatePIDSettings()`
  - **Estimated Effort:** 3-4 hours

- [ ] **TODO-004:** Simplify POST /api/config handler in web_server.cpp (107 lines)
  - **File:** src/web_server.cpp:180-287
  - **Violation:** 5x over function length limit
  - **Action:** Extract lambda to named function, break into helpers
  - **Estimated Effort:** 2 hours

### Code Cleanup

- [ ] **TODO-005:** Remove ALL commented-out code
  - **Files:**
    - src/main.cpp (lines 49-50, 94-96, 209-219)
    - include/config.h (lines 62-66)
    - src/config_manager.cpp (lines 400, 414, 428, 442)
  - **Action:** Delete commented code (it's in git history)
  - **Estimated Effort:** 30 minutes

- [ ] **TODO-006:** Fix duplicate WIFI_WATCHDOG_TIMEOUT definition
  - **File:** include/config.h:63-66
  - **Issue:**
    ```cpp
    #define WIFI_WATCHDOG_TIMEOUT 1800000
    #define WIFI_WATCHDOG_TIMEOUT_MS 1800000  // Duplicate!
    ```
  - **Action:** Remove duplicate, standardize on one name
  - **Estimated Effort:** 15 minutes

---

## 🟠 P1 - HIGH PRIORITY

### Documentation

- [ ] **TODO-007:** Add Doxygen comments to all public functions in config_manager.cpp
  - **File:** src/config_manager.cpp
  - **Missing:** Function-level documentation for all public methods
  - **Estimated Effort:** 2 hours

- [ ] **TODO-008:** Add Doxygen comments to all public functions in knx_manager.cpp
  - **File:** src/knx_manager.cpp
  - **Missing:** Comprehensive function documentation
  - **Estimated Effort:** 2 hours

- [ ] **TODO-009:** Add Doxygen comments to web_server.cpp endpoints
  - **File:** src/web_server.cpp
  - **Missing:** API endpoint documentation
  - **Estimated Effort:** 1.5 hours

- [ ] **TODO-010:** Add Doxygen comments to mqtt_manager.cpp
  - **File:** src/mqtt_manager.cpp
  - **Missing:** Function documentation
  - **Estimated Effort:** 1.5 hours

### Code Quality

- [ ] **TODO-011:** Remove blank lines within all functions
  - **Files:** All .cpp files
  - **Violation:** Project standard "No blank lines within functions"
  - **Action:** Remove blank lines that separate logic within functions
  - **Estimated Effort:** 2 hours (manual review required)

- [ ] **TODO-012:** Replace magic numbers with named constants in main.cpp
  - **File:** src/main.cpp
  - **Constants needed:**
    - `WIFI_CONNECT_TIMEOUT_SEC = 180`
    - `SENSOR_UPDATE_INTERVAL_MS = 30000`
    - `CONNECTIVITY_CHECK_INTERVAL_MS = 300000`
  - **Estimated Effort:** 1 hour

- [ ] **TODO-013:** Replace magic numbers with named constants in wifi_connection.cpp
  - **File:** src/wifi_connection.cpp
  - **Constants needed:**
    - `WIFI_CONNECT_TIMEOUT_MS = 10000`
    - `WIFI_RECONNECT_TIMEOUT_MS = 10000`
    - `INTERNET_CHECK_INTERVAL_MS = 300000`
  - **Estimated Effort:** 1 hour

- [ ] **TODO-014:** Replace magic numbers with named constants in adaptive_pid_controller.cpp
  - **File:** src/adaptive_pid_controller.cpp
  - **Constants needed:**
    - `PID_ADAPTATION_INTERVAL_SEC = 60.0f`
  - **Estimated Effort:** 30 minutes

- [ ] **TODO-015:** Consolidate PID rounding logic
  - **Files:** config_manager.cpp, web_server.cpp, adaptive_pid_controller.cpp
  - **Issue:** Duplicate rounding logic in 4+ places
  - **Action:** Create static helper in ConfigManager:
    ```cpp
    static float roundToPrecision(float value, int decimals);
    ```
  - **Estimated Effort:** 1.5 hours

### Architecture

- [ ] **TODO-016:** Remove duplicate KnxAddressChangedCallback definition
  - **File:** src/web_server.cpp:21-23
  - **Issue:** Callback defined as global instead of using class member
  - **Action:** Remove global, use header definition only
  - **Estimated Effort:** 30 minutes

- [ ] **TODO-017:** Remove unused global variable `configPortalActive`
  - **File:** src/main.cpp:53
  - **Issue:** Declared but never used
  - **Action:** Delete unused variable
  - **Estimated Effort:** 5 minutes

---

## 🟡 P2 - MEDIUM PRIORITY

### Feature Implementation (Phase 1)

- [ ] **TODO-018:** Make KNX group addresses configurable via web UI
  - **Files:** include/config.h, include/config_manager.h, src/config_manager.cpp, src/web_server.cpp, data/config.html
  - **Current:** Hardcoded in config.h
  - **Action:**
    1. Add getters/setters to ConfigManager for all KNX group addresses
    2. Add API endpoints to web_server.cpp
    3. Update web interface to display/edit addresses
    4. Store in persistent storage
  - **Estimated Effort:** 6-8 hours

- [ ] **TODO-019:** Make KNX debug flag runtime-configurable
  - **Files:** include/config.h, src/knx_manager.cpp, data/config.html
  - **Current:** Compile-time flag `KNX_DEBUG_ENABLED`
  - **Action:**
    1. Move to runtime configuration in ConfigManager
    2. Add web UI toggle
    3. Add function to enable/disable KNX logging at runtime
  - **Estimated Effort:** 2-3 hours

### Error Handling

- [ ] **TODO-020:** Standardize error handling pattern across all managers
  - **Files:** All manager classes
  - **Issue:** Inconsistent return values and error propagation
  - **Action:**
    1. Document error handling standard
    2. Choose pattern (exceptions vs return codes vs callbacks)
    3. Implement consistently
  - **Estimated Effort:** 4-6 hours

- [ ] **TODO-021:** Add validation before applying configuration changes
  - **File:** src/config_manager.cpp, src/web_server.cpp
  - **Action:**
    1. Validate all settings before applying
    2. Create backup of current config
    3. Apply changes atomically
    4. Rollback on validation failure
  - **Estimated Effort:** 3-4 hours

### Code Organization

- [ ] **TODO-022:** Consolidate Preferences namespaces
  - **Files:** src/config_manager.cpp, src/watchdog_manager.cpp
  - **Issue:** Using "thermostat", "watchdog", "config" namespaces
  - **Action:**
    1. Document namespace separation rationale
    2. Or consolidate to single namespace
  - **Estimated Effort:** 2 hours

- [ ] **TODO-023:** Standardize TAG naming across all files
  - **Files:** All .cpp files
  - **Issue:** Mixed use of `TAG` vs `TAG_MODULE` patterns
  - **Action:** Choose one pattern and apply consistently
  - **Estimated Effort:** 1.5 hours

### Performance

- [ ] **TODO-024:** Implement flash wear reduction for PID parameters
  - **File:** src/main.cpp:280-316
  - **Issue:** Writes to flash every 10 seconds if parameters changed
  - **Action:** Implement write coalescing (batch writes every 5 minutes)
  - **Estimated Effort:** 2 hours

- [ ] **TODO-025:** Optimize String allocations in logging
  - **Files:** src/wifi_connection.cpp and others
  - **Issue:** Frequent String concatenation in log messages
  - **Action:** Use snprintf() or reserve String capacity
  - **Estimated Effort:** 3-4 hours

---

## 🟢 P3 - LOW PRIORITY

### Code Quality

- [ ] **TODO-026:** Add const-correctness to getter functions
  - **Files:** All manager classes
  - **Action:** Mark all getters as const, use const& for return values
  - **Example:**
    ```cpp
    String getWifiSSID() const;
    const String& getWifiSSID() const;
    ```
  - **Estimated Effort:** 2 hours

- [ ] **TODO-027:** Extract main controller to Application class
  - **File:** src/main.cpp
  - **Issue:** Global variables and functions in main
  - **Action:** Create `ThermostatApplication` class to encapsulate state
  - **Estimated Effort:** 6-8 hours

- [ ] **TODO-028:** Document initialization order dependencies
  - **File:** src/main.cpp
  - **Action:** Add comments documenting why components must be initialized in specific order
  - **Estimated Effort:** 1 hour

### Documentation

- [ ] **TODO-029:** Complete docs/API.md
  - **File:** docs/API.md
  - **Missing:** Full REST API specification
  - **Action:** Document all endpoints with request/response examples
  - **Estimated Effort:** 4-6 hours

- [ ] **TODO-030:** Create CHANGELOG.md
  - **File:** New file
  - **Action:** Document version history and changes
  - **Estimated Effort:** 2 hours

- [ ] **TODO-031:** Create TROUBLESHOOTING.md
  - **File:** New file
  - **Action:** Document common issues and solutions
  - **Estimated Effort:** 3-4 hours

### Testing

- [ ] **TODO-032:** Add unit tests for PID controller
  - **File:** New test files
  - **Action:** Create tests for PID calculation logic
  - **Estimated Effort:** 8-12 hours

- [ ] **TODO-033:** Add unit tests for ConfigManager
  - **File:** New test files
  - **Action:** Create tests with mocked Preferences
  - **Estimated Effort:** 6-8 hours

### Monitoring

- [ ] **TODO-034:** Add heap memory monitoring
  - **Files:** src/main.cpp, new src/memory_monitor.cpp
  - **Action:**
    1. Log free heap size periodically
    2. Detect fragmentation
    3. Log low-memory warnings
  - **Estimated Effort:** 3-4 hours

- [ ] **TODO-035:** Add KNX error monitoring (non-invasive)
  - **File:** src/knx_manager.cpp
  - **Action:** Add counters for KNX errors without changing core logic
  - **Estimated Effort:** 2-3 hours

---

## 🔵 P4 - OPTIONAL / FUTURE

### Feature Implementation (Phase 2)

- [ ] **TODO-036:** Add MQTT authentication to web UI
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 2
  - **Files:** Multiple
  - **Estimated Effort:** 4-6 hours

- [ ] **TODO-037:** Make MQTT topics configurable
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 2
  - **Files:** Multiple
  - **Estimated Effort:** 6-8 hours

- [ ] **TODO-038:** Make BME280 settings configurable (address, pins, interval)
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 2
  - **Files:** Multiple
  - **Estimated Effort:** 8-10 hours

- [ ] **TODO-039:** Make watchdog timeouts configurable
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 2
  - **Files:** Multiple
  - **Estimated Effort:** 4-6 hours

- [ ] **TODO-040:** Make PID deadband and update interval configurable
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 2
  - **Files:** Multiple
  - **Estimated Effort:** 3-4 hours

- [ ] **TODO-041:** Make WiFi reconnect attempts configurable
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 2
  - **Files:** Multiple
  - **Estimated Effort:** 2-3 hours

### Feature Implementation (Phase 3)

- [ ] **TODO-042:** Make MQTT topic structure customizable
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 3
  - **Files:** Multiple
  - **Estimated Effort:** 6-8 hours

- [ ] **TODO-043:** Advanced Home Assistant integration settings
  - **Reference:** docs/Designplan Variables Thermostat.md - Phase 3
  - **Files:** Multiple
  - **Estimated Effort:** 8-12 hours

### Architecture Improvements

- [ ] **TODO-044:** Implement dependency injection for managers
  - **File:** src/main.cpp
  - **Action:** Remove global instances, inject dependencies
  - **Estimated Effort:** 12-16 hours

- [ ] **TODO-045:** Create abstract interfaces for sensors
  - **Files:** New interface files
  - **Action:** Support pluggable sensor types
  - **Estimated Effort:** 8-10 hours

### Security (Low Priority for Internal Network)

- [ ] **TODO-046:** Add basic auth for web interface
  - **Files:** src/web_server.cpp
  - **Action:** Optional authentication for defense in depth
  - **Estimated Effort:** 4-6 hours

- [ ] **TODO-047:** Add MQTT TLS support
  - **Files:** src/mqtt_manager.cpp
  - **Action:** Encrypted MQTT connections
  - **Estimated Effort:** 6-8 hours

---

## SUMMARY STATISTICS

### By Priority
- 🔴 **P0 Critical:** 6 items (~12-15 hours)
- 🟠 **P1 High:** 11 items (~25-30 hours)
- 🟡 **P2 Medium:** 8 items (~30-40 hours)
- 🟢 **P3 Low:** 9 items (~40-50 hours)
- 🔵 **P4 Optional:** 12 items (~80-100 hours)

### By Category
- **Code Standards:** 6 items
- **Documentation:** 7 items
- **Code Quality:** 10 items
- **Architecture:** 7 items
- **Features:** 13 items
- **Testing:** 2 items
- **Monitoring:** 2 items

### Recommended Focus
**Week 1-2:** Complete all P0 and half of P1 items
**Month 1:** Complete all P0, P1, and critical P2 items
**Month 2-3:** Complete remaining P2 and Phase 1 features
**Month 3+:** P3 items and Phase 2/3 features as needed

---

## TRACKING

Use this section to track progress:

### Sprint 1 (Week 1-2)
- [ ] Complete TODO-001 through TODO-006 (P0)
- [ ] Complete TODO-007 through TODO-011 (P1 Documentation)

### Sprint 2 (Week 3-4)
- [ ] Complete TODO-012 through TODO-017 (P1 Code Quality)
- [ ] Complete TODO-018 through TODO-019 (Phase 1 Features)

### Sprint 3 (Month 2)
- [ ] Complete TODO-020 through TODO-025 (P2 items)

### Backlog
- [ ] All P3 and P4 items as capacity allows

---

*End of TODO Inventory*
