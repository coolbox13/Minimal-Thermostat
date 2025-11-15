# Graph Visualization Refactor - Implementation Guide

**Target:** Preact Frontend Refactoring
**Approach:** Option 1 - Adaptive Multi-Resolution Time Windows
**Date:** 2025-11-15

---

## Overview

Implement an adaptive multi-resolution time-series visualization for 2880 historical datapoints across 3 data sources (temperature, humidity, valve position). The system uses intelligent downsampling based on the selected time range to maintain performance while preserving data fidelity.

---

## Current State

### Data Structure
- **Backend:** Circular buffer storing 2880 datapoints (24 hours at 30-second intervals)
- **API Endpoint:** `/api/history?maxPoints=<n>` (maxPoints=0 returns all data)
- **Data Sources:**
  - Temperature (°C) - Primary Y-axis (left)
  - Humidity (%) - Secondary Y-axis (right)
  - Valve Position (0-100%) - Secondary Y-axis (right)

### Current Issues
1. All 2880 points transferred to client regardless of time range selected
2. No intelligent downsampling - either all points or simple skip-based sampling
3. Chart.js rendering all points even for 1-hour view
4. Performance degrades with full dataset

---

## Target Implementation

### Resolution Strategy

| Time Range | Target Points | Strategy | Rationale |
|------------|--------------|----------|-----------|
| **1 hour** | ~120 points | Full resolution (every point) | Only ~120 points, no need to downsample |
| **4 hours** | 250-300 points | LTTB downsampling | Preserve trends while reducing load |
| **12 hours** | 250-300 points | LTTB downsampling | Balanced detail vs performance |
| **24 hours** | 300-400 points | LTTB downsampling | Show full day without overwhelming chart |

### Why LTTB (Largest-Triangle-Three-Buckets)?
- Preserves peaks, valleys, and inflection points
- Better than simple averaging or skip-based sampling
- Maintains visual fidelity of the original data
- Industry standard for time-series downsampling (used by Grafana, Prometheus, etc.)

---

## Preact Component Architecture

### Component Structure

```
<GraphContainer>
  ├── <TimeRangeSelector>  (1h/4h/12h/24h buttons)
  ├── <DataPointsBadge>    ("Showing X of 2880 points")
  └── <HistoryChart>       (Chart component with downsampled data)
```

### Component Responsibilities

#### `<GraphContainer>` (Parent)
- Fetch data from `/api/history?maxPoints=0`
- Manage selected time range state
- Store raw 2880 datapoints
- Pass downsampled data to `<HistoryChart>`

#### `<TimeRangeSelector>`
- Render time range buttons (1h, 4h, 12h, 24h)
- Highlight active selection
- Callback to parent on selection change

#### `<DataPointsBadge>`
- Display transparency indicator: "Showing 287 of 2880 points"
- Optional: Show downsampling method (e.g., "LTTB algorithm")

#### `<HistoryChart>`
- Receive downsampled data as props
- Render chart using chosen library (Chart.js, uPlot, or Recharts)
- Handle responsive sizing
- Display 3 datasets with dual Y-axes

---

## Implementation Details

### 1. Data Fetching

```javascript
import { useState, useEffect } from 'preact/hooks';

const GraphContainer = () => {
  const [rawData, setRawData] = useState(null);
  const [timeRange, setTimeRange] = useState(24); // hours
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch('/api/history?maxPoints=0');
        const data = await response.json();
        setRawData(data);
      } catch (error) {
        console.error('Failed to fetch history:', error);
      } finally {
        setLoading(false);
      }
    };

    fetchData();
    // Refresh every 30 seconds to get new datapoints
    const interval = setInterval(fetchData, 30000);
    return () => clearInterval(interval);
  }, []);

  // ... rest of component
};
```

### 2. Time-Based Filtering

```javascript
import { useMemo } from 'preact/hooks';

const filterByTimeRange = (data, hours) => {
  if (!data || !data.timestamps) return null;

  const nowSeconds = Math.floor(Date.now() / 1000);
  const cutoffTime = nowSeconds - (hours * 3600);

  const indices = [];
  for (let i = 0; i < data.timestamps.length; i++) {
    if (data.timestamps[i] >= cutoffTime) {
      indices.push(i);
    }
  }

  return {
    timestamps: indices.map(i => data.timestamps[i]),
    temperatures: indices.map(i => data.temperatures[i]),
    humidities: indices.map(i => data.humidities[i]),
    valvePositions: indices.map(i => data.valvePositions[i]),
    count: indices.length
  };
};
```

