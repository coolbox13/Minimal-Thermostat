import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';

const html = htm.bind(h);

/**
 * Status Page
 * Displays ESP32 system status information
 */
export function Status() {
  const [status, setStatus] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    fetchStatus();
    const interval = setInterval(fetchStatus, 10000); // Update every 10s
    return () => clearInterval(interval);
  }, []);

  const fetchStatus = async () => {
    try {
      const response = await fetch('/api/status');
      const data = await response.json();
      setStatus(data);
      setLoading(false);
    } catch (err) {
      setError(err.message);
      setLoading(false);
    }
  };

  if (loading) {
    return html`
      <div class="space-y-6">
        ${[...Array(3)].map(() => html`
          <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6 animate-pulse">
            <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-32 mb-4"></div>
            <div class="space-y-3">
              ${[...Array(4)].map(() => html`
                <div class="flex justify-between">
                  <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-24"></div>
                  <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-32"></div>
                </div>
              `)}
            </div>
          </div>
        `)}
      </div>
    `;
  }

  if (error) {
    return html`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-red-500 text-5xl mb-4">⚠️</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            Failed to Load Status
          </h3>
          <p class="text-gray-600 dark:text-gray-400 mb-4">${error}</p>
          <button
            onClick=${fetchStatus}
            class="px-4 py-2 bg-primary-500 text-white rounded-lg hover:bg-primary-600"
          >
            Retry
          </button>
        </div>
      </div>
    `;
  }

  // Helper function to format duration
  const formatDuration = (seconds) => {
    if (!seconds) return '--';
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    if (days > 0) return `${days}d ${hours}h ${minutes}m`;
    if (hours > 0) return `${hours}h ${minutes}m ${secs}s`;
    if (minutes > 0) return `${minutes}m ${secs}s`;
    return `${secs}s`;
  };

  // Helper function to format interval (ms to readable)
  const formatInterval = (ms) => {
    if (!ms) return '--';
    if (ms < 1000) return `${ms}ms`;
    if (ms < 60000) return `${(ms / 1000).toFixed(1)}s`;
    if (ms < 3600000) return `${(ms / 60000).toFixed(1)}min`;
    return `${(ms / 3600000).toFixed(1)}h`;
  };

  return html`
    <div class="space-y-6">
      <!-- System Information -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>🖥️</span>
          <span>System Information</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Uptime', value: formatDuration(status?.system?.uptime), icon: '⏱️' },
            { label: 'Free Heap', value: status?.system?.free_heap ? `${(status.system.free_heap / 1024).toFixed(1)} KB` : '--', icon: '💾' },
            { label: 'Total Heap', value: status?.system?.total_heap ? `${(status.system.total_heap / 1024).toFixed(1)} KB` : '--', icon: '💾' },
            { label: 'Chip Model', value: status?.system?.chip_model || '--', icon: '🔧' },
            { label: 'CPU Frequency', value: status?.system?.cpu_freq_mhz ? `${status.system.cpu_freq_mhz} MHz` : '--', icon: '⚡' },
            { label: 'Free Flash', value: status?.system?.free_flash_kb ? `${status.system.free_flash_kb} KB` : '--', icon: '💿' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white">${value}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Network Information -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>📡</span>
          <span>Network Status</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'WiFi SSID', value: status?.wifi?.ssid || '--', icon: '📶' },
            { label: 'IP Address', value: status?.wifi?.ip || '--', icon: '🌐' },
            { label: 'Signal Strength', value: status?.wifi?.rssi ? `${status.wifi.rssi} dBm` : '--', icon: '📡' },
            { label: 'Signal Quality', value: status?.wifi?.quality ? `${status.wifi.quality}%` : '--', icon: '📊' },
            { label: 'MAC Address', value: status?.wifi?.mac || '--', icon: '🔖' },
            { label: 'Connected', value: status?.wifi?.connected ? '✅ Yes' : '❌ No', icon: '🔌' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white font-mono text-sm">${value}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- MQTT Status -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>📨</span>
          <span>MQTT Configuration</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Server', value: status?.mqtt?.server || '--', icon: '🖥️' },
            { label: 'Port', value: status?.mqtt?.port || '--', icon: '🔌' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white font-mono text-sm">${value}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Timing Intervals -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>⏱️</span>
          <span>Timing Intervals</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Sensor Update', value: formatInterval(status?.timing?.sensor_update_interval), icon: '🌡️' },
            { label: 'History Update', value: formatInterval(status?.timing?.history_update_interval), icon: '📊' },
            { label: 'PID Update', value: formatInterval(status?.timing?.pid_update_interval), icon: '🎛️' },
            { label: 'Connectivity Check', value: formatInterval(status?.timing?.connectivity_check_interval), icon: '🔍' },
            { label: 'PID Config Write', value: formatInterval(status?.timing?.pid_config_write_interval), icon: '💾' },
            { label: 'WiFi Connect Timeout', value: status?.timing?.wifi_connect_timeout ? `${status.timing.wifi_connect_timeout}s` : '--', icon: '⏳' },
            { label: 'Max Reconnect Attempts', value: status?.timing?.max_reconnect_attempts || '--', icon: '🔄' },
            { label: 'System Watchdog', value: formatInterval(status?.timing?.system_watchdog_timeout), icon: '🐕' },
            { label: 'WiFi Watchdog', value: formatInterval(status?.timing?.wifi_watchdog_timeout), icon: '📡' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white">${value}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- PID Configuration -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>🎛️</span>
          <span>PID Configuration</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Kp', value: status?.pid?.kp?.toFixed(3) || '--', icon: '📈' },
            { label: 'Ki', value: status?.pid?.ki?.toFixed(3) || '--', icon: '📊' },
            { label: 'Kd', value: status?.pid?.kd?.toFixed(3) || '--', icon: '📉' },
            { label: 'Setpoint', value: status?.pid?.setpoint ? `${status.pid.setpoint.toFixed(1)}°C` : '--', icon: '🎯' },
            { label: 'Deadband', value: status?.pid?.deadband ? `${status.pid.deadband.toFixed(2)}°C` : '--', icon: '📏' },
            { label: 'Valve Position', value: status?.pid?.valve_position !== undefined ? `${status.pid.valve_position}%` : '--', icon: '🔧' },
            { label: 'Adaptation Interval', value: status?.pid?.adaptation_interval ? `${status.pid.adaptation_interval}s` : '--', icon: '⏱️' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white">${value}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Presets -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>🌡️</span>
          <span>Temperature Presets</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Current Preset', value: status?.presets?.current || '--', icon: '📍' },
            { label: 'Eco', value: status?.presets?.eco ? `${status.presets.eco.toFixed(1)}°C` : '--', icon: '🌱' },
            { label: 'Comfort', value: status?.presets?.comfort ? `${status.presets.comfort.toFixed(1)}°C` : '--', icon: '😊' },
            { label: 'Away', value: status?.presets?.away ? `${status.presets.away.toFixed(1)}°C` : '--', icon: '🏠' },
            { label: 'Sleep', value: status?.presets?.sleep ? `${status.presets.sleep.toFixed(1)}°C` : '--', icon: '😴' },
            { label: 'Boost', value: status?.presets?.boost ? `${status.presets.boost.toFixed(1)}°C` : '--', icon: '🔥' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white">${value}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Diagnostics -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>🔍</span>
          <span>Diagnostics</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Last Reboot Reason', value: status?.diagnostics?.last_reboot_reason || '--', icon: '🔄' },
            { label: 'Reboot Count', value: status?.diagnostics?.reboot_count || '--', icon: '🔢' },
            { label: 'Watchdog Reboots', value: status?.diagnostics?.consecutive_watchdog_reboots || '--', icon: '🐕' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white">${value}</span>
            </div>
          `)}
        </div>
      </div>
    </div>
  `;
}
