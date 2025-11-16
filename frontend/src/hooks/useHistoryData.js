import { useState, useEffect } from 'preact/hooks';

/**
 * Custom hook for fetching and managing historical sensor data
 * Implements polling strategy from GRAPH_VISUALIZATION_REFACTOR.md
 *
 * @param {number} refreshInterval - Polling interval in ms (default 30000)
 * @returns {Object} { data, loading, error, refetch }
 */
export function useHistoryData(refreshInterval = 30000) {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchData = async () => {
    try {
      const response = await fetch('/api/history?maxPoints=0');

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }

      const json = await response.json();

      // Validate data structure
      if (!json.timestamps || !json.temperatures) {
        throw new Error('Invalid data format received from API');
      }

      setData({
        timestamps: json.timestamps || [],
        temperatures: json.temperatures || [],
        humidities: json.humidities || [],
        pressures: json.pressures || [],
        valvePositions: json.valvePositions || [],
        count: json.timestamps?.length || 0,
      });
      setError(null);
    } catch (err) {
      console.error('Failed to fetch history data:', err);
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    // Initial fetch
    fetchData();

    // Set up polling interval
    const interval = setInterval(fetchData, refreshInterval);

    // Cleanup on unmount
    return () => clearInterval(interval);
  }, [refreshInterval]);

  return {
    data,
    loading,
    error,
    refetch: fetchData, // Allow manual refresh
  };
}
