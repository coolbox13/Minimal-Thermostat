import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';
import { FactoryResetModal } from '../components/common/FactoryResetModal.jsx';
import { ValidatedInput } from '../components/common/ValidatedInput.jsx';
import { validatePID } from '../utils/validation.js';
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

  // PID parameter state with validation
  const [pidValues, setPidValues] = useState({
    kp: '2.0',
    ki: '0.5',
    kd: '1.0',
  });
  const [pidValid, setPidValid] = useState({
    kp: true,
    ki: true,
    kd: true,
  });

  useEffect(() => {
    fetchConfig();
  }, []);

  // Update PID state when config loads
  useEffect(() => {
    if (config?.pid) {
      setPidValues({
        kp: config.pid.kp?.toString() || '2.0',
        ki: config.pid.ki?.toString() || '0.5',
        kd: config.pid.kd?.toString() || '1.0',
      });
    }
  }, [config]);

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

  // Handle PID value changes
  const handlePidChange = (param, value) => {
    setPidValues(prev => ({ ...prev, [param]: value }));

    // Validate
    const validation = validatePID(value, param);
    setPidValid(prev => ({ ...prev, [param]: validation.isValid }));
  };

  // Save PID configuration
  const savePidConfig = async () => {
    // Check if all values are valid
    const allValid = pidValid.kp && pidValid.ki && pidValid.kd;
    if (!allValid) {
      toast.error('Please fix validation errors before saving');
      return;
    }

    await saveConfig('pid', {
      kp: parseFloat(pidValues.kp),
      ki: parseFloat(pidValues.ki),
      kd: parseFloat(pidValues.kd),
    });
  };

  // Check if PID form is valid for save
  const isPidFormValid = pidValid.kp && pidValid.ki && pidValid.kd;

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

        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4 mb-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>PID Tuning Guide:</strong> Kp controls proportional response, Ki reduces steady-state error, and Kd dampens oscillations. Adjust carefully for optimal temperature control.
          </p>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
          <${ValidatedInput}
            label="Kp (Proportional)"
            type="number"
            step="0.1"
            value=${pidValues.kp}
            onChange=${(value) => handlePidChange('kp', value)}
            validator=${(value) => validatePID(value, 'kp')}
            helpText="Range: 0.1 - 100"
            required=${true}
          />

          <${ValidatedInput}
            label="Ki (Integral)"
            type="number"
            step="0.01"
            value=${pidValues.ki}
            onChange=${(value) => handlePidChange('ki', value)}
            validator=${(value) => validatePID(value, 'ki')}
            helpText="Range: 0 - 10"
            required=${true}
          />

          <${ValidatedInput}
            label="Kd (Derivative)"
            type="number"
            step="0.01"
            value=${pidValues.kd}
            onChange=${(value) => handlePidChange('kd', value)}
            validator=${(value) => validatePID(value, 'kd')}
            helpText="Range: 0 - 10"
            required=${true}
          />
        </div>

        <div class="mt-4 flex items-center gap-3">
          <button
            onClick=${savePidConfig}
            disabled=${saving || !isPidFormValid}
            class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 disabled:cursor-not-allowed text-white rounded-lg font-medium transition-all"
          >
            ${saving ? 'Saving...' : 'Save PID Settings'}
          </button>

          ${!isPidFormValid && html`
            <span class="text-sm text-red-600 dark:text-red-400 flex items-center gap-1">
              <span>⚠️</span>
              <span>Please fix validation errors</span>
            </span>
          `}
        </div>
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
