# Thermostat Calibration Guide

## Overview

This guide outlines a systematic approach to calibrate the ESP32 KNX Thermostat against a known working reference thermostat. The goal is to align the ESP32's PID control output (virtual valve position) with actual valve positions from a validated system.

## Data Collection Strategy

### Required Data Points

Log the following metrics every 10 seconds (aligned with PID update cycle):

1. **Timestamp** - synchronized between both systems
2. **Temperature readings:**
   - ESP32 BME280 reading
   - Reference thermostat reading (if accessible)
   - Delta between them (sensor calibration check)
3. **Setpoint** - should be identical on both systems
4. **Valve positions:**
   - ESP32 calculated position (0-100%)
   - Actual valve position from working thermostat
   - **Difference** (this is your key calibration metric)
5. **Error signal** - (setpoint - current temp)
6. **Rate of change** - how fast temperature is moving
7. **Operating mode** - which preset, manual, etc.
8. **Environmental context:**
   - Time of day
   - Outdoor temperature (if available)
   - Day of week (occupancy patterns)

## Monitoring Approach

### Visualization Dashboard

Create a comparison view (could use the existing History page) showing:

- **Dual valve position chart** - overlay ESP32 vs actual on same graph
- **Temperature tracking** - current temp vs setpoint
- **Valve position error** - real-time difference between actual and virtual
- **Error distribution histogram** - to see patterns
- **PID component breakdown** - P, I, D terms individually (if exposed)

### Data Export

- Log to MQTT for Home Assistant visualization
- Store locally to CSV/JSON for detailed analysis
- Keep at least 2-4 weeks of data across different scenarios

## Calibration Scenarios

Collect data across different operating conditions to ensure robust calibration:

### 1. Warm-up Phase (Cold Start)

- Room 3-5°C below setpoint
- Large error, valve should be near 100%
- **Tests:** proportional response, integral buildup

### 2. Steady-State Maintenance

- Temperature near setpoint (±0.5°C)
- Valve modulating to maintain
- **Tests:** fine control, deadband behavior, integral term

### 3. Cool-Down

- Setpoint lowered 2-3°C
- Valve closes, passive cooling
- **Tests:** how quickly system backs off

### 4. Step Changes

- Preset switches (eco → comfort)
- Manual setpoint adjustments
- **Tests:** transient response, overshoot/undershoot

### 5. Disturbances

- Window opening
- Sunlight through window
- Occupancy changes
- **Tests:** disturbance rejection

## Analysis Metrics

### Primary Metric

**Mean Absolute Error (MAE)** between valve positions:

```
MAE = avg(|actual_valve - esp32_valve|)
```

- **Target:** < 5% for good calibration
- **Acceptable:** < 10%

### Secondary Metrics

- **RMSE** - penalizes larger errors more
- **Correlation coefficient** - how well they track together
- **Lag time** - does ESP32 respond slower/faster?
- **Overshoot comparison** - does one overshoot more?
- **Settling time** - how long to stabilize after change

### Comfort Metrics

- Temperature stability (standard deviation when at setpoint)
- Time in comfort band (±0.5°C from setpoint)
- Maximum deviation from setpoint

### Efficiency Metrics

- Total valve-time (integrate valve position over time)
- Energy wasted on overshooting

## Calibration Process

### Phase 1: Baseline (Week 1)

- Run both systems in parallel with current settings
- Collect data across all scenarios
- **Don't change anything yet**
- Analyze: Where do they differ most?

### Phase 2: Pattern Analysis

Ask yourself:

- Is ESP32 too aggressive (overshoots) or too sluggish (slow response)?
- Does it oscillate around setpoint?
- Is there steady-state error?
- Does the error change with temperature differential?

### Phase 3: PID Tuning

Based on patterns observed:

#### If ESP32 valve position is consistently higher:

- ESP32 is being too aggressive
- **Action:** Reduce Kp (proportional gain)
- Check if sensor reads colder than reality

#### If ESP32 valve position is consistently lower:

- ESP32 is too conservative
- **Action:** Increase Kp
- Check if sensor reads warmer than reality

#### If ESP32 oscillates (valve swinging up/down):

- **Actions:**
  - Reduce Kp
  - Increase Kd (derivative term) to dampen
  - Check deadband setting

#### If ESP32 has steady-state offset:

- Temperature settles but not at setpoint
- **Actions:**
  - Increase Ki (integral term)
  - Watch for integral windup

### Phase 4: Iterative Testing

- Change **ONE parameter at a time**
- Wait 2-3 days for data across different conditions
- Compare new MAE to baseline
- Keep change if improved, revert if worse
- Document everything

## Practical Implementation

### Quick Win - Use Existing Infrastructure

#### 1. MQTT Publishing

Publish both valve positions to separate topics:

```
esp32_thermostat/valve_esp32
esp32_thermostat/valve_actual  # manually publish from reference
```

#### 2. Home Assistant Dashboard

Create comparison graph:

```yaml
- type: history-graph
  entities:
    - sensor.valve_esp32
    - sensor.valve_actual
```

#### 3. CSV Export

Add a data logging endpoint:

```
GET /api/calibration-log
Returns: timestamp, temp, setpoint, valve_esp32, valve_actual, error
```

#### 4. Add Valve Difference Metric

Show real-time on dashboard: `|actual - esp32|`

Color coded:
- Green: < 5%
- Yellow: < 10%
- Red: > 10%

## Red Flags to Watch For

- **Runaway heating:** ESP32 valve stays at 100% past setpoint
- **Oscillation:** Valve swinging rapidly, temperature unstable
- **Thermal shock:** Large rapid changes uncomfortable even if mathematically correct
- **Sensor drift:** If temperature difference between sensors grows over time
- **Time-of-day patterns:** May need different tuning for night vs day

## Advanced: Automated Tuning

Once you have good data, you could:

### 1. Use Existing Auto-tune

Run auto-tune but validate against actual valve behavior

### 2. Supervised Learning

Train a correction model:

- **Input:** ESP32 calculated valve position + context (temp, error, time of day)
- **Output:** Correction factor to match actual valve
- Simple linear regression might work

### 3. Transfer Function Identification

Model the room as a system, optimize PID to that model

## Recommended Timeline

- **Week 1:** Baseline data collection
- **Week 2:** First PID adjustment based on analysis
- **Week 3:** Second iteration
- **Week 4:** Validation across full week including weekend
- **Ongoing:** Monitor for seasonal drift, outdoor temp correlation

## Data Collection Checklist

Before starting calibration:

- [ ] Both thermostats set to identical setpoints
- [ ] Heating schedules aligned
- [ ] Data logging configured (MQTT/CSV)
- [ ] Visualization dashboard ready
- [ ] Reference valve position accessible
- [ ] Baseline PID parameters documented
- [ ] Calendar marked for 4-week monitoring period

## Notes and Observations

Use this section to document your calibration journey:

### Week 1 Observations:

```
[Date]:
[Conditions]:
[Notes]:
```

### PID Parameter Changes:

```
[Date]:
[Parameter Changed]: Kp/Ki/Kd
[Old Value]:
[New Value]:
[Reason]:
[Result after 48-72h]:
```

### Environmental Factors:

```
[Weather impacts]:
[Occupancy patterns]:
[Other disturbances]:
```