### 3. LTTB Downsampling Implementation

**Install dependency:**
```bash
npm install downsample
```

**Or implement LTTB manually:**

```javascript
/**
 * Largest-Triangle-Three-Buckets downsampling algorithm
 * @param {Array} data - Array of {x: timestamp, y: value} points
 * @param {number} threshold - Target number of points
 * @returns {Array} Downsampled data
 */
const lttb = (data, threshold) => {
  if (threshold >= data.length || threshold === 0) {
    return data; // Nothing to do
  }

  const sampled = [];
  const bucketSize = (data.length - 2) / (threshold - 2);

  let a = 0;
  sampled.push(data[a]); // Always add first point

  for (let i = 0; i < threshold - 2; i++) {
    let avgX = 0;
    let avgY = 0;
    let avgRangeStart = Math.floor((i + 1) * bucketSize) + 1;
    let avgRangeEnd = Math.floor((i + 2) * bucketSize) + 1;
    avgRangeEnd = avgRangeEnd < data.length ? avgRangeEnd : data.length;

    const avgRangeLength = avgRangeEnd - avgRangeStart;

    for (; avgRangeStart < avgRangeEnd; avgRangeStart++) {
      avgX += data[avgRangeStart].x;
      avgY += data[avgRangeStart].y;
    }
    avgX /= avgRangeLength;
    avgY /= avgRangeLength;

    let rangeOffs = Math.floor((i + 0) * bucketSize) + 1;
    const rangeTo = Math.floor((i + 1) * bucketSize) + 1;

    const pointAX = data[a].x;
    const pointAY = data[a].y;

    let maxArea = -1;
    let maxAreaPoint = null;

    for (; rangeOffs < rangeTo; rangeOffs++) {
      const area = Math.abs(
        (pointAX - avgX) * (data[rangeOffs].y - pointAY) -
        (pointAX - data[rangeOffs].x) * (avgY - pointAY)
      ) * 0.5;

      if (area > maxArea) {
        maxArea = area;
        maxAreaPoint = rangeOffs;
      }
    }

    sampled.push(data[maxAreaPoint]);
    a = maxAreaPoint;
  }

  sampled.push(data[data.length - 1]); // Always add last point

  return sampled;
};
```

### 4. Downsampling Logic with Memoization

```javascript
import { useMemo } from 'preact/hooks';

const GraphContainer = () => {
  // ... state and data fetching ...

  const processedData = useMemo(() => {
    if (!rawData) return null;

    // Step 1: Filter by time range
    const filtered = filterByTimeRange(rawData, timeRange);
    if (!filtered) return null;

    // Step 2: Determine if downsampling is needed
    const targetPoints = getTargetPoints(timeRange);

    if (filtered.count <= targetPoints) {
      // No downsampling needed
      return {
        ...filtered,
        downsampledCount: filtered.count,
        originalCount: rawData.count
      };
    }

    // Step 3: Downsample each dataset independently
    const timestamps = filtered.timestamps;

    const tempData = timestamps.map((t, i) => ({
      x: t,
      y: filtered.temperatures[i]
    }));
    const humidData = timestamps.map((t, i) => ({
      x: t,
      y: filtered.humidities[i]
    }));
    const valveData = timestamps.map((t, i) => ({
      x: t,
      y: filtered.valvePositions[i]
    }));

    const sampledTemp = lttb(tempData, targetPoints);
    const sampledHumid = lttb(humidData, targetPoints);
    const sampledValve = lttb(valveData, targetPoints);

    // Note: After LTTB, each dataset might have slightly different timestamps
    // For multi-line charts, you may need to align them or use the temperature
    // timestamps as the primary axis

    return {
      timestamps: sampledTemp.map(d => d.x),
      temperatures: sampledTemp.map(d => d.y),
      humidities: sampledHumid.map(d => d.y),
      valvePositions: sampledValve.map(d => d.y),
      downsampledCount: sampledTemp.length,
      originalCount: rawData.count
    };

  }, [rawData, timeRange]);

  const getTargetPoints = (hours) => {
    switch(hours) {
      case 1: return 120;   // Full resolution
      case 4: return 275;
      case 12: return 275;
      case 24: return 350;
      default: return 300;
    }
  };

  // ... render components ...
};
```

