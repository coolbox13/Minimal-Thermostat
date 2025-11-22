import { h } from 'preact';
import htm from 'htm';
import { useSensorData } from '../../hooks/useSensorData.js';

const html = htm.bind(h);

/**
 * SensorCard Component
 * Displays current sensor readings with loading and error states
 * Implements skeleton loading from PREACT_UX_IMPROVEMENTS.md
 */
export function SensorCard() {
  const { data, loading, error } = useSensorData(5000); // Poll every 5 seconds

  // Skeleton Loading State (show if loading OR data is null)
  if (loading || !data) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="animate-pulse">
          <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-40 mb-4"></div>
          <div class="space-y-3">
            ${[...Array(4)].map(() => html`
              <div class="flex justify-between items-center">
                <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-24"></div>
                <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-20"></div>
              </div>
            `)}
          </div>
        </div>
      </div>
    `;
  }

  // Error State
  if (error) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Current Readings
        </h2>
        <div class="text-center py-4">
          <div class="text-red-500 text-3xl mb-2">⚠️</div>
          <p class="text-red-600 dark:text-red-400 text-sm">${error}</p>
        </div>
      </div>
    `;
  }

  // Data State
  const readings = [
    {
      label: 'Temperature',
      value: data?.temperature != null ? `${data.temperature.toFixed(1)}°C` : '--',
      icon: '🌡️',
      color: 'text-blue-600 dark:text-blue-400',
    },
    {
      label: 'Humidity',
      value: data?.humidity != null ? `${data.humidity.toFixed(1)}%` : '--',
      icon: '💧',
      color: 'text-green-600 dark:text-green-400',
    },
    {
      label: 'Pressure',
      value: data?.pressure != null ? `${data.pressure.toFixed(0)} hPa` : '--',
      icon: '🌤️',
      color: 'text-purple-600 dark:text-purple-400',
    },
    {
      label: 'Valve Position',
      value: data?.valve != null ? `${data.valve}%` : '--',
      icon: '🔧',
      color: 'text-orange-600 dark:text-orange-400',
    },
  ];

  return html`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Current Readings
      </h2>
      <div class="space-y-3">
        ${readings.map(({ label, value, icon, color }) => html`
          <div class="flex justify-between items-center py-2 border-b border-gray-100 dark:border-gray-700 last:border-0">
            <div class="flex items-center gap-2">
              <span class="text-xl">${icon}</span>
              <span class="text-gray-600 dark:text-gray-400">${label}</span>
            </div>
            <span class="${color} text-lg font-semibold">${value}</span>
          </div>
        `)}
      </div>
    </div>
  `;
}
