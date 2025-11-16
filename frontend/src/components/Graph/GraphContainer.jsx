import { h } from 'preact';
import { useState, useMemo } from 'preact/hooks';
import htm from 'htm';
import { useHistoryData } from '../../hooks/useHistoryData.js';
import { filterByTimeRange, getTargetPoints } from '../../utils/dataProcessing.js';
import { lttb, toPoints, fromPoints } from '../../utils/lttb.js';
import { TimeRangeSelector } from './TimeRangeSelector.jsx';
import { DataPointsBadge } from './DataPointsBadge.jsx';
import { HistoryChart } from './HistoryChart.jsx';

const html = htm.bind(h);

/**
 * GraphContainer Component
 * Main orchestrator for graph visualization with LTTB downsampling
 * Implements complete spec from GRAPH_VISUALIZATION_REFACTOR.md
 */
export function GraphContainer() {
  const [timeRange, setTimeRange] = useState(24); // Default: 24 hours
  const { data: rawData, loading, error } = useHistoryData(30000); // Poll every 30s

  // Process data: filter by time range → downsample with LTTB
  const processedData = useMemo(() => {
    if (!rawData || !rawData.timestamps || rawData.timestamps.length === 0) {
      return null;
    }

    // Step 1: Filter by selected time range
    const filtered = filterByTimeRange(rawData, timeRange);
    if (!filtered || filtered.count === 0) {
      return null;
    }

    // Step 2: Determine if downsampling is needed
    const targetPoints = getTargetPoints(timeRange);
    const needsDownsampling = filtered.count > targetPoints;

    if (!needsDownsampling) {
      // Return filtered data as-is
      return {
        timestamps: filtered.timestamps,
        temperatures: filtered.temperatures,
        humidities: filtered.humidities,
        valvePositions: filtered.valvePositions,
        downsampledCount: filtered.count,
        originalCount: rawData.count,
        downsampled: false,
      };
    }

    // Step 3: Downsample each dataset with LTTB
    const tempPoints = toPoints(filtered.timestamps, filtered.temperatures);
    const humidPoints = toPoints(filtered.timestamps, filtered.humidities);
    const valvePoints = toPoints(filtered.timestamps, filtered.valvePositions);

    const sampledTemp = lttb(tempPoints, targetPoints);
    const sampledHumid = lttb(humidPoints, targetPoints);
    const sampledValve = lttb(valvePoints, targetPoints);

    // Use temperature timestamps as primary (all should be similar after LTTB)
    const { timestamps, values: temperatures } = fromPoints(sampledTemp);
    const { values: humidities } = fromPoints(sampledHumid);
    const { values: valvePositions } = fromPoints(sampledValve);

    return {
      timestamps,
      temperatures,
      humidities,
      valvePositions,
      downsampledCount: timestamps.length,
      originalCount: rawData.count,
      downsampled: true,
    };
  }, [rawData, timeRange]);

  // Loading state
  if (loading) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="animate-pulse space-y-4">
          <div class="h-10 bg-gray-200 dark:bg-gray-700 rounded w-64 mx-auto"></div>
          <div class="h-64 bg-gray-200 dark:bg-gray-700 rounded"></div>
          <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-48"></div>
        </div>
      </div>
    `;
  }

  // Error state
  if (error) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-red-500 text-5xl mb-4">⚠️</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            Failed to Load History Data
          </h3>
          <p class="text-gray-600 dark:text-gray-400 mb-4">${error}</p>
          <button
            onClick=${() => window.location.reload()}
            class="px-4 py-2 bg-primary-500 text-white rounded-lg hover:bg-primary-600"
          >
            Retry
          </button>
        </div>
      </div>
    `;
  }

  // No data state
  if (!processedData) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-gray-400 text-5xl mb-4">📊</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            No Data Available
          </h3>
          <p class="text-gray-600 dark:text-gray-400">
            ${timeRange}h time range has no data points yet.
          </p>
        </div>
      </div>
    `;
  }

  // Render chart with data
  return html`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Historical Data
      </h2>

      <!-- Time Range Selector -->
      <${TimeRangeSelector}
        selected=${timeRange}
        onChange=${setTimeRange}
      />

      <!-- Chart -->
      <div class="mb-4">
        <${HistoryChart}
          timestamps=${processedData.timestamps}
          temperatures=${processedData.temperatures}
          humidities=${processedData.humidities}
          valvePositions=${processedData.valvePositions}
          timeRange=${timeRange}
        />
      </div>

      <!-- Data Points Badge -->
      <div class="flex justify-end">
        <${DataPointsBadge}
          showing=${processedData.downsampledCount}
          total=${processedData.originalCount}
          downsampled=${processedData.downsampled}
        />
      </div>
    </div>
  `;
}
