# Persistent Storage and Configuration Management PRD

## Overview
Implement persistent storage for PID controller parameters and system state to maintain configuration across device reboots, along with a web interface for parameter management.

## Problem Statement
Currently, the ESP32-KNX-Thermostat resets to default PID parameters and setpoint temperature upon reboot, losing important tuned values and system state. This results in suboptimal performance until the system re-adapts.

## Goals
- Maintain PID parameters and system state across device reboots
- Provide web interface for viewing and modifying PID parameters
- Separate data persistence logic from web interface implementation
- Ensure thread-safe access to stored parameters

## Technical Design

### 1. Persistent Storage Manager

#### Component: `PersistenceManager`
- Utilizes ESP32's Preferences library for non-volatile storage
- Manages saving/loading of:
  - PID coefficients (Kp, Ki, Kd)
  - Setpoint temperature
  - Adaptation parameters
  - Last valve position
  - System performance metrics

#### Storage Schema
```
Namespace: thermostat_config
Keys:
- pid_kp: float
- pid_ki: float
- pid_kd: float
- setpoint_temp: float
- adaptation_rate: float
- adaptation_enabled: uint8_t
- deadband: float
- last_valve_pos: float
```

### 2. Web Interface Enhancement

#### New API Endpoints
- GET `/api/pid/config` - Retrieve current PID configuration
- POST `/api/pid/config` - Update PID parameters
- GET `/api/pid/status` - Get current PID performance metrics

#### Web UI Components
- New "Configuration" tab in web interface
- PID parameter input fields with validation
- Real-time parameter update feedback
- Performance metrics display

### 3. Integration Points

#### Startup Sequence
1. Initialize PersistenceManager
2. Load stored parameters
3. Initialize PID controller with stored values
4. Start web server

#### Parameter Update Flow
1. Web interface sends update request
2. Validate parameters
3. Update PID controller
4. Store new values
5. Return success/failure response

## Security Considerations
- Implement parameter validation to prevent invalid values
- Add authentication for parameter modification
- Ensure thread-safe access to stored parameters

## Performance Requirements
- Parameter save operation < 100ms
- Web interface response time < 500ms
- Minimal impact on control loop timing

## Testing Requirements
- Verify parameter persistence across reboots
- Test parameter validation
- Verify web interface functionality
- Load testing of parameter updates
- Power loss recovery testing

## Future Enhancements
- Backup/restore configuration
- Parameter presets for different scenarios
- Automatic parameter optimization history
- Remote configuration via MQTT/KNX