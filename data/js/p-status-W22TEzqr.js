import{_ as e,d as a,y as t}from"./v-preact-DZQM6r13.js";import{h as s}from"./v-misc-DwoQrUSZ.js";const l=s.bind(e);function i(){const[e,s]=a(null),[i,d]=a(!0),[r,o]=a(null);t(()=>{n();const e=setInterval(n,1e4);return()=>clearInterval(e)},[]);const n=async()=>{try{const e=await fetch("/api/status"),a=await e.json();s(a),d(!1)}catch(e){o(e.message),d(!1)}};if(i)return l`
      <div class="space-y-6">
        ${[...Array(3)].map(()=>l`
          <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6 animate-pulse">
            <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-32 mb-4"></div>
            <div class="space-y-3">
              ${[...Array(4)].map(()=>l`
                <div class="flex justify-between">
                  <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-24"></div>
                  <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-32"></div>
                </div>
              `)}
            </div>
          </div>
        `)}
      </div>
    `;if(r)return l`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-red-500 text-5xl mb-4">⚠️</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            Failed to Load Status
          </h3>
          <p class="text-gray-600 dark:text-gray-400 mb-4">${r}</p>
          <button
            onClick=${n}
            class="px-4 py-2 bg-primary-500 text-white rounded-lg hover:bg-primary-600"
          >
            Retry
          </button>
        </div>
      </div>
    `;const g=e=>e?e<1e3?`${e}ms`:e<6e4?`${(e/1e3).toFixed(1)}s`:e<36e5?`${(e/6e4).toFixed(1)}min`:`${(e/36e5).toFixed(1)}h`:"--";return l`
    <div class="space-y-6">
      <!-- System Information -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          System Information
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Uptime",value:(e=>{if(!e)return"--";const a=Math.floor(e/86400),t=Math.floor(e%86400/3600),s=Math.floor(e%3600/60),l=e%60;return a>0?`${a}d ${t}h ${s}m`:t>0?`${t}h ${s}m ${l}s`:s>0?`${s}m ${l}s`:`${l}s`})(e?.system?.uptime)},{label:"Free Heap",value:e?.system?.free_heap?`${(e.system.free_heap/1024).toFixed(1)} KB`:"--"},{label:"Total Heap",value:e?.system?.total_heap?`${(e.system.total_heap/1024).toFixed(1)} KB`:"--"},{label:"Chip Model",value:e?.system?.chip_model||"--"},{label:"CPU Frequency",value:e?.system?.cpu_freq_mhz?`${e.system.cpu_freq_mhz} MHz`:"--"},{label:"Free Flash",value:e?.system?.free_flash_kb?`${e.system.free_flash_kb} KB`:"--"}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${a}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Network Information -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Network Status
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"WiFi SSID",value:e?.wifi?.ssid||"--"},{label:"IP Address",value:e?.wifi?.ip||"--"},{label:"Signal Strength",value:e?.wifi?.rssi?`${e.wifi.rssi} dBm`:"--"},{label:"Signal Quality",value:e?.wifi?.quality?`${e.wifi.quality}%`:"--"},{label:"MAC Address",value:e?.wifi?.mac||"--"},{label:"Connected",value:e?.wifi?.connected?"✅ Yes":"❌ No"}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white font-mono text-sm">${a}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- MQTT Status -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          MQTT Configuration
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Server",value:e?.mqtt?.server||"--"},{label:"Port",value:e?.mqtt?.port||"--"}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white font-mono text-sm">${a}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Timing Intervals -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Timing Intervals
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Sensor Update",value:g(e?.timing?.sensor_update_interval)},{label:"History Update",value:g(e?.timing?.history_update_interval)},{label:"PID Update",value:g(e?.timing?.pid_update_interval)},{label:"Connectivity Check",value:g(e?.timing?.connectivity_check_interval)},{label:"PID Config Write",value:g(e?.timing?.pid_config_write_interval)},{label:"WiFi Connect Timeout",value:e?.timing?.wifi_connect_timeout?`${e.timing.wifi_connect_timeout}s`:"--"},{label:"Max Reconnect Attempts",value:e?.timing?.max_reconnect_attempts||"--"},{label:"System Watchdog",value:g(e?.timing?.system_watchdog_timeout)},{label:"WiFi Watchdog",value:g(e?.timing?.wifi_watchdog_timeout)}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${a}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- PID Configuration -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          PID Configuration
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Kp",value:e?.pid?.kp?.toFixed(3)||"--"},{label:"Ki",value:e?.pid?.ki?.toFixed(3)||"--"},{label:"Kd",value:e?.pid?.kd?.toFixed(3)||"--"},{label:"Setpoint",value:e?.pid?.setpoint?`${e.pid.setpoint.toFixed(1)}°C`:"--"},{label:"Deadband",value:e?.pid?.deadband?`${e.pid.deadband.toFixed(2)}°C`:"--"},{label:"Valve Position",value:void 0!==e?.pid?.valve_position?`${e.pid.valve_position}%`:"--"},{label:"Adaptation Interval",value:e?.pid?.adaptation_interval?`${e.pid.adaptation_interval}s`:"--"}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${a}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Presets -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Temperature Presets
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Current Preset",value:e?.presets?.current||"--"},{label:"Eco",value:e?.presets?.eco?`${e.presets.eco.toFixed(1)}°C`:"--"},{label:"Comfort",value:e?.presets?.comfort?`${e.presets.comfort.toFixed(1)}°C`:"--"},{label:"Away",value:e?.presets?.away?`${e.presets.away.toFixed(1)}°C`:"--"},{label:"Sleep",value:e?.presets?.sleep?`${e.presets.sleep.toFixed(1)}°C`:"--"},{label:"Boost",value:e?.presets?.boost?`${e.presets.boost.toFixed(1)}°C`:"--"}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${a}</span>
            </div>
          `)}
        </div>
      </div>

      <!-- Diagnostics -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Diagnostics
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Last Reboot Reason",value:e?.diagnostics?.last_reboot_reason||"--"},{label:"Reboot Count",value:e?.diagnostics?.reboot_count||"--"},{label:"Watchdog Reboots",value:e?.diagnostics?.consecutive_watchdog_reboots||"--"}].map(({label:e,value:a})=>l`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${e}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${a}</span>
            </div>
          `)}
        </div>
      </div>
    </div>
  `}export{i as S};
