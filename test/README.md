# ESP32 KNX Thermostat - Test Suite

Comprehensive unit test suite for the ESP32 KNX Thermostat project using the Unity testing framework.

## Overview

This test suite provides extensive coverage of business logic, algorithms, and data structures without requiring actual ESP32 hardware. Tests run on native (x86) platform using mocked hardware dependencies.

## Test Structure

```
test/
├── mocks/                      # Mock implementations of hardware dependencies
│   ├── Arduino.h/cpp           # Arduino framework mock
│   ├── MockPreferences.h       # ESP32 NVS storage mock
│   ├── WiFi.h                  # WiFi connectivity mock
│   ├── PubSubClient.h          # MQTT client mock
│   ├── Adafruit_BME280.h       # Temperature sensor mock
│   ├── WebServer.h             # Async web server mock
│   ├── esp-knx-ip.h            # KNX protocol mock
│   ├── SPIFFS.h                # Filesystem mock
│   ├── logger.h                # Logging mock
│   └── ntp_manager.h           # NTP time sync mock
│
├── test_adaptive_pid/          # PID Controller tests (HIGH PRIORITY)
│   └── test_pid_controller.cpp # 30+ tests covering PID algorithms
│
├── test_config_manager/        # Configuration Manager tests (HIGH PRIORITY)
│   └── test_config_manager.cpp # 40+ tests covering JSON, validation, storage
│
├── test_history_manager/       # History Manager tests (MEDIUM PRIORITY)
│   └── test_history_manager.cpp # 30+ tests covering circular buffer operations
│
├── test_sensor_health/         # Sensor Health Monitor tests (MEDIUM PRIORITY)
│   └── test_sensor_health_monitor.cpp # 25+ tests covering failure detection
│
└── test_valve_health/          # Valve Health Monitor tests (MEDIUM PRIORITY)
    └── test_valve_health_monitor.cpp # 30+ tests covering valve tracking
```

## Test Coverage Targets

| Component | Priority | Tests | Coverage Target |
|-----------|----------|-------|-----------------|
| Adaptive PID Controller | HIGH | 30+ | 80% |
| Config Manager | HIGH | 40+ | 70% |
| History Manager | MEDIUM | 30+ | 90% |
| Sensor Health Monitor | MEDIUM | 25+ | 70% |
| Valve Health Monitor | MEDIUM | 30+ | 70% |

**Total: 155+ unit tests**

## Running Tests

### Prerequisites

- PlatformIO Core installed
- Python 3.7+

### Run All Tests

```bash
pio test --environment native
```

### Run Specific Test Suite

```bash
# PID Controller tests
pio test --environment native --filter test_adaptive_pid

# Config Manager tests
pio test --environment native --filter test_config_manager

# History Manager tests
pio test --environment native --filter test_history_manager

# Sensor Health tests
pio test --environment native --filter test_sensor_health

# Valve Health tests
pio test --environment native --filter test_valve_health
```

### Verbose Output

```bash
pio test --environment native --verbose
```

## Test Suites

### 1. Adaptive PID Controller (test_adaptive_pid/)

Tests the self-tuning PID controller for temperature regulation.

**Test Suites:**
- **Basic PID Calculation**: Proportional, integral, derivative terms
- **Deadband Functionality**: ±0.2°C deadband behavior
- **Anti-Windup Protection**: Integral term limiting
- **Output Clamping**: 0-100% valve position limits
- **Error Handling**: NaN, Infinity, out-of-range inputs
- **Temperature History**: 300-sample circular buffer
- **Auto-Tuning**: Ziegler-Nichols parameter adaptation
- **Performance Analysis**: Rise time, overshoot, settling time
- **Setpoint Changes**: Controller response to target changes
- **Adaptation Logic**: Self-tuning parameter adjustments

**Key Test Cases:**
- Proportional term: `error × Kp`
- Integral accumulation with anti-windup
- Derivative on measurement (prevents derivative kick)
- Deadband prevents oscillation around setpoint
- Auto-tuning from oscillating temperature data
- Performance metrics from history analysis

### 2. Configuration Manager (test_config_manager/)

Tests persistent configuration storage and JSON serialization.

**Test Suites:**
- **Initialization**: Singleton pattern, default values
- **Getters/Setters**: WiFi, MQTT, KNX, PID, timing parameters
- **JSON Export/Import**: Full configuration backup/restore
- **Validation**: MQTT port, KNX addresses, PID parameters
- **Precision Rounding**: Kp (2 decimals), Ki/Kd (3 decimals)
- **Diagnostic Settings**: Reboot tracking, watchdog counters
- **Factory Reset**: Clear all stored preferences
- **Edge Cases**: Boundaries, empty values, long strings

**Key Test Cases:**
- Export/import round-trip preserves all settings
- Invalid MQTT port (>65535) rejected
- Invalid KNX area (>15) rejected
- Invalid setpoint (<5°C or >30°C) rejected
- PID parameters rounded to correct precision

### 3. History Manager (test_history_manager/)