### 5. Chart Library Options

#### **Option A: Chart.js** (Current - easiest migration)
```bash
npm install chart.js preact-chartjs
```

Pros: Already familiar, good documentation
Cons: Heavier bundle size, less performant with large datasets

#### **Option B: uPlot** (Recommended for performance)
```bash
npm install uplot preact-uplot
```

Pros: Extremely fast, small bundle (~40kb), designed for time-series
Cons: Less features, more manual configuration

#### **Option C: Recharts** (Best Preact integration)
```bash
npm install recharts
```

Pros: Declarative API, great with React/Preact, responsive out of box
Cons: Medium bundle size, slower than uPlot

**Recommendation:** Use **uPlot** for best performance and future-proofing.

---

## UI/UX Specifications

### Time Range Selector
- **Layout:** Horizontal button group above chart
- **Buttons:** 1h | 4h | 12h | 24h
- **Active state:** Highlighted/different color
- **Default:** 24h selected

### Data Points Badge
- **Position:** Top-right corner of chart or below time range selector
- **Format:** "Showing 287 of 2,880 points"
- **Style:** Subtle, small text, low contrast (informational, not prominent)
- **Tooltip (optional):** "Downsampled using LTTB algorithm for performance"

### Chart Axes
- **X-axis:** Time (formatted based on range - show hours for 1h/4h, show date for 12h/24h)
- **Left Y-axis:** Temperature (°C) - Primary dataset
- **Right Y-axis:** Humidity (%) and Valve Position (%)
- **Grid lines:** Light, subtle
- **Legend:** Show all 3 datasets with color coding

### Responsive Behavior
- **Mobile:** Stack time range buttons if needed, adjust chart height
- **Tablet/Desktop:** Full horizontal layout
- **Touch:** Support touch gestures for chart interaction if using uPlot

---

## Data Flow Diagram

```
┌─────────────────┐
│  ESP32 Device   │
│  (30s interval) │
└────────┬────────┘
         │ addDataPoint()
         ▼
┌─────────────────────────┐
│  HistoryManager         │
│  Circular Buffer (2880) │
└────────┬────────────────┘
         │ GET /api/history?maxPoints=0
         ▼
┌─────────────────────────┐
│  Preact: GraphContainer │
│  - Store raw 2880 pts   │
│  - Time range state     │
└────────┬────────────────┘
         │ useMemo
         ▼
┌─────────────────────────┐
│  Processing Pipeline    │
│  1. Filter by time      │
│  2. Check if > target   │
│  3. LTTB downsample     │
└────────┬────────────────┘
         │ props
         ▼
┌─────────────────────────┐
│  HistoryChart Component │
│  Render with uPlot      │
└─────────────────────────┘
```

---

## Performance Considerations

### Optimization Strategies

1. **Memoization:** Use `useMemo` for downsampling to avoid recalculation on every render
2. **Web Workers (future):** Move LTTB calculation to background thread for large datasets
3. **Incremental Updates:** When new datapoint arrives, only recalculate if visible in current time range
4. **Debounce Time Range Changes:** Prevent rapid re-renders if user clicks multiple buttons quickly

### Expected Performance

| Metric | Before | After |
|--------|--------|-------|
| Data transfer | ~140KB JSON | ~140KB (same - all data still fetched) |
| Points rendered (1h) | 120 | 120 (no change) |
| Points rendered (24h) | 2880 | ~350 (87% reduction) |
| Chart render time (24h) | ~500ms | ~50ms (estimated) |
| Memory usage | High (all points in chart) | Low (only downsampled in chart) |

**Note:** Data transfer remains the same because we still fetch all 2880 points. Future optimization could add server-side time filtering to reduce network payload.

---

## Backend Changes (Optional but Recommended)

### Add Time Range Query Parameter

Modify `/api/history` endpoint to accept `hours` parameter:

