import { useState, useEffect } from 'preact/hooks';

/**
 * Custom hook for fetching system information (version, uptime, etc.)
 * Fetches once on mount - system info doesn't change frequently
 *
 * @returns {Object} { version, loading, error }
 */
export function useSystemInfo() {
  const [version, setVersion] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    const abortController = new AbortController();

    async function fetchSystemInfo() {
      try {
        const response = await fetch('/api/status', { signal: abortController.signal });

        if (!response.ok) {
          throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }

        const json = await response.json();
        setVersion(json.system?.firmware_version ?? 'Unknown');
        setError(null);
      } catch (err) {
        if (err.name === 'AbortError') {
          return;
        }
        console.error('Failed to fetch system info:', err);
        setError(err.message);
      } finally {
        setLoading(false);
      }
    }

    fetchSystemInfo();

    return () => {
      abortController.abort();
    };
  }, []);

  return {
    version,
    loading,
    error,
  };
}
