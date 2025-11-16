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
            { label: 'Uptime', value: status?.uptime || '--', icon: '⏱️' },
            { label: 'Free Heap', value: status?.freeHeap || '--', icon: '💾' },
            { label: 'Chip Model', value: status?.chipModel || '--', icon: '🔧' },
            { label: 'CPU Frequency', value: status?.cpuFreq || '--', icon: '⚡' },
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
            { label: 'WiFi SSID', value: status?.wifiSSID || '--', icon: '📶' },
            { label: 'IP Address', value: status?.ipAddress || '--', icon: '🌐' },
            { label: 'Signal Strength', value: status?.rssi || '--', icon: '📡' },
            { label: 'MAC Address', value: status?.macAddress || '--', icon: '🔖' },
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
          <span>MQTT Status</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[
            { label: 'Connection', value: status?.mqttConnected ? '✅ Connected' : '❌ Disconnected', icon: '🔌' },
            { label: 'Broker', value: status?.mqttBroker || '--', icon: '🖥️' },
            { label: 'Client ID', value: status?.mqttClientId || '--', icon: '🆔' },
            { label: 'Last Message', value: status?.mqttLastMessage || '--', icon: '📮' },
          ].map(({ label, value, icon }) => html`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400 flex items-center gap-2">
                <span>${icon}</span>
                <span>${label}</span>
              </span>
              <span class="font-semibold text-gray-900 dark:text-white text-sm">${value}</span>
            </div>
          `)}
        </div>
      </div>
    </div>
  `;
}
