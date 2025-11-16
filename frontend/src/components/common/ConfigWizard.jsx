import { h } from 'preact';
import { useState } from 'preact/hooks';
import htm from 'htm';
import { ValidatedInput } from './ValidatedInput.jsx';
import { Spinner } from './Spinner.jsx';
import {
  validateSSID,
  validateBroker,
  validatePort,
  validatePID,
} from '../../utils/validation.js';
import toast from '../../utils/toast.js';

const html = htm.bind(h);

/**
 * ConfigWizard Component
 * Step-by-step configuration wizard for first-time setup
 * Steps: WiFi → MQTT → Sensors (PID) → Review & Save
 */
export function ConfigWizard({ isOpen, onClose, onComplete }) {
  const [currentStep, setCurrentStep] = useState(0);
  const [saving, setSaving] = useState(false);

  // WiFi configuration state
  const [wifiConfig, setWifiConfig] = useState({
    ssid: '',
    password: '',
  });

  // MQTT configuration state
  const [mqttConfig, setMqttConfig] = useState({
    broker: '',
    port: '1883',
    username: '',
    password: '',
  });

  // PID configuration state
  const [pidConfig, setPidConfig] = useState({
    kp: '2.0',
    ki: '0.5',
    kd: '1.0',
  });

  const steps = [
    { id: 'wifi', title: 'WiFi Setup', icon: '📡' },
    { id: 'mqtt', title: 'MQTT Setup', icon: '📨' },
    { id: 'sensors', title: 'Sensor Setup', icon: '🌡️' },
    { id: 'review', title: 'Review & Save', icon: '✅' },
  ];

  // Validation state for each step
  const isWifiValid = () => {
    const ssidValid = validateSSID(wifiConfig.ssid).isValid;
    const passwordValid = wifiConfig.password.length >= 8;
    return ssidValid && passwordValid;
  };

  const isMqttValid = () => {
    const brokerValid = validateBroker(mqttConfig.broker).isValid;
    const portValid = validatePort(mqttConfig.port).isValid;
    return brokerValid && portValid;
  };

  const isPidValid = () => {
    const kpValid = validatePID(pidConfig.kp, 'kp').isValid;
    const kiValid = validatePID(pidConfig.ki, 'ki').isValid;
    const kdValid = validatePID(pidConfig.kd, 'kd').isValid;
    return kpValid && kiValid && kdValid;
  };

  const canProceed = () => {
    switch (currentStep) {
      case 0:
        return isWifiValid();
      case 1:
        return isMqttValid();
      case 2:
        return isPidValid();
      case 3:
        return true; // Review step
      default:
        return false;
    }
  };

  const handleNext = () => {
    if (currentStep < steps.length - 1) {
      setCurrentStep(currentStep + 1);
    }
  };

  const handleBack = () => {
    if (currentStep > 0) {
      setCurrentStep(currentStep - 1);
    }
  };

  const handleSave = async () => {
    setSaving(true);
    try {
      // Save WiFi configuration
      await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          section: 'wifi',
          ssid: wifiConfig.ssid,
          password: wifiConfig.password,
        }),
      });

      // Save MQTT configuration
      await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          section: 'mqtt',
          broker: mqttConfig.broker,
          port: parseInt(mqttConfig.port),
          username: mqttConfig.username,
          password: mqttConfig.password,
        }),
      });

      // Save PID configuration
      await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          section: 'pid',
          kp: parseFloat(pidConfig.kp),
          ki: parseFloat(pidConfig.ki),
          kd: parseFloat(pidConfig.kd),
        }),
      });

      toast.success('Configuration saved successfully!');
      setSaving(false);
      onComplete?.();
      onClose?.();
    } catch (err) {
      toast.error(`Failed to save configuration: ${err.message}`);
      setSaving(false);
    }
  };

  const handleSkip = () => {
    onClose?.();
  };

  if (!isOpen) return null;

  return html`
    <div class="fixed inset-0 z-50 overflow-y-auto">
      <!-- Backdrop -->
      <div class="fixed inset-0 bg-black bg-opacity-50 transition-opacity"></div>

      <!-- Modal -->
      <div class="flex min-h-full items-center justify-center p-4">
        <div class="relative bg-white dark:bg-gray-800 rounded-2xl shadow-2xl max-w-3xl w-full p-8 animate-slideUp">
          <!-- Header -->
          <div class="mb-8">
            <h2 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
              Setup Wizard
            </h2>
            <p class="text-gray-600 dark:text-gray-400">
              Let's configure your ESP32 Thermostat step by step
            </p>
          </div>

          <!-- Progress Indicator -->
          <div class="mb-8">
            <div class="flex justify-between mb-4">
              ${steps.map(
                (step, index) => html`
                  <div
                    class="flex flex-col items-center flex-1"
                    key=${step.id}
                  >
                    <!-- Step Icon -->
                    <div
                      class="${index <= currentStep
                        ? 'bg-primary-500 text-white'
                        : 'bg-gray-200 dark:bg-gray-700 text-gray-500 dark:text-gray-400'}
                      w-12 h-12 rounded-full flex items-center justify-center text-xl font-bold transition-all duration-300 mb-2"
                    >
                      ${index < currentStep
                        ? '✓'
                        : index === currentStep
                        ? step.icon
                        : index + 1}
                    </div>
                    <!-- Step Title -->
                    <span
                      class="${index <= currentStep
                        ? 'text-gray-900 dark:text-white font-semibold'
                        : 'text-gray-500 dark:text-gray-400'}
                      text-sm text-center"
                    >
                      ${step.title}
                    </span>
                    <!-- Connector Line -->
                    ${index < steps.length - 1 &&
                    html`
                      <div
                        class="${index < currentStep
                          ? 'bg-primary-500'
                          : 'bg-gray-200 dark:bg-gray-700'}
                        h-1 w-full absolute top-6 left-1/2 -z-10 transition-all duration-300"
                        style="width: calc(100% / ${steps.length} - 3rem)"
                      ></div>
                    `}
                  </div>
                `
              )}
            </div>
          </div>

          <!-- Step Content -->
          <div class="mb-8 min-h-[400px]">
            ${currentStep === 0 && renderWifiStep()}
            ${currentStep === 1 && renderMqttStep()}
            ${currentStep === 2 && renderSensorsStep()}
            ${currentStep === 3 && renderReviewStep()}
          </div>

          <!-- Navigation Buttons -->
          <div class="flex justify-between items-center pt-6 border-t border-gray-200 dark:border-gray-700">
            <div>
              ${currentStep > 0 &&
              html`
                <button
                  onClick=${handleBack}
                  disabled=${saving}
                  class="px-6 py-2 text-gray-700 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg font-medium transition-all disabled:opacity-50"
                >
                  ← Back
                </button>
              `}
            </div>

            <div class="flex gap-3">
              <button
                onClick=${handleSkip}
                disabled=${saving}
                class="px-6 py-2 text-gray-600 dark:text-gray-400 hover:text-gray-900 dark:hover:text-white font-medium transition-all disabled:opacity-50"
              >
                Skip Setup
              </button>

              ${currentStep < steps.length - 1
                ? html`
                    <button
                      onClick=${handleNext}
                      disabled=${!canProceed() || saving}
                      class="px-6 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 disabled:cursor-not-allowed text-white rounded-lg font-medium transition-all"
                    >
                      Next →
                    </button>
                  `
                : html`
                    <button
                      onClick=${handleSave}
                      disabled=${saving}
                      class="px-6 py-2 bg-green-500 hover:bg-green-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all flex items-center gap-2"
                    >
                      ${saving
                        ? html`<${Spinner} size="sm" color="white" />
                            <span>Saving...</span>`
                        : html`<span>Save Configuration</span>`}
                    </button>
                  `}
            </div>
          </div>
        </div>
      </div>
    </div>
  `;

  // WiFi Setup Step
  function renderWifiStep() {
    return html`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>WiFi Setup:</strong> Configure your WiFi network credentials
            to connect your ESP32 to the internet. Make sure you're using a
            2.4GHz network as the ESP32 doesn't support 5GHz.
          </p>
        </div>

        <${ValidatedInput}
          label="WiFi Network Name (SSID)"
          type="text"
          value=${wifiConfig.ssid}
          onChange=${(value) => setWifiConfig({ ...wifiConfig, ssid: value })}
          validator=${validateSSID}
          placeholder="Enter your WiFi network name"
          required=${true}
          helpText="Maximum 32 characters"
        />

        <div>
          <label
            class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2"
          >
            WiFi Password
          </label>
          <input
            type="password"
            value=${wifiConfig.password}
            onInput=${(e) =>
              setWifiConfig({ ...wifiConfig, password: e.target.value })}
            class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border ${wifiConfig
              .password.length >= 8
              ? 'border-green-500'
              : wifiConfig.password.length > 0
              ? 'border-red-500'
              : 'border-gray-300 dark:border-gray-600'} rounded-lg text-gray-900 dark:text-white"
            placeholder="Enter WiFi password"
          />
          <p class="mt-1 text-xs text-gray-500 dark:text-gray-400">
            Minimum 8 characters
          </p>
          ${wifiConfig.password.length > 0 &&
          wifiConfig.password.length < 8 &&
          html`
            <p class="mt-1 text-xs text-red-600 dark:text-red-400">
              Password must be at least 8 characters
            </p>
          `}
        </div>
      </div>
    `;
  }

  // MQTT Setup Step
  function renderMqttStep() {
    return html`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>MQTT Setup:</strong> Configure your MQTT broker to enable
            communication with Home Assistant and other smart home platforms.
            You can skip this step if you don't use MQTT.
          </p>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div class="md:col-span-2">
            <${ValidatedInput}
              label="MQTT Broker Address"
              type="text"
              value=${mqttConfig.broker}
              onChange=${(value) =>
                setMqttConfig({ ...mqttConfig, broker: value })}
              validator=${validateBroker}
              placeholder="mqtt.example.com or 192.168.1.100"
              required=${true}
              helpText="Hostname or IP address of your MQTT broker"
            />
          </div>

          <${ValidatedInput}
            label="MQTT Port"
            type="number"
            value=${mqttConfig.port}
            onChange=${(value) => setMqttConfig({ ...mqttConfig, port: value })}
            validator=${validatePort}
            required=${true}
            helpText="Usually 1883 for MQTT"
          />

          <div>
            <label
              class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2"
            >
              Username (Optional)
            </label>
            <input
              type="text"
              value=${mqttConfig.username}
              onInput=${(e) =>
                setMqttConfig({ ...mqttConfig, username: e.target.value })}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="MQTT username"
            />
          </div>

          <div class="md:col-span-2">
            <label
              class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2"
            >
              Password (Optional)
            </label>
            <input
              type="password"
              value=${mqttConfig.password}
              onInput=${(e) =>
                setMqttConfig({ ...mqttConfig, password: e.target.value })}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="MQTT password"
            />
          </div>
        </div>
      </div>
    `;
  }

  // Sensors Setup Step
  function renderSensorsStep() {
    return html`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>PID Tuning:</strong> Configure the PID controller parameters
            for optimal temperature regulation. The default values work well for
            most setups, but you can fine-tune them later based on your specific
            radiator and room characteristics.
          </p>
        </div>

        <div class="bg-yellow-50 dark:bg-yellow-900/20 border border-yellow-200 dark:border-yellow-800 rounded-lg p-4">
          <p class="text-sm text-yellow-700 dark:text-yellow-300">
            <strong>Note:</strong> Kp controls proportional response, Ki reduces
            steady-state error, and Kd dampens oscillations. Start with default
            values and adjust if needed.
          </p>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
          <${ValidatedInput}
            label="Kp (Proportional)"
            type="number"
            step="0.1"
            value=${pidConfig.kp}
            onChange=${(value) => setPidConfig({ ...pidConfig, kp: value })}
            validator=${(value) => validatePID(value, 'kp')}
            required=${true}
            helpText="Range: 0.1 - 100"
          />

          <${ValidatedInput}
            label="Ki (Integral)"
            type="number"
            step="0.01"
            value=${pidConfig.ki}
            onChange=${(value) => setPidConfig({ ...pidConfig, ki: value })}
            validator=${(value) => validatePID(value, 'ki')}
            required=${true}
            helpText="Range: 0 - 10"
          />

          <${ValidatedInput}
            label="Kd (Derivative)"
            type="number"
            step="0.01"
            value=${pidConfig.kd}
            onChange=${(value) => setPidConfig({ ...pidConfig, kd: value })}
            validator=${(value) => validatePID(value, 'kd')}
            required=${true}
            helpText="Range: 0 - 10"
          />
        </div>
      </div>
    `;
  }

  // Review & Save Step
  function renderReviewStep() {
    return html`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg p-4">
          <p class="text-sm text-green-700 dark:text-green-300">
            <strong>Review Your Configuration:</strong> Please review your
            settings before saving. You can always change these later in the
            Configuration page.
          </p>
        </div>

        <!-- WiFi Configuration Review -->
        <div class="bg-white dark:bg-gray-700 rounded-lg p-6 border border-gray-200 dark:border-gray-600">
          <h3
            class="text-lg font-semibold text-gray-900 dark:text-white mb-4 flex items-center gap-2"
          >
            <span>📡</span>
            <span>WiFi Configuration</span>
          </h3>
          <div class="space-y-2">
            <div class="flex justify-between">
              <span class="text-gray-600 dark:text-gray-400">Network:</span>
              <span class="text-gray-900 dark:text-white font-medium"
                >${wifiConfig.ssid}</span
              >
            </div>
            <div class="flex justify-between">
              <span class="text-gray-600 dark:text-gray-400">Password:</span>
              <span class="text-gray-900 dark:text-white font-medium"
                >${'•'.repeat(wifiConfig.password.length)}</span
              >
            </div>
          </div>
        </div>

        <!-- MQTT Configuration Review -->
        <div class="bg-white dark:bg-gray-700 rounded-lg p-6 border border-gray-200 dark:border-gray-600">
          <h3
            class="text-lg font-semibold text-gray-900 dark:text-white mb-4 flex items-center gap-2"
          >
            <span>📨</span>
            <span>MQTT Configuration</span>
          </h3>
          <div class="space-y-2">
            <div class="flex justify-between">
              <span class="text-gray-600 dark:text-gray-400">Broker:</span>
              <span class="text-gray-900 dark:text-white font-medium"
                >${mqttConfig.broker}:${mqttConfig.port}</span
              >
            </div>
            ${mqttConfig.username &&
            html`
              <div class="flex justify-between">
                <span class="text-gray-600 dark:text-gray-400">Username:</span>
                <span class="text-gray-900 dark:text-white font-medium"
                  >${mqttConfig.username}</span
                >
              </div>
            `}
          </div>
        </div>

        <!-- PID Configuration Review -->
        <div class="bg-white dark:bg-gray-700 rounded-lg p-6 border border-gray-200 dark:border-gray-600">
          <h3
            class="text-lg font-semibold text-gray-900 dark:text-white mb-4 flex items-center gap-2"
          >
            <span>🎛️</span>
            <span>PID Configuration</span>
          </h3>
          <div class="grid grid-cols-3 gap-4">
            <div class="text-center">
              <div class="text-gray-600 dark:text-gray-400 text-sm mb-1">
                Kp
              </div>
              <div class="text-gray-900 dark:text-white font-bold text-xl">
                ${pidConfig.kp}
              </div>
            </div>
            <div class="text-center">
              <div class="text-gray-600 dark:text-gray-400 text-sm mb-1">
                Ki
              </div>
              <div class="text-gray-900 dark:text-white font-bold text-xl">
                ${pidConfig.ki}
              </div>
            </div>
            <div class="text-center">
              <div class="text-gray-600 dark:text-gray-400 text-sm mb-1">
                Kd
              </div>
              <div class="text-gray-900 dark:text-white font-bold text-xl">
                ${pidConfig.kd}
              </div>
            </div>
          </div>
        </div>
      </div>
    `;
  }
}
