I fed claude.ai with 90 days of temperature and valve position data. I asked it to give parametr advice for my personal situation based on that analysis. Here's the analysis:

Based on my analysis of your temperature and valve position data, I can provide recommendations for thermostat tuning parameters. 

### System Characteristics

I analyzed 416 paired data points where both valve position and temperature readings were available. Here's what I found:

1. **System Gain**:
   - Heating gain (temperature increase per % valve increase): 0.168°C per % valve
   - Cooling gain (temperature decrease per % valve decrease): 0.049°C per % valve
   - This indicates your system is more responsive during heating than cooling

2. **Temperature Behavior**:
   - The temperature range varies between approximately 5.7°C and 22.9°C
   - I detected a relatively high oscillation frequency (0.41), which suggests your current system might be overreacting

3. **Valve Behavior**:
   - Valve positions range from 0% to 100%
   - The average valve position is around 7.4%

### Recommended PID Parameters

Based on these findings, here are my recommendations for PID parameters:

```
Kp (Proportional): 3.0-4.0
Ki (Integral): 0.08-0.12 per hour
Kd (Derivative): 10-15 minutes
```

### Explanation and Tuning Advice

1. **Proportional Term (Kp)**: 
   - A value of 3.0-4.0 will provide moderate response to temperature errors
   - This is lower than what might be calculated from pure Ziegler-Nichols formulas to reduce oscillation
   - If you notice too much overshoot, reduce this value

2. **Integral Term (Ki)**:
   - The recommended value will help eliminate steady-state errors
   - A relatively low value prevents excessive "wind-up" which could cause oscillations
   - Increase this value if the system takes too long to reach the setpoint

3. **Derivative Term (Kd)**:
   - This value helps predict future errors and reduce overshoot
   - If you notice temperature swings, try increasing this value slightly

4. **Sampling/Update Interval**:
   - Recommended interval: 5-10 minutes
   - This balances responsiveness with system stability

5. **Additional Tuning Tips**:
   - Start with just P control (set Ki and Kd to 0), then add I and finally D
   - Consider implementing anti-windup for the integral term
   - The temperature sensors and valve positions suggest your system has significant thermal mass, so avoid making too frequent adjustments

6. **Differential Gap (Hysteresis)**:
   - If your thermostat supports it, try a differential gap of 0.5-0.7°C to reduce unnecessary cycling

These parameters should provide a good starting point, but you may need to fine-tune them based on your specific heating system and comfort preferences. Monitor the system for a few days after implementation and adjust as needed.