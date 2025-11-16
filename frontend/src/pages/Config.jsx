import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';
import { FactoryResetModal } from '../components/common/FactoryResetModal.jsx';
import toast from '../utils/toast.js';

const html = htm.bind(h);

/**
 * Config Page
 * Configuration settings for WiFi, MQTT, PID, and system
 */
export function Config() {
  const [config, setConfig] = useState(null);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [showResetModal, setShowResetModal] = useState(false);

  useEffect(() => {
    fetchConfig();
  }, []);

  const fetchConfig = async () => {
    try {
      const response = await fetch('/api/config');
      const data = await response.json();
      setConfig(data);
      setLoading(false);
    } catch (err) {
      toast.error(`Failed to load config: ${err.message}`);
      setLoading(false);
    }
  };

  const saveConfig = async (section, values) => {
    setSaving(true);
    try {
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ section, ...values }),
      });

      if (!response.ok) {
        throw new Error('Failed to save configuration');
      }

      toast.success('Configuration saved successfully');
      await fetchConfig(); // Refresh config
    } catch (err) {
      toast.error(`Failed to save config: ${err.message}`);
    } finally {
      setSaving(false);
    }
  };

  const handleFactoryReset = async () => {
    await fetch('/api/reset', { method: 'POST' });
    // Device will reboot, show message
    toast.info('Device is resetting... Please wait.');
    setTimeout(() => {
      window.location.reload();
    }, 5000);
  };

  if (loading) {
    return html`
      <div class="space-y-6">
        ${[...Array(4)].map(() => html`
          <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6 animate-pulse">
            <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-40 mb-4"></div>
            <div class="space-y-3">
              ${[...Array(3)].map(() => html`
                <div class="h-10 bg-gray-200 dark:bg-gray-700 rounded"></div>
              `)}
            </div>
          </div>
        `)}
      </div>
    `;
  }

  return html`
    <div class="space-y-6">
      <!-- WiFi Configuration -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>📡</span>
          <span>WiFi Configuration</span>
        </h2>
        <div class="space-y-4">
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              SSID
            </label>
            <input
              type="text"
              value=${config?.wifi?.ssid || ''}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="Enter WiFi SSID"
            />
          </div>
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Password
            </label>
            <input
              type="password"
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="Enter WiFi password"
            />
          </div>
          <button
            disabled=${saving}
            class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${saving ? 'Saving...' : 'Save WiFi Settings'}
          </button>
        </div>
      </div>

      <!-- MQTT Configuration -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>📨</span>
          <span>MQTT Configuration</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Broker Address
            </label>
            <input
              type="text"
              value=${config?.mqtt?.broker || ''}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="mqtt.example.com"
            />
          </div>
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Port
            </label>
            <input
              type="number"
              value=${config?.mqtt?.port || 1883}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
            />
          </div>
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Username
            </label>
            <input
              type="text"
              value=${config?.mqtt?.username || ''}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
            />
          </div>
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Password
            </label>
            <input
              type="password"
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
            />
          </div>
        </div>
        <button
          disabled=${saving}
          class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
        >
          ${saving ? 'Saving...' : 'Save MQTT Settings'}
        </button>
      </div>

      <!-- PID Configuration -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>🎛️</span>
          <span>PID Configuration</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Kp (Proportional)
            </label>
            <input
              type="number"
              step="0.1"
              value=${config?.pid?.kp || 2.0}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
            />
          </div>
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Ki (Integral)
            </label>
            <input
              type="number"
              step="0.01"
              value=${config?.pid?.ki || 0.5}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
            />
          </div>
          <div>
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Kd (Derivative)
            </label>
            <input
              type="number"
              step="0.01"
              value=${config?.pid?.kd || 1.0}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
            />
          </div>
        </div>
        <button
          disabled=${saving}
          class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
        >
          ${saving ? 'Saving...' : 'Save PID Settings'}
        </button>
      </div>

      <!-- Danger Zone -->
      <div class="bg-red-50 dark:bg-red-900/20 border-2 border-red-200 dark:border-red-800 rounded-xl p-6">
        <h2 class="text-xl font-bold text-red-800 dark:text-red-300 mb-2 flex items-center gap-2">
          <span>⚠️</span>
          <span>Danger Zone</span>
        </h2>
        <p class="text-red-700 dark:text-red-400 mb-4">
          Irreversible actions that will affect your device configuration.
        </p>
        <div class="space-y-3">
          <div class="flex justify-between items-center p-4 bg-white dark:bg-gray-800 rounded-lg">
            <div>
              <h3 class="font-semibold text-gray-900 dark:text-white">Factory Reset</h3>
              <p class="text-sm text-gray-600 dark:text-gray-400">Reset all settings to factory defaults</p>
            </div>
            <button
              onClick=${() => setShowResetModal(true)}
              class="px-4 py-2 bg-red-500 hover:bg-red-600 text-white rounded-lg font-medium transition-all"
            >
              Reset Device
            </button>
          </div>
        </div>
      </div>

      <!-- Factory Reset Modal -->
      <${FactoryResetModal}
        isOpen=${showResetModal}
        onClose=${() => setShowResetModal(false)}
        onConfirm=${handleFactoryReset}
      />
    </div>
  `;
}
