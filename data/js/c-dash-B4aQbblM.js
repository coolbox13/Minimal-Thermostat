import{d as e,q as t,y as r,_ as a}from"./v-preact-DZQM6r13.js";import{h as o}from"./v-misc-DwoQrUSZ.js";import{z as s}from"./v-ui-Dc9lvtut.js";function n(a=5e3){const[o,s]=e(null),[n,l]=e(!0),[d,i]=e(null),c=t(async e=>{try{const t=await fetch("/api/sensor",{signal:e});if(!t.ok)throw new Error(`HTTP ${t.status}: ${t.statusText}`);const r=await t.json();s({temperature:r.temperature??0,humidity:r.humidity??0,pressure:r.pressure??0,valve:r.valve??0,setpoint:r.setpoint??22}),i(null)}catch(t){if("AbortError"===t.name)return;i(t.message)}finally{l(!1)}},[]);return r(()=>{let e=!0,t=null,r=new AbortController;const o=async()=>{await c(r.signal),e&&(t=setTimeout(o,a))};return o(),()=>{e=!1,r.abort(),t&&clearTimeout(t)}},[a,c]),{data:o,loading:n,error:d,refetch:c}}const l=o.bind(a);function d(){const{data:e,loading:t,error:r}=n(5e3);if(t||!e)return l`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="animate-pulse">
          <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-40 mb-4"></div>
          <div class="space-y-3">
            ${[...Array(4)].map(()=>l`
              <div class="flex justify-between items-center">
                <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-24"></div>
                <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-20"></div>
              </div>
            `)}
          </div>
        </div>
      </div>
    `;if(r)return l`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Current Readings
        </h2>
        <div class="text-center py-4">
          <div class="text-red-500 text-3xl mb-2">⚠️</div>
          <p class="text-red-600 dark:text-red-400 text-sm">${r}</p>
        </div>
      </div>
    `;const a=[{label:"Temperature",value:null!=e?.temperature?`${e.temperature.toFixed(1)}°C`:"--",color:"text-blue-600 dark:text-blue-400"},{label:"Humidity",value:null!=e?.humidity?`${e.humidity.toFixed(1)}%`:"--",color:"text-green-600 dark:text-green-400"},{label:"Pressure",value:null!=e?.pressure?`${e.pressure.toFixed(0)} hPa`:"--",color:"text-purple-600 dark:text-purple-400"},{label:"Valve Position",value:null!=e?.valve?`${e.valve}%`:"--",color:"text-orange-600 dark:text-orange-400"}];return l`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Current Readings
      </h2>
      <div class="space-y-3">
        ${a.map(({label:e,value:t,color:r})=>l`
          <div class="flex justify-between items-center py-2 border-b border-gray-100 dark:border-gray-700 last:border-0">
            <span class="text-gray-600 dark:text-gray-400">${e}</span>
            <span class="${r} text-lg font-semibold">${t}</span>
          </div>
        `)}
      </div>
    </div>
  `}const i={duration:3e3,position:"top-center",style:{borderRadius:"12px",padding:"12px 16px",fontSize:"14px",maxWidth:"500px"}},c={success:function(e){return s.success(e,{...i,icon:"✅",style:{...i.style,background:"#10b981",color:"#ffffff"}})},error:function(e){return s.error(e,{...i,duration:4e3,icon:"❌",style:{...i.style,background:"#ef4444",color:"#ffffff"}})},info:function(e){return s(e,{...i,icon:"ℹ️",style:{...i.style,background:"#3b82f6",color:"#ffffff"}})},warning:function(e){return s(e,{...i,icon:"⚠️",style:{...i.style,background:"#f59e0b",color:"#ffffff"}})},loading:function(e){return s.loading(e,{...i,duration:1/0,style:{...i.style,background:"#6b7280",color:"#ffffff"}})},promise:function(e,t){return s.promise(e,{loading:t.loading,success:t.success,error:t.error},{...i,success:{icon:"✅",style:{...i.style,background:"#10b981",color:"#ffffff"}},error:{icon:"❌",style:{...i.style,background:"#ef4444",color:"#ffffff"}}})},dismiss:function(e){e?s.dismiss(e):s.dismiss()}},u=o.bind(a);function p(){const{presetMode:a,applyPreset:o,getPresetTemperature:s}=function(){const[a,o]=e("none"),[s,n]=e({eco:18,comfort:22,away:16,sleep:19,boost:24}),[l,d]=e(!0),[i,c]=e(null),u=t(async e=>{try{const t=await fetch("/api/config",{signal:e});if(!t.ok)throw new Error(`HTTP ${t.status}: ${t.statusText}`);const r=await t.json();r.presets&&(o(r.presets.current||"none"),n({eco:r.presets.eco??18,comfort:r.presets.comfort??22,away:r.presets.away??16,sleep:r.presets.sleep??19,boost:r.presets.boost??24})),c(null)}catch(t){if("AbortError"===t.name)return;c(t.message)}finally{d(!1)}},[]),p=t(async e=>{const t=a;o(e);try{const t=s[e];if(null==t)throw new Error(`Invalid preset mode: ${e}`);const r=await fetch("/api/setpoint",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:`value=${t}`});if(!r.ok)throw new Error(`HTTP ${r.status}: ${r.statusText}`);return c(null),!0}catch(r){return o(t),c(r.message),!1}},[a,s]),g=t(e=>"none"===e?null:s[e]??null,[s]);return r(()=>{const e=new AbortController;return u(e.signal),()=>{e.abort()}},[]),{presetMode:a,presetConfig:s,applyPreset:p,getPresetTemperature:g,loading:l,error:i,refetch:u}}(),{data:l}=n(5e3),[d,i]=e(22),[p,g]=e(!1),[b,f]=e(null);r(()=>{null!=l?.setpoint&&i(l.setpoint)},[l?.setpoint]);const y=t(async e=>{const t=e.target.value;try{await o(t);const e=s(t);e&&i(e),c.success(`Preset changed to ${"none"===t?"Manual":t}`)}catch(r){c.error("Failed to change preset mode")}},[o,s]),m=t(async()=>{const e=l?.setpoint||d;g(!0),f(null);try{if(!(await fetch("/api/setpoint",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:`value=${d}`})).ok)throw new Error("Failed to update setpoint");c.success(`Temperature set to ${d.toFixed(1)}°C`)}catch(t){i(e),f(t.message),c.error(`Failed to update setpoint: ${t.message}`)}finally{g(!1)}},[d,l]),v=s(a);return u`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Controls
      </h2>

      <!-- Preset Mode Selector -->
      <div class="mb-6">
        <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
          Preset Mode
        </label>
        <div class="flex items-center gap-3">
          <select
            value=${a}
            onChange=${y}
            class="flex-1 px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white focus:ring-2 focus:ring-primary-500 focus:border-transparent transition-all"
          >
            <option value="none">None (Manual)</option>
            <option value="eco">🌱 Eco</option>
            <option value="comfort">🏠 Comfort</option>
            <option value="away">✈️ Away</option>
            <option value="sleep">😴 Sleep</option>
            <option value="boost">🔥 Boost</option>
          </select>
          ${v&&u`
            <span class="text-sm font-medium text-primary-600 dark:text-primary-400 min-w-[60px] text-right">
              ${v.toFixed(1)}°C
            </span>
          `}
        </div>
      </div>

      <!-- Temperature Setpoint Slider -->
      <div class="mb-6">
        <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
          Temperature Setpoint
        </label>

        <!-- Mobile-Optimized Slider with Floating Value -->
        <div class="relative mb-4">
          <input
            type="range"
            min="15"
            max="30"
            step="0.5"
            value=${d}
            onInput=${e=>i(parseFloat(e.target.value))}
            class="w-full h-3 bg-gray-200 dark:bg-gray-700 rounded-lg appearance-none cursor-pointer slider-thumb"
            style="background: linear-gradient(to right, #1e88e5 0%, #1e88e5 ${(d-15)/15*100}%, rgb(229 231 235) ${(d-15)/15*100}%, rgb(229 231 235) 100%)"
          />

          <!-- Floating Value Display -->
          <div
            class="absolute -top-10 left-1/2 transform -translate-x-1/2 bg-primary-500 text-white px-3 py-1 rounded-lg text-lg font-bold shadow-lg"
            style="left: ${(d-15)/15*100}%"
          >
            ${d.toFixed(1)}°C
            <div class="absolute top-full left-1/2 transform -translate-x-1/2 w-0 h-0 border-l-4 border-r-4 border-t-4 border-l-transparent border-r-transparent border-t-primary-500"></div>
          </div>
        </div>

        <!-- Min/Max Labels -->
        <div class="flex justify-between text-xs text-gray-500 dark:text-gray-400 mb-4">
          <span>15°C</span>
          <span>30°C</span>
        </div>
      </div>

      <!-- Set Button -->
      <button
        onClick=${m}
        disabled=${p}
        class="w-full px-4 py-3 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white font-medium rounded-lg transition-all duration-200 shadow-md hover:shadow-lg disabled:cursor-not-allowed"
      >
        ${p?"Updating...":"Set Temperature"}
      </button>

      <!-- Error Display -->
      ${b&&u`
        <div class="mt-3 p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
          <p class="text-sm text-red-600 dark:text-red-400">${b}</p>
        </div>
      `}
    </div>
  `}const g=o.bind(a);function b(){const[a,o]=e(!1),[s,n]=e(0),[l,d]=e(!1),[i,u]=e(null),[p,b]=e(null);r(()=>{const e=async()=>{try{const e=await fetch("/api/manual-override"),t=await e.json();o(t.enabled||!1),n(t.position||0),b(t.remainingSeconds||null)}catch(e){}};e();const t=setInterval(e,5e3);return()=>clearInterval(t)},[]),r(()=>{if(!a||!p)return;const e=setInterval(()=>{b(e=>e<=1?(o(!1),null):e-1)},1e3);return()=>clearInterval(e)},[a,p]);const f=t(async()=>{const e=!a,t=a;o(e),d(!0),u(null);try{const t=new URLSearchParams;if(t.append("enabled",e),e&&t.append("position",s),!(await fetch("/api/manual-override",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:t.toString()})).ok)throw new Error("Failed to toggle override");e?(b(1800),c.success("Manual override enabled")):(b(null),n(0),c.info("Manual override disabled"))}catch(r){o(t),u(r.message),c.error(`Failed to toggle override: ${r.message}`)}finally{d(!1)}},[a,s]),y=t(async()=>{d(!0),u(null);try{const e=new URLSearchParams;if(e.append("enabled",!0),e.append("position",s),!(await fetch("/api/manual-override",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:e.toString()})).ok)throw new Error("Failed to update valve position");c.success(`Valve position set to ${s}%`)}catch(e){u(e.message),c.error(`Failed to update valve position: ${e.message}`)}finally{d(!1)}},[s]);return g`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Manual Override
      </h2>

      <!-- Valve Position Slider -->
      <div class="mb-4">
        <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
          Valve Position
        </label>

        <div class="relative mb-4">
          <input
            type="range"
            min="0"
            max="100"
            step="5"
            value=${s}
            onInput=${e=>n(parseInt(e.target.value))}
            disabled=${!a}
            class="w-full h-3 bg-gray-200 dark:bg-gray-700 rounded-lg appearance-none cursor-pointer disabled:opacity-50 disabled:cursor-not-allowed"
            style="background: linear-gradient(to right, #ff9800 0%, #ff9800 ${s}%, rgb(229 231 235) ${s}%, rgb(229 231 235) 100%)"
          />

          <!-- Floating Value Display -->
          ${a&&g`
            <div
              class="absolute -top-10 left-1/2 transform -translate-x-1/2 bg-orange-500 text-white px-3 py-1 rounded-lg text-lg font-bold shadow-lg"
              style="left: ${s}%"
            >
              ${s}%
              <div class="absolute top-full left-1/2 transform -translate-x-1/2 w-0 h-0 border-l-4 border-r-4 border-t-4 border-l-transparent border-r-transparent border-t-orange-500"></div>
            </div>
          `}
        </div>

        <!-- Min/Max Labels -->
        <div class="flex justify-between text-xs text-gray-500 dark:text-gray-400 mb-4">
          <span>0%</span>
          <span>100%</span>
        </div>
      </div>

      <!-- Toggle and Set Buttons -->
      <div class="space-y-2">
        <button
          onClick=${f}
          disabled=${l}
          class="${a?"bg-red-500 hover:bg-red-600":"bg-orange-500 hover:bg-orange-600"} w-full px-4 py-3 disabled:bg-gray-400 text-white font-medium rounded-lg transition-all duration-200 shadow-md hover:shadow-lg disabled:cursor-not-allowed"
        >
          ${a?"Disable Manual Control":"Enable Manual Control"}
        </button>

        ${a&&g`
          <button
            onClick=${y}
            disabled=${l}
            class="w-full px-4 py-2 bg-gray-600 hover:bg-gray-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-all duration-200 disabled:cursor-not-allowed"
          >
            ${l?"Updating...":"Update Position"}
          </button>
        `}
      </div>

      <!-- Timer Display -->
      ${a&&p&&g`
        <div class="mt-3 p-3 bg-orange-50 dark:bg-orange-900/20 border border-orange-200 dark:border-orange-800 rounded-lg">
          <p class="text-sm text-orange-700 dark:text-orange-300 flex items-center gap-2">
            <span>⏱️</span>
            <span>Override active for ${m=p,m?`${Math.floor(m/60)}:${(m%60).toString().padStart(2,"0")}`:""}</span>
          </p>
        </div>
      `}

      <!-- Error Display -->
      ${i&&g`
        <div class="mt-3 p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
          <p class="text-sm text-red-600 dark:text-red-400">${i}</p>
        </div>
      `}
    </div>
  `;var m}const f=o.bind(a);function y(){return f`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        System
      </h2>

      <div class="space-y-2">
        <!-- Refresh Button -->
        <button
          onClick=${()=>{window.location.reload()}}
          class="w-full px-4 py-2 bg-primary-500 hover:bg-primary-600 text-white font-medium rounded-lg transition-all duration-200 shadow-sm hover:shadow-md"
        >
          Refresh Data
        </button>

        <!-- Navigation Links -->
        ${[{label:"Event Logs",href:"/logs"},{label:"Serial Monitor",href:"/serial"},{label:"Firmware Update",href:"/update"},{label:"Configuration",href:"/config"},{label:"System Status",href:"/status"}].map(({label:e,href:t})=>f`
          <a
            href=${t}
            class="block w-full px-4 py-2 bg-gray-100 hover:bg-gray-200 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white font-medium rounded-lg transition-all duration-200 text-center shadow-sm hover:shadow-md"
          >
            ${e}
          </a>
        `)}
      </div>
    </div>
  `}export{p as C,b as M,d as S,y as a,c as t};
