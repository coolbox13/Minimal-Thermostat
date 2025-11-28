import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';

const html = htm.bind(h);

/**
 * Logs Page
 * Displays event logs from the ESP32
 */
export function Logs() {
  const [logs, setLogs] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [filter, setFilter] = useState('all'); // all, info, warning, error

  useEffect(() => {
    fetchLogs();
    const interval = setInterval(fetchLogs, 5000); // Update every 5s
    return () => clearInterval(interval);
  }, []);

  const fetchLogs = async () => {
    try {
      const response = await fetch('/api/logs');
      const data = await response.json();
      // API returns flat array, normalize level to lowercase for filtering
      const normalizedLogs = (Array.isArray(data) ? data : data.logs || []).map(log => ({
        ...log,
        level: log.level?.toLowerCase() || 'info'
      }));
      setLogs(normalizedLogs);
      setLoading(false);
    } catch (err) {
      setError(err.message);
      setLoading(false);
    }
  };

  const clearLogs = async () => {
    try {
      await fetch('/api/logs/clear', { method: 'POST' });
      setLogs([]);
    } catch (err) {
      setError(err.message);
    }
  };

  const filteredLogs = filter === 'all'
    ? logs
    : logs.filter(log => log.level === filter);

  const formatTimestamp = (timestamp) => {
    // Timestamp is in milliseconds since boot
    const totalSeconds = Math.floor(timestamp / 1000);
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;
    return `${hours}h ${minutes}m ${seconds}s`;
  };

  const getLevelStyle = (level) => {
    switch (level) {
      case 'error':
        return {
          bg: 'bg-red-50 dark:bg-red-900/20 border-red-200 dark:border-red-800',
          text: 'text-red-700 dark:text-red-300',
          icon: '❌',
        };
      case 'warning':
        return {
          bg: 'bg-orange-50 dark:bg-orange-900/20 border-orange-200 dark:border-orange-800',
          text: 'text-orange-700 dark:text-orange-300',
          icon: '⚠️',
        };
      case 'info':
      default:
        return {
          bg: 'bg-blue-50 dark:bg-blue-900/20 border-blue-200 dark:border-blue-800',
          text: 'text-blue-700 dark:text-blue-300',
          icon: 'ℹ️',
        };
    }
  };

  if (loading) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="animate-pulse space-y-4">
          <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-32"></div>
          ${[...Array(5)].map(() => html`
            <div class="border border-gray-200 dark:border-gray-700 rounded-lg p-4">
              <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-full"></div>
              <div class="h-3 bg-gray-200 dark:bg-gray-700 rounded w-24 mt-2"></div>
            </div>
          `)}
        </div>
      </div>
    `;
  }

  return html`
    <div class="space-y-4">
      <!-- Header with Filters and Actions -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-4">
        <div class="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
          <!-- Filter Buttons -->
          <div class="flex gap-2 flex-wrap">
            ${[
              { value: 'all', label: 'All', icon: '📋' },
              { value: 'info', label: 'Info', icon: 'ℹ️' },
              { value: 'warning', label: 'Warning', icon: '⚠️' },
              { value: 'error', label: 'Error', icon: '❌' },
            ].map(({ value, label, icon }) => html`
              <button
                key=${value}
                onClick=${() => setFilter(value)}
                class="${filter === value
                  ? 'bg-primary-500 text-white'
                  : 'bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300'
                } px-3 py-2 rounded-lg font-medium transition-all duration-200 hover:shadow-md flex items-center gap-1"
              >
                <span>${icon}</span>
                <span>${label}</span>
              </button>
            `)}
          </div>

          <!-- Actions -->
          <div class="flex gap-2">
            <button
              onClick=${fetchLogs}
              class="px-4 py-2 bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 rounded-lg font-medium hover:bg-gray-200 dark:hover:bg-gray-600 transition-all"
            >
              🔄 Refresh
            </button>
            <button
              onClick=${clearLogs}
              class="px-4 py-2 bg-red-500 text-white rounded-lg font-medium hover:bg-red-600 transition-all"
            >
              🗑️ Clear
            </button>
          </div>
        </div>
      </div>

      <!-- Logs List -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Event Logs (${filteredLogs.length})
        </h2>

        ${error && html`
          <div class="mb-4 p-4 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
            <p class="text-red-600 dark:text-red-400">${error}</p>
          </div>
        `}

        ${filteredLogs.length === 0 ? html`
          <div class="text-center py-12">
            <div class="text-gray-400 text-5xl mb-4">📋</div>
            <p class="text-gray-600 dark:text-gray-400">No logs available</p>
          </div>
        ` : html`
          <div class="space-y-2 max-h-[400px] sm:max-h-[600px] overflow-y-auto">
            ${filteredLogs.map((log, index) => {
              const style = getLevelStyle(log.level);
              return html`
                <div
                  key=${index}
                  class="${style.bg} border ${style.text} rounded-lg p-4 transition-all hover:shadow-md"
                >
                  <div class="flex items-start gap-3">
                    <span class="text-xl flex-shrink-0">${style.icon}</span>
                    <div class="flex-1 min-w-0">
                      <p class="font-mono text-sm break-words">${log.message}</p>
                      <p class="text-xs opacity-70 mt-1">
                        <span class="font-medium">[${log.tag || 'SYSTEM'}]</span> at ${formatTimestamp(log.timestamp)}
                      </p>
                    </div>
                  </div>
                </div>
              `;
            })}
          </div>
        `}
      </div>
    </div>
  `;
}
