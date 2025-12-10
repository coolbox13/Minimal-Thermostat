import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';
import { FactoryResetModal } from '../components/common/FactoryResetModal.jsx';
import { ConfigWizard } from '../components/common/ConfigWizard.jsx';
import { ValidatedInput } from '../components/common/ValidatedInput.jsx';
import { validatePID } from '../utils/validation.js';
import toast from '../utils/toast.js';

const html = htm.bind(h);

/**
 * Config Page
 * Complete configuration settings matching main branch functionality
 */
export function Config() {
  const [config, setConfig] = useState(null);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [activeTab, setActiveTab] = useState('network');
  const [showResetModal, setShowResetModal] = useState(false);
  const [showWizard, setShowWizard] = useState(false);

  // Form state
  const [formData, setFormData] = useState({});

  // PID parameter state with validation
  const [pidValues, setPidValues] = useState({
    kp: '2.0',
    ki: '0.5',
    kd: '1.0',
    setpoint: '22.0',
    deadband: '0.5',
    adaptation_enabled: true,
    adaptation_interval: '1800',
  });
  const [pidValid, setPidValid] = useState({
    kp: true,
    ki: true,
    kd: true,
    setpoint: true,
    deadband: true,
    adaptation_interval: true,
  });

  useEffect(() => {
    fetchConfig();
  }, []);

  // Update form state when config loads
  useEffect(() => {
    if (config) {
      setFormData({
        wifi_ssid: config.network?.wifi_ssid || '',
        wifi_pass: '',
        ntp_server: config.network?.ntp_server || 'pool.ntp.org',
        ntp_timezone_offset: config.network?.ntp_timezone_offset || 0,
        ntp_daylight_offset: config.network?.ntp_daylight_offset || 0,
        mqtt_server: config.mqtt?.server || '',
        mqtt_port: config.mqtt?.port || 1883,
        mqtt_username: config.mqtt?.username || '',
        mqtt_password: '',
        mqtt_json_aggregate_enabled: config.mqtt?.json_aggregate_enabled || false,
        knx_area: config.knx?.area || 0,
        knx_line: config.knx?.line || 0,
        knx_member: config.knx?.member || 0,
        knx_test: config.knx?.use_test || false,
        knx_valve_cmd_area: config.knx?.valve_command?.area || 0,
        knx_valve_cmd_line: config.knx?.valve_command?.line || 0,
        knx_valve_cmd_member: config.knx?.valve_command?.member || 0,
        knx_valve_fb_area: config.knx?.valve_feedback?.area || 0,
        knx_valve_fb_line: config.knx?.valve_feedback?.line || 0,
        knx_valve_fb_member: config.knx?.valve_feedback?.member || 0,
        bme280_address: config.bme280?.address || '0x76',
        bme280_sda: config.bme280?.sda_pin || 21,
        bme280_scl: config.bme280?.scl_pin || 22,
        bme280_interval: config.bme280?.interval || 30,
        preset_eco: config.presets?.eco || 18.0,
        preset_comfort: config.presets?.comfort || 22.0,
        preset_away: config.presets?.away || 16.0,
        preset_sleep: config.presets?.sleep || 19.0,
        preset_boost: config.presets?.boost || 24.0,
        sensor_update_interval: config.timing?.sensor_update_interval || 30000,
        history_update_interval: config.timing?.history_update_interval || 30000,
        pid_update_interval: config.timing?.pid_update_interval || 10000,
        connectivity_check_interval: config.timing?.connectivity_check_interval || 300000,
        pid_config_write_interval: config.timing?.pid_config_write_interval || 300000,
        wifi_connect_timeout: config.timing?.wifi_connect_timeout || 180,
        system_watchdog_timeout: config.timing?.system_watchdog_timeout || 2700000,
        wifi_watchdog_timeout: config.timing?.wifi_watchdog_timeout || 1800000,
        max_reconnect_attempts: config.timing?.max_reconnect_attempts || 10,
        webhook_enabled: config.webhook?.enabled || false,
        webhook_url: config.webhook?.url || '',
        webhook_temp_low: config.webhook?.temp_low_threshold || 15.0,
        webhook_temp_high: config.webhook?.temp_high_threshold || 30.0,
      });

      if (config.pid) {
        setPidValues({
          kp: config.pid.kp?.toString() || '2.0',
          ki: config.pid.ki?.toString() || '0.5',
          kd: config.pid.kd?.toString() || '1.0',
          setpoint: config.pid.setpoint?.toString() || '22.0',
          deadband: config.pid.deadband?.toString() || '0.5',
          adaptation_enabled: config.pid.adaptation_enabled ?? true,
          adaptation_interval: config.pid.adaptation_interval?.toString() || '60',
        });
      }
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

  const updateFormData = (key, value) => {
    setFormData(prev => ({ ...prev, [key]: value }));
  };

  const saveConfig = async (section, values) => {
    setSaving(true);
    try {
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ [section]: values }),
      });

      if (!response.ok) {
        const error = await response.json();
        throw new Error(error.message || 'Failed to save configuration');
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
    const validation = validatePID(value, param);
    setPidValid(prev => ({ ...prev, [param]: validation.isValid }));
  };

  // Save PID configuration
  const savePidConfig = async () => {
    const allValid = Object.values(pidValid).every(v => v);
    if (!allValid) {
      toast.error('Please fix validation errors before saving');
      return;
    }

    await saveConfig('pid', {
      kp: parseFloat(pidValues.kp),
      ki: parseFloat(pidValues.ki),
      kd: parseFloat(pidValues.kd),
      setpoint: parseFloat(pidValues.setpoint),
      deadband: parseFloat(pidValues.deadband),
      adaptation_enabled: pidValues.adaptation_enabled,
      adaptation_interval: parseFloat(pidValues.adaptation_interval),
    });
  };

  const isPidFormValid = Object.values(pidValid).every(v => v);

  const handleFactoryReset = async () => {
    try {
      const response = await fetch('/api/factory-reset', { method: 'POST' });
      if (!response.ok) throw new Error('Factory reset failed');
      toast.info('Device is resetting... Please wait.');
      setTimeout(() => {
        window.location.reload();
      }, 5000);
    } catch (err) {
      toast.error(`Factory reset failed: ${err.message}`);
    }
  };

  const handleReboot = async () => {
    try {
      const response = await fetch('/api/reboot', { method: 'POST' });
      if (!response.ok) throw new Error('Reboot failed');
      toast.info('Device is rebooting... Please wait.');
      setTimeout(() => {
        window.location.reload();
      }, 5000);
    } catch (err) {
      toast.error(`Reboot failed: ${err.message}`);
    }
  };

  const handleExportConfig = async () => {
    try {
      const response = await fetch('/api/config/export');
      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `thermostat-config-${new Date().toISOString().split('T')[0]}.json`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      window.URL.revokeObjectURL(url);
      toast.success('Configuration exported');
    } catch (err) {
      toast.error(`Export failed: ${err.message}`);
    }
  };

  const handleImportConfig = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = async (e) => {
      const file = e.target.files[0];
      if (!file) return;

      try {
        const text = await file.text();
        const json = JSON.parse(text);
        const response = await fetch('/api/config/import', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: text,
        });

        if (!response.ok) throw new Error('Import failed');
        toast.success('Configuration imported successfully');
        await fetchConfig();
      } catch (err) {
        toast.error(`Import failed: ${err.message}`);
      }
    };
    input.click();
  };

  const testWebhook = async () => {
    try {
      const response = await fetch('/api/webhook/test', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          url: formData.webhook_url,
          temp_low: formData.webhook_temp_low,
          temp_high: formData.webhook_temp_high,
        }),
      });

      if (!response.ok) throw new Error('Webhook test failed');
      toast.success('Webhook test sent successfully');
    } catch (err) {
      toast.error(`Webhook test failed: ${err.message}`);
    }
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

  const tabs = [
    { id: 'network', label: 'Network' },
    { id: 'mqtt', label: 'MQTT' },
    { id: 'knx', label: 'KNX' },
    { id: 'bme280', label: 'BME280' },
    { id: 'pid', label: 'PID' },
    { id: 'presets', label: 'Presets' },
    { id: 'timing', label: 'Timing' },
    { id: 'webhook', label: 'Webhook' },
  ];

  return html`
    <div class="space-y-6">
      <!-- Getting Started Section -->
      <div class="bg-gradient-to-r from-primary-500 to-blue-600 rounded-xl shadow-lg p-6 text-white">
        <div class="flex flex-col md:flex-row md:items-center md:justify-between gap-4">
          <div>
            <h2 class="text-2xl font-bold mb-2 flex items-center gap-2">
              <span>🚀</span>
              <span>Getting Started</span>
            </h2>
            <p class="text-white/90">
              New to the ESP32 Thermostat? Use our setup wizard to configure your device step-by-step.
            </p>
          </div>
          <button
            onClick=${() => setShowWizard(true)}
            class="px-6 py-3 bg-white text-primary-600 hover:bg-gray-100 rounded-lg font-semibold transition-all flex items-center gap-2 whitespace-nowrap self-start md:self-center"
          >
            <span>✨</span>
            <span>Launch Setup Wizard</span>
          </button>
        </div>
      </div>

      <!-- Tab Navigation -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-4">
        <div class="flex flex-wrap gap-2">
          ${tabs.map(tab => html`
            <button
              onClick=${() => setActiveTab(tab.id)}
              class="px-4 py-2 rounded-lg font-medium transition-all ${
                activeTab === tab.id
                  ? 'bg-primary-500 text-white'
                  : 'bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 hover:bg-gray-200 dark:hover:bg-gray-600'
              }"
            >
              ${tab.label}
            </button>
          `)}
        </div>
      </div>

      <!-- Network Tab -->
      ${activeTab === 'network' && html`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>📡</span>
            <span>Network Configuration</span>
          </h2>
          <div class="space-y-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                WiFi SSID
              </label>
              <input
                type="text"
                value=${formData.wifi_ssid || ''}
                onInput=${(e) => updateFormData('wifi_ssid', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Enter WiFi SSID"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                WiFi Password
              </label>
              <input
                type="password"
                autocomplete="current-password"
                value=${formData.wifi_pass || ''}
                onInput=${(e) => updateFormData('wifi_pass', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Leave empty to keep current password"
              />
            </div>
            <div class="border-t border-gray-200 dark:border-gray-700 pt-4 mt-4">
              <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-4">NTP Time Synchronization</h3>
              <div class="space-y-4">
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                    NTP Server
                  </label>
                  <input
                    type="text"
                    value=${formData.ntp_server || 'pool.ntp.org'}
                    onInput=${(e) => updateFormData('ntp_server', e.target.value)}
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                    placeholder="pool.ntp.org"
                  />
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                    Timezone Offset (seconds)
                  </label>
                  <input
                    type="number"
                    value=${formData.ntp_timezone_offset || 0}
                    onInput=${(e) => updateFormData('ntp_timezone_offset', parseInt(e.target.value))}
                    step="3600"
                    min="-43200"
                    max="43200"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                  <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    UTC offset in seconds (e.g., UTC+1 = 3600, UTC+2 = 7200, UTC-5 = -18000)
                  </p>
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                    Daylight Saving Offset (seconds)
                  </label>
                  <input
                    type="number"
                    value=${formData.ntp_daylight_offset || 0}
                    onInput=${(e) => updateFormData('ntp_daylight_offset', parseInt(e.target.value))}
                    step="3600"
                    min="0"
                    max="7200"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                  <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    DST offset in seconds (typically 3600 for 1 hour, 0 = no DST)
                  </p>
                </div>
              </div>
            </div>
            <button
              onClick=${() => saveConfig('network', {
                wifi_ssid: formData.wifi_ssid,
                wifi_pass: formData.wifi_pass || undefined,
                ntp_server: formData.ntp_server,
                ntp_timezone_offset: formData.ntp_timezone_offset,
                ntp_daylight_offset: formData.ntp_daylight_offset,
              })}
              disabled=${saving}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
            >
              ${saving ? 'Saving...' : 'Save Network Settings'}
            </button>
          </div>
        </div>
      `}

      <!-- MQTT Tab -->
      ${activeTab === 'mqtt' && html`
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
                value=${formData.mqtt_server || ''}
                onInput=${(e) => updateFormData('mqtt_server', e.target.value)}
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
                value=${formData.mqtt_port || 1883}
                onInput=${(e) => updateFormData('mqtt_port', parseInt(e.target.value))}
                min="1"
                max="65535"
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Username
              </label>
              <input
                type="text"
                value=${formData.mqtt_username || ''}
                onInput=${(e) => updateFormData('mqtt_username', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Leave empty for no authentication"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Password
              </label>
              <input
                type="password"
                autocomplete="current-password"
                value=${formData.mqtt_password || ''}
                onInput=${(e) => updateFormData('mqtt_password', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Leave empty to keep current password"
              />
            </div>
          </div>
          <div class="mt-4 flex items-center gap-3">
            <input
              type="checkbox"
              id="mqtt_json_aggregate_enabled"
              checked=${formData.mqtt_json_aggregate_enabled || false}
              onChange=${(e) => updateFormData('mqtt_json_aggregate_enabled', e.target.checked)}
              class="w-4 h-4 rounded"
            />
            <label for="mqtt_json_aggregate_enabled" class="text-sm font-medium text-gray-700 dark:text-gray-300">
              Enable JSON Aggregate
            </label>
            <span class="text-xs text-gray-500 dark:text-gray-400">
              Publish all data as JSON on 'telegraph' topic
            </span>
          </div>
          <button
            onClick=${() => saveConfig('mqtt', {
              server: formData.mqtt_server,
              port: formData.mqtt_port,
              username: formData.mqtt_username || undefined,
              password: formData.mqtt_password || undefined,
              json_aggregate_enabled: formData.mqtt_json_aggregate_enabled,
            })}
            disabled=${saving}
            class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${saving ? 'Saving...' : 'Save MQTT Settings'}
          </button>
        </div>
      `}

      <!-- KNX Tab -->
      ${activeTab === 'knx' && html`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🏠</span>
            <span>KNX Configuration</span>
          </h2>
          <div class="space-y-6">
            <div>
              <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-4">Physical Address</h3>
              <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Area</label>
                  <input
                    type="number"
                    value=${formData.knx_area || 0}
                    onInput=${(e) => updateFormData('knx_area', parseInt(e.target.value))}
                    min="0"
                    max="15"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Line</label>
                  <input
                    type="number"
                    value=${formData.knx_line || 0}
                    onInput=${(e) => updateFormData('knx_line', parseInt(e.target.value))}
                    min="0"
                    max="15"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Member</label>
                  <input
                    type="number"
                    value=${formData.knx_member || 0}
                    onInput=${(e) => updateFormData('knx_member', parseInt(e.target.value))}
                    min="0"
                    max="255"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                </div>
              </div>
            </div>
            <div>
              <div class="flex items-center gap-3 mb-4">
                <input
                  type="checkbox"
                  id="knx_test"
                  checked=${formData.knx_test || false}
                  onChange=${(e) => updateFormData('knx_test', e.target.checked)}
                  class="w-4 h-4 rounded"
                />
                <label for="knx_test" class="text-sm font-medium text-gray-700 dark:text-gray-300">
                  Use Test Addresses
                </label>
              </div>
              ${formData.knx_test && html`
                <div class="bg-yellow-50 dark:bg-yellow-900/20 border border-yellow-200 dark:border-yellow-800 rounded-lg p-4">
                  <p class="text-sm text-yellow-800 dark:text-yellow-300">
                    <strong>Active Test Addresses:</strong> Valve Command/Feedback: 10/2/2
                  </p>
                </div>
              `}
              ${!formData.knx_test && html`
                <div class="space-y-4">
                  <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
                    <h4 class="font-semibold text-blue-900 dark:text-blue-300 mb-3">Valve Command Address</h4>
                    <div class="grid grid-cols-3 gap-3">
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Area</label>
                        <input
                          type="number"
                          value=${formData.knx_valve_cmd_area || 0}
                          onInput=${(e) => updateFormData('knx_valve_cmd_area', parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Line</label>
                        <input
                          type="number"
                          value=${formData.knx_valve_cmd_line || 0}
                          onInput=${(e) => updateFormData('knx_valve_cmd_line', parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Member</label>
                        <input
                          type="number"
                          value=${formData.knx_valve_cmd_member || 0}
                          onInput=${(e) => updateFormData('knx_valve_cmd_member', parseInt(e.target.value))}
                          min="0"
                          max="255"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                    </div>
                  </div>
                  <div class="bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg p-4">
                    <h4 class="font-semibold text-green-900 dark:text-green-300 mb-3">Valve Feedback Address</h4>
                    <div class="grid grid-cols-3 gap-3">
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Area</label>
                        <input
                          type="number"
                          value=${formData.knx_valve_fb_area || 0}
                          onInput=${(e) => updateFormData('knx_valve_fb_area', parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Line</label>
                        <input
                          type="number"
                          value=${formData.knx_valve_fb_line || 0}
                          onInput=${(e) => updateFormData('knx_valve_fb_line', parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Member</label>
                        <input
                          type="number"
                          value=${formData.knx_valve_fb_member || 0}
                          onInput=${(e) => updateFormData('knx_valve_fb_member', parseInt(e.target.value))}
                          min="0"
                          max="255"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                    </div>
                  </div>
                </div>
              `}
            </div>
            <button
              onClick=${() => saveConfig('knx', {
                area: formData.knx_area,
                line: formData.knx_line,
                member: formData.knx_member,
                use_test: formData.knx_test,
                valve_command: {
                  area: formData.knx_valve_cmd_area,
                  line: formData.knx_valve_cmd_line,
                  member: formData.knx_valve_cmd_member,
                },
                valve_feedback: {
                  area: formData.knx_valve_fb_area,
                  line: formData.knx_valve_fb_line,
                  member: formData.knx_valve_fb_member,
                },
              })}
              disabled=${saving}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
            >
              ${saving ? 'Saving...' : 'Save KNX Settings'}
            </button>
          </div>
        </div>
      `}

      <!-- BME280 Tab -->
      ${activeTab === 'bme280' && html`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🌡️</span>
            <span>BME280 Sensor Configuration</span>
          </h2>
          <div class="space-y-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                I²C Address
              </label>
              <select
                value=${formData.bme280_address || '0x76'}
                onInput=${(e) => updateFormData('bme280_address', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              >
                <option value="0x76">0x76 (default)</option>
                <option value="0x77">0x77</option>
              </select>
            </div>
            <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  SDA Pin
                </label>
                <input
                  type="number"
                  value=${formData.bme280_sda || 21}
                  onInput=${(e) => updateFormData('bme280_sda', parseInt(e.target.value))}
                  min="0"
                  max="39"
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  SCL Pin
                </label>
                <input
                  type="number"
                  value=${formData.bme280_scl || 22}
                  onInput=${(e) => updateFormData('bme280_scl', parseInt(e.target.value))}
                  min="0"
                  max="39"
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Update Interval (seconds)
              </label>
              <input
                type="number"
                value=${formData.bme280_interval || 30}
                onInput=${(e) => updateFormData('bme280_interval', parseInt(e.target.value))}
                min="1"
                max="3600"
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
              <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                Seconds between sensor readings (1-3600)
              </p>
            </div>
            <button
              onClick=${() => saveConfig('bme280', {
                address: formData.bme280_address,
                sda_pin: formData.bme280_sda,
                scl_pin: formData.bme280_scl,
                interval: formData.bme280_interval,
              })}
              disabled=${saving}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
            >
              ${saving ? 'Saving...' : 'Save BME280 Settings'}
            </button>
          </div>
        </div>
      `}

      <!-- PID Tab -->
      ${activeTab === 'pid' && html`
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
          <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
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
          <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Default Setpoint (°C)
              </label>
              <input
                type="number"
                step="0.5"
                min="5"
                max="30"
                value=${pidValues.setpoint}
                onInput=${(e) => handlePidChange('setpoint', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Deadband (°C)
              </label>
              <input
                type="number"
                step="0.1"
                min="0"
                max="5"
                value=${pidValues.deadband}
                onInput=${(e) => handlePidChange('deadband', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
            </div>
          </div>

          <!-- Adaptation Settings -->
          <div class="mt-6 p-4 bg-gray-50 dark:bg-gray-900 rounded-lg border border-gray-200 dark:border-gray-700">
            <h4 class="text-sm font-semibold text-gray-900 dark:text-white mb-4">Auto-Tuning & Adaptation</h4>

            <div class="mb-4">
              <label class="flex items-center gap-3 cursor-pointer">
                <input
                  type="checkbox"
                  checked=${pidValues.adaptation_enabled}
                  onChange=${(e) => setPidValues({ ...pidValues, adaptation_enabled: e.target.checked })}
                  class="w-5 h-5 text-primary-600 bg-gray-100 border-gray-300 rounded focus:ring-primary-500 dark:focus:ring-primary-600 dark:ring-offset-gray-800 focus:ring-2 dark:bg-gray-700 dark:border-gray-600"
                />
                <div>
                  <span class="text-sm font-medium text-gray-900 dark:text-white">
                    Enable Continuous Adaptation
                  </span>
                  <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    Automatically adjusts PID parameters based on system performance. Disable for manual calibration.
                  </p>
                </div>
              </label>
            </div>

            ${pidValues.adaptation_enabled && html`
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  Adaptation Interval (seconds)
                </label>
                <input
                  type="number"
                  step="1"
                  min="10"
                  max="600"
                  value=${pidValues.adaptation_interval}
                  onInput=${(e) => handlePidChange('adaptation_interval', e.target.value)}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                  How often the system re-evaluates and adjusts PID parameters (10-600 seconds)
                </p>
              </div>
            `}
          </div>

          <div class="flex items-center gap-3">
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
      `}

      <!-- Presets Tab -->
      ${activeTab === 'presets' && html`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🌡️</span>
            <span>Temperature Presets</span>
          </h2>
          <p class="text-gray-600 dark:text-gray-400 mb-6">
            Configure the temperature for each preset mode. These presets can be quickly selected from the dashboard.
          </p>
          <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
            ${[
              { key: 'preset_eco', label: 'Eco Temperature', hint: 'Energy-saving temperature (typically 16-18°C)' },
              { key: 'preset_comfort', label: 'Comfort Temperature', hint: 'Comfortable temperature (typically 20-22°C)' },
              { key: 'preset_away', label: 'Away Temperature', hint: 'Temperature when away (typically 16-17°C)' },
              { key: 'preset_sleep', label: 'Sleep Temperature', hint: 'Nighttime temperature (typically 18-19°C)' },
              { key: 'preset_boost', label: 'Boost Temperature', hint: 'Quick heating temperature (typically 23-24°C)' },
            ].map(({ key, label, hint }) => html`
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  ${label} (°C)
                </label>
                <input
                  type="number"
                  step="0.5"
                  min="5"
                  max="30"
                  value=${formData[key] || ''}
                  onInput=${(e) => updateFormData(key, parseFloat(e.target.value))}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">${hint}</p>
              </div>
            `)}
          </div>
          <button
            onClick=${() => saveConfig('presets', {
              eco: formData.preset_eco,
              comfort: formData.preset_comfort,
              away: formData.preset_away,
              sleep: formData.preset_sleep,
              boost: formData.preset_boost,
            })}
            disabled=${saving}
            class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${saving ? 'Saving...' : 'Save Preset Settings'}
          </button>
        </div>
      `}

      <!-- Timing Tab -->
      ${activeTab === 'timing' && html`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>⏱️</span>
            <span>Timing Intervals</span>
          </h2>
          <p class="text-gray-600 dark:text-gray-400 mb-6">
            Configure how often the system updates sensor data, history, PID calculations, and performs connectivity checks.
          </p>
          <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
            ${[
              { key: 'sensor_update_interval', label: 'Sensor Update Interval', unit: 'ms', min: 1000, max: 300000, hint: 'How often to read sensor (1s - 5min)' },
              { key: 'history_update_interval', label: 'History Update Interval', unit: 'ms', min: 1000, max: 300000, hint: 'How often to save history (1s - 5min)' },
              { key: 'pid_update_interval', label: 'PID Update Interval', unit: 'ms', min: 1000, max: 60000, hint: 'How often to calculate PID (1s - 1min)' },
              { key: 'connectivity_check_interval', label: 'Connectivity Check Interval', unit: 'ms', min: 60000, max: 3600000, hint: 'How often to check connectivity (1min - 1hr)' },
              { key: 'pid_config_write_interval', label: 'PID Config Write Interval', unit: 'ms', min: 60000, max: 3600000, hint: 'How often to save PID config (1min - 1hr)' },
              { key: 'wifi_connect_timeout', label: 'WiFi Connect Timeout', unit: 's', min: 10, max: 600, hint: 'WiFi connection timeout (10s - 10min)' },
              { key: 'system_watchdog_timeout', label: 'System Watchdog Timeout', unit: 'ms', min: 60000, max: 7200000, hint: 'System watchdog timeout (1min - 2hr)' },
              { key: 'wifi_watchdog_timeout', label: 'WiFi Watchdog Timeout', unit: 'ms', min: 60000, max: 7200000, hint: 'WiFi watchdog timeout (1min - 2hr)' },
              { key: 'max_reconnect_attempts', label: 'Max Reconnect Attempts', unit: '', min: 1, max: 100, hint: 'Maximum WiFi reconnection attempts' },
            ].map(({ key, label, unit, min, max, hint }) => html`
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  ${label}${unit ? ` (${unit})` : ''}
                </label>
                <input
                  type="number"
                  value=${formData[key] || ''}
                  onInput=${(e) => updateFormData(key, parseInt(e.target.value))}
                  min=${min}
                  max=${max}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">${hint}</p>
              </div>
            `)}
          </div>
          <button
            onClick=${() => saveConfig('timing', {
              sensor_update_interval: formData.sensor_update_interval,
              history_update_interval: formData.history_update_interval,
              pid_update_interval: formData.pid_update_interval,
              connectivity_check_interval: formData.connectivity_check_interval,
              pid_config_write_interval: formData.pid_config_write_interval,
              wifi_connect_timeout: formData.wifi_connect_timeout,
              system_watchdog_timeout: formData.system_watchdog_timeout,
              wifi_watchdog_timeout: formData.wifi_watchdog_timeout,
              max_reconnect_attempts: formData.max_reconnect_attempts,
            })}
            disabled=${saving}
            class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${saving ? 'Saving...' : 'Save Timing Settings'}
          </button>
        </div>
      `}

      <!-- Webhook Tab -->
      ${activeTab === 'webhook' && html`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🔔</span>
            <span>Webhook Configuration</span>
          </h2>
          <div class="space-y-4">
            <div class="flex items-center gap-3">
              <input
                type="checkbox"
                id="webhook_enabled"
                checked=${formData.webhook_enabled || false}
                onChange=${(e) => updateFormData('webhook_enabled', e.target.checked)}
                class="w-4 h-4 rounded"
              />
              <label for="webhook_enabled" class="text-sm font-medium text-gray-700 dark:text-gray-300">
                Enable Webhook
              </label>
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Webhook URL
              </label>
              <input
                type="url"
                value=${formData.webhook_url || ''}
                onInput=${(e) => updateFormData('webhook_url', e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="https://maker.ifttt.com/trigger/event/with/key/YOUR_KEY"
              />
            </div>
            <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  Low Temperature Threshold (°C)
                </label>
                <input
                  type="number"
                  step="0.5"
                  min="-20"
                  max="50"
                  value=${formData.webhook_temp_low || 15.0}
                  onInput=${(e) => updateFormData('webhook_temp_low', parseFloat(e.target.value))}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  High Temperature Threshold (°C)
                </label>
                <input
                  type="number"
                  step="0.5"
                  min="-20"
                  max="50"
                  value=${formData.webhook_temp_high || 30.0}
                  onInput=${(e) => updateFormData('webhook_temp_high', parseFloat(e.target.value))}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
            </div>
            <div class="flex gap-3">
              <button
                onClick=${() => saveConfig('webhook', {
                  enabled: formData.webhook_enabled,
                  url: formData.webhook_url,
                  temp_low_threshold: formData.webhook_temp_low,
                  temp_high_threshold: formData.webhook_temp_high,
                })}
                disabled=${saving}
                class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
              >
                ${saving ? 'Saving...' : 'Save Webhook Settings'}
              </button>
              <button
                onClick=${testWebhook}
                disabled=${!formData.webhook_url}
                class="px-4 py-2 bg-yellow-500 hover:bg-yellow-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
              >
                Test Webhook
              </button>
            </div>
          </div>
        </div>
      `}

      <!-- Actions Bar -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>⚙️</span>
          <span>Device Actions</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <button
            onClick=${handleExportConfig}
            class="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>📥</span>
            <span>Export Configuration</span>
          </button>
          <button
            onClick=${handleImportConfig}
            class="px-4 py-2 bg-yellow-500 hover:bg-yellow-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>📤</span>
            <span>Import Configuration</span>
          </button>
          <button
            onClick=${handleReboot}
            class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>🔄</span>
            <span>Reboot Device</span>
          </button>
          <button
            onClick=${() => setShowResetModal(true)}
            class="px-4 py-2 bg-red-500 hover:bg-red-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>⚠️</span>
            <span>Factory Reset</span>
          </button>
        </div>
      </div>

      <!-- Factory Reset Modal -->
      <${FactoryResetModal}
        isOpen=${showResetModal}
        onClose=${() => setShowResetModal(false)}
        onConfirm=${handleFactoryReset}
      />

      <!-- Configuration Wizard -->
      <${ConfigWizard}
        isOpen=${showWizard}
        onClose=${() => setShowWizard(false)}
        onComplete=${() => {
          setShowWizard(false);
          fetchConfig();
        }}
      />
    </div>
  `;
}
