import { h } from 'preact';
import { useState, useEffect, useCallback } from 'preact/hooks';
import htm from 'htm';
import toast from '../../utils/toast.js';

const html = htm.bind(h);

/**
 * ManualOverrideCard Component
 * Manual valve position control with countdown timer
 * Implements optimistic updates from PREACT_UX_IMPROVEMENTS.md
 */
export function ManualOverrideCard() {
  const [overrideEnabled, setOverrideEnabled] = useState(false);
  const [valvePosition, setValvePosition] = useState(0);
  const [isUpdating, setIsUpdating] = useState(false);
  const [error, setError] = useState(null);
  const [remainingTime, setRemainingTime] = useState(null);

  // Fetch current override state
  useEffect(() => {
    const fetchOverrideState = async () => {
      try {
        const response = await fetch('/api/manual-override');
        const data = await response.json();
        setOverrideEnabled(data.enabled || false);
        setValvePosition(data.position || 0);
        setRemainingTime(data.remainingSeconds || null);
      } catch (err) {
        // Silently fail initial fetch
      }
    };

    fetchOverrideState();
    const interval = setInterval(fetchOverrideState, 5000);
    return () => clearInterval(interval);
  }, []);

  // Countdown timer
  useEffect(() => {
    if (!overrideEnabled || !remainingTime) return;

    const interval = setInterval(() => {
      setRemainingTime(prev => {
        if (prev <= 1) {
          setOverrideEnabled(false);
          return null;
        }
        return prev - 1;
      });
    }, 1000);

    return () => clearInterval(interval);
  }, [overrideEnabled, remainingTime]);

  // Toggle override mode
  const handleToggleOverride = useCallback(async () => {
    const newState = !overrideEnabled;
    const previousState = overrideEnabled;

    // Optimistic update
    setOverrideEnabled(newState);
    setIsUpdating(true);
    setError(null);

    try {
      const params = new URLSearchParams();
      params.append('enabled', newState);
      if (newState) {
        params.append('position', valvePosition);
      }

      const response = await fetch('/api/manual-override', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params.toString(),
      });

      if (!response.ok) {
        throw new Error('Failed to toggle override');
      }

      if (newState) {
        setRemainingTime(1800); // 30 minutes default
        toast.success('Manual override enabled');
      } else {
        setRemainingTime(null);
        setValvePosition(0);
        toast.info('Manual override disabled');
      }
    } catch (err) {
      // Rollback on error
      setOverrideEnabled(previousState);
      setError(err.message);
      toast.error(`Failed to toggle override: ${err.message}`);
    } finally {
      setIsUpdating(false);
    }
  }, [overrideEnabled, valvePosition]);

  // Update valve position
  const handleSetValvePosition = useCallback(async () => {
    setIsUpdating(true);
    setError(null);

    try {
      const params = new URLSearchParams();
      params.append('enabled', true);
      params.append('position', valvePosition);

      const response = await fetch('/api/manual-override', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params.toString(),
      });

      if (!response.ok) {
        throw new Error('Failed to update valve position');
      }

      toast.success(`Valve position set to ${valvePosition}%`);
    } catch (err) {
      setError(err.message);
      toast.error(`Failed to update valve position: ${err.message}`);
    } finally {
      setIsUpdating(false);
    }
  }, [valvePosition]);

  // Format remaining time
  const formatTime = (seconds) => {
    if (!seconds) return '';
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  };

  return html`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Manual Override
      </h2>

      <!-- Valve Position Slider -->
      <div class="mb-4">
        <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
          Valve Position
        </label>

        <div class="relative mb-4">
          <input
            type="range"
            min="0"
            max="100"
            step="5"
            value=${valvePosition}
            onInput=${(e) => setValvePosition(parseInt(e.target.value))}
            disabled=${!overrideEnabled}
            class="w-full h-3 bg-gray-200 dark:bg-gray-700 rounded-lg appearance-none cursor-pointer disabled:opacity-50 disabled:cursor-not-allowed"
            style="background: linear-gradient(to right, #ff9800 0%, #ff9800 ${valvePosition}%, rgb(229 231 235) ${valvePosition}%, rgb(229 231 235) 100%)"
          />

          <!-- Floating Value Display -->
          ${overrideEnabled && html`
            <div
              class="absolute -top-10 left-1/2 transform -translate-x-1/2 bg-orange-500 text-white px-3 py-1 rounded-lg text-lg font-bold shadow-lg"
              style="left: ${valvePosition}%"
            >
              ${valvePosition}%
              <div class="absolute top-full left-1/2 transform -translate-x-1/2 w-0 h-0 border-l-4 border-r-4 border-t-4 border-l-transparent border-r-transparent border-t-orange-500"></div>
            </div>
          `}
        </div>

        <!-- Min/Max Labels -->
        <div class="flex justify-between text-xs text-gray-500 dark:text-gray-400 mb-4">
          <span>0%</span>
          <span>100%</span>
        </div>
      </div>

      <!-- Toggle and Set Buttons -->
      <div class="space-y-2">
        <button
          onClick=${handleToggleOverride}
          disabled=${isUpdating}
          class="${overrideEnabled
            ? 'bg-red-500 hover:bg-red-600'
            : 'bg-orange-500 hover:bg-orange-600'
          } w-full px-4 py-3 disabled:bg-gray-400 text-white font-medium rounded-lg transition-all duration-200 shadow-md hover:shadow-lg disabled:cursor-not-allowed"
        >
          ${overrideEnabled ? 'Disable Manual Control' : 'Enable Manual Control'}
        </button>

        ${overrideEnabled && html`
          <button
            onClick=${handleSetValvePosition}
            disabled=${isUpdating}
            class="w-full px-4 py-2 bg-gray-600 hover:bg-gray-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-all duration-200 disabled:cursor-not-allowed"
          >
            ${isUpdating ? 'Updating...' : 'Update Position'}
          </button>
        `}
      </div>

      <!-- Timer Display -->
      ${overrideEnabled && remainingTime && html`
        <div class="mt-3 p-3 bg-orange-50 dark:bg-orange-900/20 border border-orange-200 dark:border-orange-800 rounded-lg">
          <p class="text-sm text-orange-700 dark:text-orange-300 flex items-center gap-2">
            <span>⏱️</span>
            <span>Override active for ${formatTime(remainingTime)}</span>
          </p>
        </div>
      `}

      <!-- Error Display -->
      ${error && html`
        <div class="mt-3 p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
          <p class="text-sm text-red-600 dark:text-red-400">${error}</p>
        </div>
      `}
    </div>
  `;
}
