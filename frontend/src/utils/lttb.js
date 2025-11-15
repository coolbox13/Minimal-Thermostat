/**
 * Largest-Triangle-Three-Buckets (LTTB) Downsampling Algorithm
 *
 * Reduces the number of data points while preserving the visual characteristics
 * of the time series. Particularly good at preserving peaks, valleys, and trends.
 *
 * Reference: https://skemman.is/bitstream/1946/15343/3/SS_MSthesis.pdf
 *
 * @param {Array<{x: number, y: number}>} data - Array of data points
 * @param {number} threshold - Target number of points after downsampling
 * @returns {Array<{x: number, y: number}>} Downsampled data
 */
export function lttb(data, threshold) {
  if (!data || data.length === 0) {
    return [];
  }

  if (threshold >= data.length || threshold === 0) {
    return data; // Nothing to do
  }

  if (threshold < 3) {
    throw new Error('Threshold must be >= 3');
  }

  const sampled = [];
  const bucketSize = (data.length - 2) / (threshold - 2);

  let a = 0; // Initially a is the first point
  sampled.push(data[a]); // Always add the first point

  for (let i = 0; i < threshold - 2; i++) {
    // Calculate point average for next bucket (containing c)
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

    // Get the range for this bucket
    let rangeOffs = Math.floor((i + 0) * bucketSize) + 1;
    const rangeTo = Math.floor((i + 1) * bucketSize) + 1;

    // Point a
    const pointAX = data[a].x;
    const pointAY = data[a].y;

    let maxArea = -1;
    let maxAreaPoint = null;
    let nextA = null;

    for (; rangeOffs < rangeTo; rangeOffs++) {
      // Calculate triangle area over three buckets
      const area = Math.abs(
        (pointAX - avgX) * (data[rangeOffs].y - pointAY) -
        (pointAX - data[rangeOffs].x) * (avgY - pointAY)
      ) * 0.5;

      if (area > maxArea) {
        maxArea = area;
        maxAreaPoint = data[rangeOffs];
        nextA = rangeOffs; // Remember which point we picked
      }
    }

    sampled.push(maxAreaPoint); // Pick the point with the largest triangle
    a = nextA; // This point is the next a (chosen b_i)
  }

  sampled.push(data[data.length - 1]); // Always add the last point

  return sampled;
}

/**
 * Helper function to convert timestamp/value arrays to point objects
 *
 * @param {number[]} timestamps - Array of Unix timestamps
 * @param {number[]} values - Array of values
 * @returns {Array<{x: number, y: number}>} Array of point objects
 */
export function toPoints(timestamps, values) {
  if (!timestamps || !values || timestamps.length !== values.length) {
    return [];
  }

  return timestamps.map((timestamp, index) => ({
    x: timestamp,
    y: values[index],
  }));
}

/**
 * Helper function to convert point objects back to separate arrays
 *
 * @param {Array<{x: number, y: number}>} points - Array of point objects
 * @returns {{timestamps: number[], values: number[]}} Separate arrays
 */
export function fromPoints(points) {
  if (!points || points.length === 0) {
    return { timestamps: [], values: [] };
  }

  return {
    timestamps: points.map(p => p.x),
    values: points.map(p => p.y),
  };
}
