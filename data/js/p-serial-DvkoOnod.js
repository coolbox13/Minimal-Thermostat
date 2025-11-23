import{_ as e,d as t,A as a,y as s}from"./v-preact-DZQM6r13.js";import{h as r}from"./v-misc-DwoQrUSZ.js";const l=r.bind(e);function i(){const[e,r]=t([]),[i,o]=t(!1),[n,d]=t(!0),c=a(null),g=a(null);return s(()=>{const e=`${"https:"===window.location.protocol?"wss:":"ws:"}//${window.location.host}/ws/serial`,t=new WebSocket(e);return g.current=t,t.onopen=()=>{o(!0),r([])},t.onmessage=e=>{const t=e.data;t&&t.trim()&&r(e=>[...e,t].slice(-500))},t.onerror=e=>{o(!1)},t.onclose=()=>{o(!1),setTimeout(()=>{g.current,WebSocket.CLOSED},3e3)},()=>{t.readyState!==WebSocket.OPEN&&t.readyState!==WebSocket.CONNECTING||t.close()}},[]),s(()=>{n&&c.current&&(c.current.scrollTop=c.current.scrollHeight)},[e,n]),l`
    <div class="space-y-4">
      <!-- Header with Controls -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-4">
        <div class="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
          <!-- Connection Status -->
          <div class="flex items-center gap-3">
            <div class="${i?"bg-green-100 dark:bg-green-900/20 text-green-700 dark:text-green-300":"bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300"} px-3 py-2 rounded-lg font-medium flex items-center gap-2">
              <div class="${i?"bg-green-500":"bg-gray-500"} w-2 h-2 rounded-full animate-pulse"></div>
              <span>${i?"Connected":"Disconnected"}</span>
            </div>
            <div class="text-sm text-gray-600 dark:text-gray-400">
              ${e.length} lines
            </div>
          </div>

          <!-- Controls -->
          <div class="flex gap-2">
            <label class="flex items-center gap-2 px-3 py-2 bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 rounded-lg font-medium cursor-pointer hover:bg-gray-200 dark:hover:bg-gray-600 transition-all">
              <input
                type="checkbox"
                checked=${n}
                onChange=${e=>d(e.target.checked)}
                class="rounded"
              />
              <span>Auto-scroll</span>
            </label>
            <button
              onClick=${()=>{r([])}}
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
          ref=${c}
          class="h-[300px] sm:h-[400px] md:h-[600px] overflow-y-auto text-green-400 leading-relaxed"
        >
          ${0===e.length?l`
            <div class="text-center text-gray-500 py-12">
              <p>Waiting for serial output...</p>
              <p class="text-xs mt-2 opacity-70">Output will appear here when available</p>
            </div>
          `:e.map((e,t)=>l`
            <div key=${t} class="hover:bg-gray-800/50 px-2 py-1">
              <span class="text-gray-600 select-none">${(t+1).toString().padStart(4,"0")}</span>
              <span class="ml-3">${e}</span>
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
  `}export{i as S};