```cpp
// In web_server.cpp
server.on("/api/history", HTTP_GET, [](AsyncWebServerRequest *request) {
    int maxPoints = 0;
    int hours = 0; // 0 = all data

    if (request->hasParam("maxPoints")) {
        maxPoints = request->getParam("maxPoints")->value().toInt();
    }

    if (request->hasParam("hours")) {
        hours = request->getParam("hours")->value().toInt();
    }

    String json = historyManager->getHistoryJson(maxPoints, hours);
    request->send(200, "application/json", json);
});
```

**Benefits:**
- Reduce network payload for 1h view from 140KB to ~20KB
- Faster response times
- Less client-side processing

**Implementation complexity:** Medium (requires modifying C++ backend)

---

## Testing Checklist

### Functional Tests
- [ ] All 2880 datapoints load correctly
- [ ] Time range buttons change visible data range
- [ ] Downsampling preserves peaks and valleys
- [ ] New datapoints appear in real-time (30s refresh)
- [ ] Chart legend shows all 3 datasets
- [ ] Dual Y-axes display correctly
- [ ] Data points badge shows correct counts

### Edge Cases
- [ ] Empty buffer (device just started) - show "No data yet"
- [ ] Partial buffer (< 2880 points) - display available data
- [ ] Network error on fetch - show error message with retry button
- [ ] Time sync issues - handle timestamp discrepancies gracefully

### Performance Tests
- [ ] Chart renders in < 100ms on desktop
- [ ] Chart renders in < 300ms on mobile
- [ ] No memory leaks over 1 hour of use
- [ ] Smooth animation when switching time ranges

### Visual Tests
- [ ] Responsive on mobile (320px width)
- [ ] Readable on tablet (768px)
- [ ] Full features on desktop (1920px)
- [ ] Color contrast meets WCAG AA standards
- [ ] Time axis labels don't overlap

---

## Migration Path from Current Implementation

### Step 1: Set up Preact project structure
- Create component files
- Install dependencies (`uplot`, `downsample` or implement LTTB)

### Step 2: Port data fetching
- Move API call from `script.js` to Preact `useEffect`
- Keep existing `/api/history` endpoint

### Step 3: Implement filtering
- Port `filterDataByTimeRange()` to Preact utility function
- Add time range state management

### Step 4: Add LTTB downsampling
- Integrate LTTB algorithm
- Add memoization for performance

### Step 5: Replace Chart.js with uPlot
- Create uPlot wrapper component
- Configure dual Y-axes
- Style to match current design

### Step 6: Add UI enhancements
- Time range selector component
- Data points badge
- Loading states and error handling

### Step 7: Testing and refinement
- Cross-browser testing
- Performance profiling
- User feedback iteration

---

## Example File Structure

```
src/
├── components/
│   ├── Graph/
│   │   ├── GraphContainer.jsx          # Main container
│   │   ├── HistoryChart.jsx            # uPlot wrapper
│   │   ├── TimeRangeSelector.jsx       # Button group
│   │   ├── DataPointsBadge.jsx         # Info badge
│   │   └── graph.module.css            # Scoped styles
│   └── ...
├── utils/
│   ├── lttb.js                         # LTTB algorithm
│   ├── dataProcessing.js               # Filter/transform helpers
│   └── ...
├── hooks/
│   └── useHistoryData.js               # Custom hook for data fetching
└── ...
```

---

## Questions for Clarification

1. **Chart Library:** Confirm if migrating away from Chart.js to uPlot is acceptable?
2. **Backend Changes:** Should we implement server-side time filtering now or later?
3. **Styling:** Any specific design system or CSS framework to follow?
4. **Mobile Priority:** What's the minimum supported screen size?
5. **Real-time Updates:** Should chart auto-scroll to show latest data, or stay at current view?

---

## References

- **LTTB Algorithm:** [Original Paper by Sveinn Steinarsson](https://skemman.is/bitstream/1946/15343/3/SS_MSthesis.pdf)
- **uPlot Documentation:** https://github.com/leeoniya/uPlot
- **Current Implementation:**
  - `/home/user/Minimal-Thermostat/data/script.js` (lines 395-579)
  - `/home/user/Minimal-Thermostat/src/history_manager.cpp`

---

**Document Version:** 1.0
**Last Updated:** 2025-11-15
**Author:** Claude (Graph Visualization Specialist)
