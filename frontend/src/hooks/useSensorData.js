import { useState, useEffect, useCallback } from 'preact/hooks';

/**
 * Custom hook for fetching real-time sensor data
 * Implements polling with error handling and recovery
 *
 * @param {number} refreshInterval - Polling interval in ms (default 5000)
 * @returns {Object} { data, loading, error, refetch }
 */
export function useSensorData(refreshInterval = 5000) {
  const [data, setData] = useState({
    temperature: 0,
    humidity: 0,
    pressure: 0,
    valve: 0,
    setpoint: 22.0,
  });
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
    // Initial fetch
    fetchData();

    // Set up polling interval
    const interval = setInterval(fetchData, refreshInterval);

    // Cleanup on unmount
    return () => clearInterval(interval);
  }, [fetchData, refreshInterval]);

  return {
    data,
    loading,
    error,
    refetch: fetchData,
  };
}
