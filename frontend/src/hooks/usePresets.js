import { useState, useEffect, useCallback } from 'preact/hooks';

/**
 * Custom hook for managing preset modes
 * Handles fetching, applying, and saving presets
 * Implements optimistic updates for instant UI feedback
 *
 * @returns {Object} { presetMode, presetConfig, applyPreset, loading, error }
 */
export function usePresets() {
  const [presetMode, setPresetMode] = useState('none');
  const [presetConfig, setPresetConfig] = useState({
    eco: 18.0,
    comfort: 22.0,
    away: 16.0,
    sleep: 19.0,
    boost: 24.0,
  });
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  // Fetch preset configuration from API
  const fetchPresetConfig = useCallback(async () => {
    try {
      const response = await fetch('/api/config');

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }

      const data = await response.json();

      if (data.presets) {
        setPresetMode(data.presets.current || 'none');
        setPresetConfig({
          eco: data.presets.eco ?? 18.0,
          comfort: data.presets.comfort ?? 22.0,
          away: data.presets.away ?? 16.0,
          sleep: data.presets.sleep ?? 19.0,
          boost: data.presets.boost ?? 24.0,
        });
      }
      setError(null);
    } catch (err) {
      console.error('Failed to fetch preset config:', err);
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }, []);

  // Apply a preset mode (with optimistic update)
  const applyPreset = useCallback(async (mode) => {
    const previousMode = presetMode;

    // Optimistic update - update UI immediately
    setPresetMode(mode);

    try {
      const response = await fetch('/api/preset', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ mode }),
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }

      setError(null);
      return true;
    } catch (err) {
      console.error('Failed to apply preset:', err);

      // Rollback on error
      setPresetMode(previousMode);
      setError(err.message);

      return false;
    }
  }, [presetMode]);

  // Get temperature for a specific preset
  const getPresetTemperature = useCallback((mode) => {
    if (mode === 'none') return null;
    return presetConfig[mode] ?? null;
  }, [presetConfig]);

  useEffect(() => {
    fetchPresetConfig();
  }, [fetchPresetConfig]);

  return {
    presetMode,
    presetConfig,
    applyPreset,
    getPresetTemperature,
    loading,
    error,
    refetch: fetchPresetConfig,
  };
}
