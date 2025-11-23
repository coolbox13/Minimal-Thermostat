import { h } from 'preact';
import { useState, useCallback, useEffect } from 'preact/hooks';
import htm from 'htm';
import { usePresets } from '../../hooks/usePresets.js';
import { useSensorData } from '../../hooks/useSensorData.js';
import toast from '../../utils/toast.js';

const html = htm.bind(h);

/**
 * ControlCard Component
 * Temperature setpoint and preset mode controls
 * Implements optimistic updates from PREACT_UX_IMPROVEMENTS.md
 */
export function ControlCard() {
  const { presetMode, presetConfig, applyPreset, getPresetTemperature } = usePresets();
  const { data: sensorData } = useSensorData(5000);

  const [setpoint, setSetpoint] = useState(22.0);
  const [isUpdating, setIsUpdating] = useState(false);
  const [error, setError] = useState(null);

  // Sync setpoint with backend when sensorData updates
  useEffect(() => {
    if (sensorData?.setpoint != null) {
      setSetpoint(sensorData.setpoint);
    }
  }, [sensorData?.setpoint]);

  // Optimistic preset change
  const handlePresetChange = useCallback(async (e) => {
    const newMode = e.target.value;

    try {
      await applyPreset(newMode);

      // Update setpoint slider to match preset temperature
      const presetTemp = getPresetTemperature(newMode);
      if (presetTemp) {
        setSetpoint(presetTemp);
      }

      toast.success(`Preset changed to ${newMode === 'none' ? 'Manual' : newMode}`);
    } catch (err) {
      toast.error('Failed to change preset mode');
    }
  }, [applyPreset, getPresetTemperature]);

  // Setpoint change with optimistic update
  const handleSetpointChange = useCallback(async () => {
    const previousSetpoint = sensorData?.setpoint || setpoint;
    setIsUpdating(true);
    setError(null);

    try {
      const response = await fetch('/api/setpoint', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `value=${setpoint}`,
      });

      if (!response.ok) {
        throw new Error('Failed to update setpoint');
      }

      toast.success(`Temperature set to ${setpoint.toFixed(1)}°C`);
    } catch (err) {
      // Rollback on error
      setSetpoint(previousSetpoint);
      setError(err.message);
      toast.error(`Failed to update setpoint: ${err.message}`);
    } finally {
      setIsUpdating(false);
    }
  }, [setpoint, sensorData]);

  // Get current preset temperature for display
  const currentPresetTemp = getPresetTemperature(presetMode);

  return html`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Controls
      </h2>

      <!-- Preset Mode Selector -->
      <div class="mb-6">
        <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
          Preset Mode
        </label>
        <div class="flex items-center gap-3">
          <select
            value=${presetMode}
            onChange=${handlePresetChange}
            class="flex-1 px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white focus:ring-2 focus:ring-primary-500 focus:border-transparent transition-all"
          >
            <option value="none">None (Manual)</option>
            <option value="eco">🌱 Eco</option>
            <option value="comfort">🏠 Comfort</option>
            <option value="away">✈️ Away</option>
            <option value="sleep">😴 Sleep</option>
            <option value="boost">🔥 Boost</option>
          </select>
          ${currentPresetTemp && html`
            <span class="text-sm font-medium text-primary-600 dark:text-primary-400 min-w-[60px] text-right">
              ${currentPresetTemp.toFixed(1)}°C
            </span>
          `}
        </div>
      </div>

      <!-- Temperature Setpoint Slider -->
      <div class="mb-6">
        <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
          Temperature Setpoint
        </label>

        <!-- Mobile-Optimized Slider with Floating Value -->
        <div class="relative mb-4">
          <input
            type="range"
            min="15"
            max="30"
            step="0.5"
            value=${setpoint}
            onInput=${(e) => setSetpoint(parseFloat(e.target.value))}
            class="w-full h-3 bg-gray-200 dark:bg-gray-700 rounded-lg appearance-none cursor-pointer slider-thumb"
            style="background: linear-gradient(to right, #1e88e5 0%, #1e88e5 ${((setpoint - 15) / 15) * 100}%, rgb(229 231 235) ${((setpoint - 15) / 15) * 100}%, rgb(229 231 235) 100%)"
          />

          <!-- Floating Value Display -->
          <div
            class="absolute -top-10 left-1/2 transform -translate-x-1/2 bg-primary-500 text-white px-3 py-1 rounded-lg text-lg font-bold shadow-lg"
            style="left: ${((setpoint - 15) / 15) * 100}%"
          >
            ${setpoint.toFixed(1)}°C
            <div class="absolute top-full left-1/2 transform -translate-x-1/2 w-0 h-0 border-l-4 border-r-4 border-t-4 border-l-transparent border-r-transparent border-t-primary-500"></div>
          </div>
        </div>

        <!-- Min/Max Labels -->
        <div class="flex justify-between text-xs text-gray-500 dark:text-gray-400 mb-4">
          <span>15°C</span>
          <span>30°C</span>
        </div>
      </div>

      <!-- Set Button -->
      <button
        onClick=${handleSetpointChange}
        disabled=${isUpdating}
        class="w-full px-4 py-3 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white font-medium rounded-lg transition-all duration-200 shadow-md hover:shadow-lg disabled:cursor-not-allowed"
      >
        ${isUpdating ? 'Updating...' : 'Set Temperature'}
      </button>

      <!-- Error Display -->
      ${error && html`
        <div class="mt-3 p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
          <p class="text-sm text-red-600 dark:text-red-400">${error}</p>
        </div>
      `}
    </div>
  `;
}
