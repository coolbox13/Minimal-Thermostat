/**
 * Data processing utilities for history data
 */

/**
 * Filter history data by time range
 *
 * @param {Object} data - History data with timestamps array
 * @param {number} hours - Number of hours to keep (from now)
 * @returns {Object} Filtered data
 */
export function filterByTimeRange(data, hours) {
  if (!data || !data.timestamps || data.timestamps.length === 0) {
    return null;
  }

  const nowSeconds = Math.floor(Date.now() / 1000);
  const cutoffTime = nowSeconds - (hours * 3600);

  // Debug: log time range info
  console.log(`[filterByTimeRange] hours=${hours}, now=${nowSeconds}, cutoff=${cutoffTime}`);
  console.log(`[filterByTimeRange] data range: ${data.timestamps[0]} - ${data.timestamps[data.timestamps.length-1]}`);
  console.log(`[filterByTimeRange] oldest data age: ${(nowSeconds - data.timestamps[0]) / 3600} hours`);

  const indices = [];
  for (let i = 0; i < data.timestamps.length; i++) {
    if (data.timestamps[i] >= cutoffTime) {
      indices.push(i);
    }
  }

  console.log(`[filterByTimeRange] found ${indices.length} points within ${hours}h range`);

  if (indices.length === 0) {
    return null;
  }

  return {
    timestamps: indices.map(i => data.timestamps[i]),
    temperatures: indices.map(i => data.temperatures[i]),
    humidities: indices.map(i => data.humidities[i]),
    pressures: indices.map(i => data.pressures ? data.pressures[i] : null),
    valvePositions: indices.map(i => data.valvePositions[i]),
    count: indices.length,
  };
}

/**
 * Get target number of points for a given time range
 * Implements the strategy from GRAPH_VISUALIZATION_REFACTOR.md
 *
 * @param {number} hours - Time range in hours
 * @returns {number} Target number of points
 */
export function getTargetPoints(hours) {
  switch (hours) {
    case 1:
      return 120;  // Full resolution for 1 hour
    case 4:
      return 275;  // LTTB downsampling
    case 12:
      return 275;  // LTTB downsampling
    case 24:
      return 350;  // LTTB downsampling
    default:
      return 300;  // Default
  }
}

/**
 * Format timestamp for display based on time range
 *
 * @param {number} timestamp - Unix timestamp in seconds
 * @param {number} timeRange - Time range in hours
 * @returns {string} Formatted time string
 */
export function formatTimestamp(timestamp, timeRange) {
  const date = new Date(timestamp * 1000);

  if (timeRange <= 4) {
    // For short ranges, show hours:minutes
    return date.toLocaleTimeString('en-US', {
      hour: '2-digit',
      minute: '2-digit',
    });
  } else {
    // For longer ranges, show date + time
    return date.toLocaleString('en-US', {
      month: 'short',
      day: 'numeric',
      hour: '2-digit',
      minute: '2-digit',
    });
  }
}

/**
 * Calculate statistics for a dataset
 *
 * @param {number[]} values - Array of numeric values
 * @returns {Object} Statistics {min, max, avg, latest}
 */
export function calculateStats(values) {
  if (!values || values.length === 0) {
    return { min: null, max: null, avg: null, latest: null };
  }

  const min = Math.min(...values);
  const max = Math.max(...values);
  const sum = values.reduce((acc, val) => acc + val, 0);
  const avg = sum / values.length;
  const latest = values[values.length - 1];

  return {
    min: min.toFixed(1),
    max: max.toFixed(1),
    avg: avg.toFixed(1),
    latest: latest.toFixed(1),
  };
}
