import { useState, useEffect, useCallback } from 'preact/hooks';

/**
 * Custom hook for fetching real-time sensor data
 * Implements polling with error handling and recovery
 *
 * @param {number} refreshInterval - Polling interval in ms (default 5000)
 * @returns {Object} { data, loading, error, refetch }
 */
export function useSensorData(refreshInterval = 5000) {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchData = useCallback(async () => {
    try {
      const response = await fetch('/api/sensor');

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }

      const json = await response.json();

      setData({
        temperature: json.temperature ?? 0,
        humidity: json.humidity ?? 0,
        pressure: json.pressure ?? 0,
        valve: json.valve ?? 0,
        setpoint: json.setpoint ?? 22.0,
      });
      setError(null);
    } catch (err) {
      console.error('Failed to fetch sensor data:', err);
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    let isMounted = true;
    let timerId = null;

    // Recursive polling pattern - only schedule next poll after current one completes
    const poll = async () => {
      await fetchData();
      if (isMounted) {
        timerId = setTimeout(poll, refreshInterval);
      }
    };

    // Start initial poll
    poll();

    // Cleanup on unmount
    return () => {
      isMounted = false;
      if (timerId) clearTimeout(timerId);
    };
  }, [refreshInterval]);

  return {
    data,
    loading,
    error,
    refetch: fetchData,
  };
}
