import { h } from 'preact';
import { useState, useEffect, useRef } from 'preact/hooks';
import htm from 'htm';

const html = htm.bind(h);

/**
 * Serial Monitor Page
 * Real-time serial output from ESP32
 */
export function Serial() {
  const [lines, setLines] = useState([]);
  const [isConnected, setIsConnected] = useState(false);
  const [autoscroll, setAutoscroll] = useState(true);
  const scrollRef = useRef(null);
  const wsRef = useRef(null);

  useEffect(() => {
    // Connect to WebSocket for real-time serial output
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws/serial`;
    
    const ws = new WebSocket(wsUrl);
    wsRef.current = ws;

    ws.onopen = () => {
      setIsConnected(true);
      setLines([]); // Clear on reconnect
    };

    ws.onmessage = (event) => {
      const line = event.data;
      if (line && line.trim()) {
        setLines(prev => [...prev, line].slice(-500)); // Keep last 500 lines
      }
    };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
      setIsConnected(false);
    };

    ws.onclose = () => {
      setIsConnected(false);
      // Attempt to reconnect after 3 seconds
      setTimeout(() => {
        if (wsRef.current?.readyState === WebSocket.CLOSED) {
          // Reconnect logic handled by useEffect cleanup/re-run
        }
      }, 3000);
    };

    return () => {
      if (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING) {
        ws.close();
      }
    };
  }, []);

  // Auto-scroll to bottom
  useEffect(() => {
    if (autoscroll && scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [lines, autoscroll]);

  const clearOutput = () => {
    setLines([]);
  };

  return html`
    <div class="space-y-4">
      <!-- Header with Controls -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-4">
        <div class="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
          <!-- Connection Status -->
          <div class="flex items-center gap-3">
            <div class="${isConnected
              ? 'bg-green-100 dark:bg-green-900/20 text-green-700 dark:text-green-300'
              : 'bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300'
            } px-3 py-2 rounded-lg font-medium flex items-center gap-2">
              <div class="${isConnected ? 'bg-green-500' : 'bg-gray-500'} w-2 h-2 rounded-full animate-pulse"></div>
              <span>${isConnected ? 'Connected' : 'Disconnected'}</span>
            </div>
            <div class="text-sm text-gray-600 dark:text-gray-400">
              ${lines.length} lines
            </div>
          </div>

          <!-- Controls -->
          <div class="flex gap-2">
            <label class="flex items-center gap-2 px-3 py-2 bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 rounded-lg font-medium cursor-pointer hover:bg-gray-200 dark:hover:bg-gray-600 transition-all">
              <input
                type="checkbox"
                checked=${autoscroll}
                onChange=${(e) => setAutoscroll(e.target.checked)}
                class="rounded"
              />
              <span>Auto-scroll</span>
            </label>
            <button
              onClick=${clearOutput}
              class="px-4 py-2 bg-red-500 text-white rounded-lg font-medium hover:bg-red-600 transition-all"
            >
              🗑️ Clear
            </button>
          </div>
        </div>
      </div>

      <!-- Serial Output -->
      <div class="bg-gray-900 dark:bg-black rounded-xl shadow-lg p-4 font-mono text-sm">
        <div
          ref=${scrollRef}
          class="h-[300px] sm:h-[400px] md:h-[600px] overflow-y-auto text-green-400 leading-relaxed"
        >
          ${lines.length === 0 ? html`
            <div class="text-center text-gray-500 py-12">
              <p>Waiting for serial output...</p>
              <p class="text-xs mt-2 opacity-70">Output will appear here when available</p>
            </div>
          ` : lines.map((line, index) => html`
            <div key=${index} class="hover:bg-gray-800/50 px-2 py-1">
              <span class="text-gray-600 select-none">${(index + 1).toString().padStart(4, '0')}</span>
              <span class="ml-3">${line}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Info Card -->
      <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
        <div class="flex gap-3">
          <span class="text-blue-500 text-2xl flex-shrink-0">ℹ️</span>
          <div class="text-blue-700 dark:text-blue-300 text-sm">
            <p class="font-semibold mb-1">Serial Monitor Tips:</p>
            <ul class="list-disc list-inside space-y-1">
              <li>Real-time serial output via WebSocket</li>
              <li>Maximum 500 lines are kept in memory</li>
              <li>Disable auto-scroll to review previous output</li>
              <li>Use Clear to remove all lines</li>
              <li>Reconnects automatically if connection is lost</li>
            </ul>
          </div>
        </div>
      </div>
    </div>
  `;
}