Tests 24-hour circular buffer for sensor data (288 samples, 5-minute intervals).

**Test Suites:**
- **Basic Operations**: Add, retrieve, clear data points
- **Circular Buffer**: 288-sample capacity, wraparound behavior
- **Data Storage**: Temperature, humidity, pressure, valve position, timestamp
- **JSON Export**: Array format for web dashboard
- **Edge Cases**: NaN values, extreme temperatures, buffer overflow
- **Time Series**: Timestamp consistency, NTP fallback

**Key Test Cases:**
- Buffer holds exactly 288 samples
- Oldest data overwritten after wraparound
- JSON export includes all data arrays
- Timestamps increment correctly (5-minute intervals)
- Handles multiple wraparounds correctly

### 4. Sensor Health Monitor (test_sensor_health/)

Tests BME280 sensor failure detection and tracking.

**Test Suites:**
- **Failure Tracking**: Consecutive failures, total failures, failure rate
- **Health Status**: Healthy/unhealthy determination
- **Last Good Value**: Fallback data for sensor failures
- **Recovery Detection**: Track sensor recovery after failures
- **History Buffer**: 300-sample circular buffer for failure rate
- **Edge Cases**: NaN, Infinity, extreme values

**Key Test Cases:**
- Consecutive failures increment counter
- Successful reading resets consecutive count
- Failure rate calculated from last 300 samples
- Last good value not updated on failure
- Recovery detection triggers once

### 5. Valve Health Monitor (test_valve_health/)

Tests valve actuator health by comparing commanded vs actual position.

**Test Suites:**
- **Position Tracking**: Commanded position, actual feedback
- **Error Calculation**: Absolute deviation percentage
- **Stuck Detection**: Large deviations, consecutive stuck events
- **Error Statistics**: Average error, maximum error
- **History Buffer**: 100-sample sliding window
- **Recovery Detection**: Track valve recovery from stuck condition
- **Thresholds**: 10% warning, 20% critical

**Key Test Cases:**
- Error = |commanded - actual|
- Consecutive large errors (>20%) mark valve as stuck
- Average error calculated from last 100 samples
- Maximum error tracked across history
- Recovery triggers when valve unsticks

## Mock Framework

The mock framework provides in-memory replacements for hardware dependencies:

### Arduino.h
- Mock `millis()`, `micros()`, `delay()` using controllable global variables
- Arduino types: `String`, `boolean`, `byte`
- Math functions: `constrain()`, `map()`
- Serial mock for logging

### MockPreferences.h
- In-memory key-value storage using `std::map`
- All ESP32 Preferences data types supported
- Identical API to real Preferences class

### WiFi.h
- Mock WiFi connection status and RSSI
- Configurable IP addresses
- Test control methods: `setMockStatus()`, `setMockRSSI()`

### PubSubClient.h
- MQTT publish/subscribe simulation
- Message storage for verification
- Test control: `simulateMessage()`, `wasPublished()`

### Adafruit_BME280.h
- Controllable temperature, humidity, pressure readings
- Failure simulation with `setMockShouldFail()`
- Test control: `setMockTemperature()`, etc.

### logger.h
- Silent logging mock (counts messages only)
- LOG_E, LOG_W, LOG_I, LOG_D, LOG_V macros

## Unity Assertions

Common assertion macros:
- `TEST_ASSERT_TRUE(condition)`
- `TEST_ASSERT_FALSE(condition)`
- `TEST_ASSERT_EQUAL_INT(expected, actual)`
- `TEST_ASSERT_EQUAL_UINT32(expected, actual)`
- `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)`
- `TEST_ASSERT_EQUAL_STRING(expected, actual)`
- `TEST_ASSERT_NOT_NULL(pointer)`

## Continuous Integration

Tests run automatically on every push via GitHub Actions:

```yaml
- name: Run unit tests
  run: pio test --environment native --verbose
```

The CI workflow validates:
- ✅ All tests pass
- ✅ Native build compiles successfully
- ✅ No mock implementation issues

## Troubleshooting

### Tests Fail to Compile

Check that `platformio.ini` includes:
```ini
[env:native]
platform = native
build_flags =
    -std=gnu++11
    -I include
    -I test/mocks
    -D UNIT_TEST
    -D NATIVE_BUILD
    -D ARDUINO=100
lib_deps =
    throwtheswitch/Unity@^2.5.2
    bblanchon/ArduinoJson @ ^6.21.3
test_framework = unity
test_build_src = yes
```

### Mock Not Found

Ensure mock headers are in `test/mocks/` and included before real headers via `-I test/mocks` flag.

### Linker Errors

Check that `test_build_src = yes` is set in `platformio.ini` to include source files.

## Resources

- [Unity Testing Framework](https://github.com/ThrowTheSwitch/Unity)
- [PlatformIO Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html)
- [ArduinoJson Documentation](https://arduinojson.org/)

---

**Test Suite Version**: 1.0
**Last Updated**: 2025-11-11
**Total Tests**: 155+
**Framework**: Unity 2.5.2
