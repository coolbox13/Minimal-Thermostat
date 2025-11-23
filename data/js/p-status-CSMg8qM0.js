import{_ as l,d as e,y as a}from"./v-preact-BAP1wjwx.js";import{h as t}from"./v-misc-CJ9EBB9u.js";const i=t.bind(l);function d(){var l,t,d,s,o,n,r,v,u,g,c,b,p,m,y,x,h,f,w,$,k,_,F,C,S,I,j,P,M,T,A,D,q,W,K,R,B,U,H,z,N,Q;const[L,E]=e(null),[V,Y]=e(!0),[G,J]=e(null);a(()=>{O();const l=setInterval(O,1e4);return()=>clearInterval(l)},[]);const O=async()=>{try{const l=await fetch("/api/status"),e=await l.json();E(e),Y(!1)}catch(l){J(l.message),Y(!1)}};if(V)return i`
      <div class="space-y-6">
        ${[...Array(3)].map(()=>i`
          <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6 animate-pulse">
            <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-32 mb-4"></div>
            <div class="space-y-3">
              ${[...Array(4)].map(()=>i`
                <div class="flex justify-between">
                  <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-24"></div>
                  <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-32"></div>
                </div>
              `)}
            </div>
          </div>
        `)}
      </div>
    `;if(G)return i`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-red-500 text-5xl mb-4">⚠️</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            Failed to Load Status
          </h3>
          <p class="text-gray-600 dark:text-gray-400 mb-4">${G}</p>
          <button
            onClick=${O}
            class="px-4 py-2 bg-primary-500 text-white rounded-lg hover:bg-primary-600"
          >
            Retry
          </button>
        </div>
      </div>
    `;const X=l=>l?l<1e3?`${l}ms`:l<6e4?`${(l/1e3).toFixed(1)}s`:l<36e5?`${(l/6e4).toFixed(1)}min`:`${(l/36e5).toFixed(1)}h`:"--";return i`
    <div class="space-y-6">
      <!-- System Information -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          System Information
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${[{label:"Uptime",value:(l=>{if(!l)return"--";const e=Math.floor(l/86400),a=Math.floor(l%86400/3600),t=Math.floor(l%3600/60),i=l%60;return e>0?`${e}d ${a}h ${t}m`:a>0?`${a}h ${t}m ${i}s`:t>0?`${t}m ${i}s`:`${i}s`})(null==(l=null==L?void 0:L.system)?void 0:l.uptime)},{label:"Free Heap",value:(null==(t=null==L?void 0:L.system)?void 0:t.free_heap)?`${(L.system.free_heap/1024).toFixed(1)} KB`:"--"},{label:"Total Heap",value:(null==(d=null==L?void 0:L.system)?void 0:d.total_heap)?`${(L.system.total_heap/1024).toFixed(1)} KB`:"--"},{label:"Chip Model",value:(null==(s=null==L?void 0:L.system)?void 0:s.chip_model)||"--"},{label:"CPU Frequency",value:(null==(o=null==L?void 0:L.system)?void 0:o.cpu_freq_mhz)?`${L.system.cpu_freq_mhz} MHz`:"--"},{label:"Free Flash",value:(null==(n=null==L?void 0:L.system)?void 0:n.free_flash_kb)?`${L.system.free_flash_kb} KB`:"--"}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${e}</span>
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
          ${[{label:"WiFi SSID",value:(null==(r=null==L?void 0:L.wifi)?void 0:r.ssid)||"--"},{label:"IP Address",value:(null==(v=null==L?void 0:L.wifi)?void 0:v.ip)||"--"},{label:"Signal Strength",value:(null==(u=null==L?void 0:L.wifi)?void 0:u.rssi)?`${L.wifi.rssi} dBm`:"--"},{label:"Signal Quality",value:(null==(g=null==L?void 0:L.wifi)?void 0:g.quality)?`${L.wifi.quality}%`:"--"},{label:"MAC Address",value:(null==(c=null==L?void 0:L.wifi)?void 0:c.mac)||"--"},{label:"Connected",value:(null==(b=null==L?void 0:L.wifi)?void 0:b.connected)?"✅ Yes":"❌ No"}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white font-mono text-sm">${e}</span>
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
          ${[{label:"Server",value:(null==(p=null==L?void 0:L.mqtt)?void 0:p.server)||"--"},{label:"Port",value:(null==(m=null==L?void 0:L.mqtt)?void 0:m.port)||"--"}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white font-mono text-sm">${e}</span>
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
          ${[{label:"Sensor Update",value:X(null==(y=null==L?void 0:L.timing)?void 0:y.sensor_update_interval)},{label:"History Update",value:X(null==(x=null==L?void 0:L.timing)?void 0:x.history_update_interval)},{label:"PID Update",value:X(null==(h=null==L?void 0:L.timing)?void 0:h.pid_update_interval)},{label:"Connectivity Check",value:X(null==(f=null==L?void 0:L.timing)?void 0:f.connectivity_check_interval)},{label:"PID Config Write",value:X(null==(w=null==L?void 0:L.timing)?void 0:w.pid_config_write_interval)},{label:"WiFi Connect Timeout",value:(null==($=null==L?void 0:L.timing)?void 0:$.wifi_connect_timeout)?`${L.timing.wifi_connect_timeout}s`:"--"},{label:"Max Reconnect Attempts",value:(null==(k=null==L?void 0:L.timing)?void 0:k.max_reconnect_attempts)||"--"},{label:"System Watchdog",value:X(null==(_=null==L?void 0:L.timing)?void 0:_.system_watchdog_timeout)},{label:"WiFi Watchdog",value:X(null==(F=null==L?void 0:L.timing)?void 0:F.wifi_watchdog_timeout)}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${e}</span>
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
          ${[{label:"Kp",value:(null==(S=null==(C=null==L?void 0:L.pid)?void 0:C.kp)?void 0:S.toFixed(3))||"--"},{label:"Ki",value:(null==(j=null==(I=null==L?void 0:L.pid)?void 0:I.ki)?void 0:j.toFixed(3))||"--"},{label:"Kd",value:(null==(M=null==(P=null==L?void 0:L.pid)?void 0:P.kd)?void 0:M.toFixed(3))||"--"},{label:"Setpoint",value:(null==(T=null==L?void 0:L.pid)?void 0:T.setpoint)?`${L.pid.setpoint.toFixed(1)}°C`:"--"},{label:"Deadband",value:(null==(A=null==L?void 0:L.pid)?void 0:A.deadband)?`${L.pid.deadband.toFixed(2)}°C`:"--"},{label:"Valve Position",value:void 0!==(null==(D=null==L?void 0:L.pid)?void 0:D.valve_position)?`${L.pid.valve_position}%`:"--"},{label:"Adaptation Interval",value:(null==(q=null==L?void 0:L.pid)?void 0:q.adaptation_interval)?`${L.pid.adaptation_interval}s`:"--"}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${e}</span>
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
          ${[{label:"Current Preset",value:(null==(W=null==L?void 0:L.presets)?void 0:W.current)||"--"},{label:"Eco",value:(null==(K=null==L?void 0:L.presets)?void 0:K.eco)?`${L.presets.eco.toFixed(1)}°C`:"--"},{label:"Comfort",value:(null==(R=null==L?void 0:L.presets)?void 0:R.comfort)?`${L.presets.comfort.toFixed(1)}°C`:"--"},{label:"Away",value:(null==(B=null==L?void 0:L.presets)?void 0:B.away)?`${L.presets.away.toFixed(1)}°C`:"--"},{label:"Sleep",value:(null==(U=null==L?void 0:L.presets)?void 0:U.sleep)?`${L.presets.sleep.toFixed(1)}°C`:"--"},{label:"Boost",value:(null==(H=null==L?void 0:L.presets)?void 0:H.boost)?`${L.presets.boost.toFixed(1)}°C`:"--"}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${e}</span>
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
          ${[{label:"Last Reboot Reason",value:(null==(z=null==L?void 0:L.diagnostics)?void 0:z.last_reboot_reason)||"--"},{label:"Reboot Count",value:(null==(N=null==L?void 0:L.diagnostics)?void 0:N.reboot_count)||"--"},{label:"Watchdog Reboots",value:(null==(Q=null==L?void 0:L.diagnostics)?void 0:Q.consecutive_watchdog_reboots)||"--"}].map(({label:l,value:e})=>i`
            <div class="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700/50 rounded-lg">
              <span class="text-gray-600 dark:text-gray-400">${l}</span>
              <span class="font-semibold text-gray-900 dark:text-white">${e}</span>
            </div>
          `)}
        </div>
      </div>
    </div>
  `}export{d as S};
