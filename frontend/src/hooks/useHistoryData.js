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
    refetch: fetchData, // Allow manual refresh
  };
}
