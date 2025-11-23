import{_ as e,d as t,y as a}from"./v-preact-DZQM6r13.js";import{h as r}from"./v-misc-DwoQrUSZ.js";import{t as s}from"./c-dash-B4aQbblM.js";import{h as d}from"./v-ui-Dc9lvtut.js";const i=r.bind(e),l="RESET";function n({isOpen:e,onClose:r,onConfirm:n}){const[o,g]=t(""),[p,b]=t(10),[c,m]=t(!1);a(()=>{e&&(g(""),b(10),m(!1))},[e]),a(()=>{if(!e||0===p)return;const t=setInterval(()=>{b(e=>Math.max(0,e-1))},1e3);return()=>clearInterval(t)},[e,p]);const u=o===l&&0===p&&!c;return i`
    <${d}
      open=${e}
      onClose=${()=>!c&&r()}
      class="relative z-50"
    >
      <!-- Backdrop -->
      <div class="fixed inset-0 bg-black/50 backdrop-blur-sm" aria-hidden="true"></div>

      <!-- Full-screen container -->
      <div class="fixed inset-0 flex items-center justify-center p-4">
        <!-- Modal panel -->
        <${d.Panel} class="mx-auto max-w-lg w-full bg-white dark:bg-gray-800 rounded-xl shadow-2xl p-6 animate-fade-in">
          <!-- Warning Icon -->
          <div class="bg-red-100 dark:bg-red-900/20 w-16 h-16 rounded-full flex items-center justify-center mb-4 mx-auto">
            <span class="text-4xl">🚨</span>
          </div>

          <!-- Title -->
          <${d.Title} class="text-2xl font-bold text-center text-gray-900 dark:text-white mb-3">
            Factory Reset
          <//>

          <!-- Warning Message -->
          <div class="bg-red-50 dark:bg-red-900/10 border-2 border-red-200 dark:border-red-800 rounded-lg p-4 mb-4">
            <p class="text-red-800 dark:text-red-200 font-semibold mb-2">
              ⚠️ This action cannot be undone!
            </p>
            <p class="text-red-700 dark:text-red-300 text-sm">
              Factory reset will:
            </p>
            <ul class="text-red-700 dark:text-red-300 text-sm list-disc list-inside mt-2 space-y-1">
              <li>Delete all configuration settings</li>
              <li>Clear WiFi and MQTT credentials</li>
              <li>Reset PID parameters to defaults</li>
              <li>Clear all preset configurations</li>
              <li>Erase temperature history data</li>
            </ul>
          </div>

          <!-- Countdown Timer -->
          ${p>0&&i`
            <div class="bg-orange-50 dark:bg-orange-900/10 border border-orange-200 dark:border-orange-800 rounded-lg p-3 mb-4 text-center">
              <p class="text-orange-700 dark:text-orange-300 font-medium">
                Please wait ${p} second${1!==p?"s":""} before confirming...
              </p>
              <div class="mt-2 h-2 bg-orange-200 dark:bg-orange-800 rounded-full overflow-hidden">
                <div
                  class="h-full bg-orange-500 transition-all duration-1000 ease-linear"
                  style="width: ${(10-p)/10*100}%"
                ></div>
              </div>
            </div>
          `}

          <!-- Type-to-Confirm Input -->
          <div class="mb-6">
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Type <span class="font-mono font-bold text-red-600 dark:text-red-400">${l}</span> to confirm:
            </label>
            <input
              type="text"
              value=${o}
              onInput=${e=>g(e.target.value.toUpperCase())}
              disabled=${c||p>0}
              placeholder=${p>0?"Waiting for countdown...":`Type "${l}"`}
              class="w-full px-4 py-3 bg-gray-50 dark:bg-gray-700 border-2 ${o===l?"border-red-500 dark:border-red-400":"border-gray-300 dark:border-gray-600"} rounded-lg text-gray-900 dark:text-white font-mono font-bold text-center text-lg focus:outline-none focus:ring-2 focus:ring-red-500 disabled:opacity-50 disabled:cursor-not-allowed transition-all"
            />
            ${o&&o!==l&&i`
              <p class="mt-2 text-sm text-red-600 dark:text-red-400">
                Text does not match. Please type exactly: ${l}
              </p>
            `}
            ${o===l&&0===p&&i`
              <p class="mt-2 text-sm text-green-600 dark:text-green-400 flex items-center gap-1">
                <span>✓</span>
                <span>Confirmed. You may now proceed.</span>
              </p>
            `}
          </div>

          <!-- Actions -->
          <div class="flex gap-3">
            <button
              onClick=${r}
              disabled=${c}
              class="flex-1 px-4 py-3 bg-gray-200 hover:bg-gray-300 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white rounded-lg font-medium transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
            >
              Cancel
            </button>
            <button
              onClick=${async()=>{if(u){m(!0);try{await n(),s.success("Factory reset completed successfully"),r()}catch(e){s.error(`Factory reset failed: ${e.message}`)}finally{m(!1)}}}}
              disabled=${!u}
              class="flex-1 px-4 py-3 ${u?"bg-red-500 hover:bg-red-600":"bg-gray-400 dark:bg-gray-600"} text-white rounded-lg font-medium transition-colors disabled:cursor-not-allowed"
            >
              ${c?"Resetting...":"Factory Reset"}
            </button>
          </div>
        <//>
      </div>
    <//>
  `}const o=r.bind(e);function g({label:e,value:r,onChange:s,validator:d,type:i="text",step:l,placeholder:n,required:g=!1,helpText:p}){const[b,c]=t(!1),[m,u]=t({isValid:!0,error:null});a(()=>{if(d&&""!==r&&null!=r){const e=d(r);u(e)}else g||""!==r&&null!=r||u({isValid:!0,error:null})},[r,d,g]);const y=b&&!m.isValid,x=b&&m.isValid&&""!==r&&null!==r;return o`
    <div class="mb-4">
      <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
        ${e}
        ${g&&o`<span class="text-red-500 ml-1">*</span>`}
      </label>

      <div class="relative">
        <input
          type=${i}
          step=${l}
          value=${r}
          onInput=${e=>s(e.target.value)}
          onBlur=${()=>{c(!0)}}
          placeholder=${n}
          class="${y?"border-red-500 focus:ring-red-500":x?"border-green-500 focus:ring-green-500":"border-gray-300 dark:border-gray-600 focus:ring-primary-500"} w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 transition-all"
        />

        <!-- Validation Icon -->
        ${y&&o`
          <div class="absolute right-3 top-1/2 transform -translate-y-1/2 text-red-500">
            <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
              <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z" clip-rule="evenodd"></path>
            </svg>
          </div>
        `}
        ${x&&o`
          <div class="absolute right-3 top-1/2 transform -translate-y-1/2 text-green-500">
            <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
              <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clip-rule="evenodd"></path>
            </svg>
          </div>
        `}
      </div>

      <!-- Error Message -->
      ${y&&o`
        <p class="mt-1 text-sm text-red-600 dark:text-red-400 flex items-center gap-1">
          <span>⚠️</span>
          <span>${m.error}</span>
        </p>
      `}

      <!-- Help Text -->
      ${p&&!y&&o`
        <p class="mt-1 text-sm text-gray-500 dark:text-gray-400">
          ${p}
        </p>
      `}
    </div>
  `}const p=r.bind(e);function b({size:e="md",color:t="primary",label:a,fullscreen:r=!1}){const s=p`
    <div
      class="${{sm:"w-4 h-4 border-2",md:"w-8 h-8 border-2",lg:"w-12 h-12 border-3",xl:"w-16 h-16 border-4"}[e]} ${{primary:"border-primary-500 border-t-transparent",white:"border-white border-t-transparent",gray:"border-gray-400 dark:border-gray-600 border-t-transparent"}[t]} rounded-full animate-spin"
      role="status"
      aria-label=${a||"Loading..."}
    ></div>
  `;return r?p`
      <div class="fixed inset-0 bg-black/50 backdrop-blur-sm flex items-center justify-center z-50">
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-2xl p-8 flex flex-col items-center gap-4">
          ${s}
          ${a&&p`
            <p class="text-gray-700 dark:text-gray-300 font-medium">${a}</p>
          `}
        </div>
      </div>
    `:a?p`
      <div class="flex items-center gap-3">
        ${s}
        <span class="text-gray-700 dark:text-gray-300">${a}</span>
      </div>
    `:s}function c(e,t){const a=parseFloat(e);if(isNaN(a))return{isValid:!1,error:"Must be a valid number"};switch(t.toLowerCase()){case"kp":if(a<.1||a>100)return{isValid:!1,error:"Kp must be between 0.1 and 100"};break;case"ki":if(a<0||a>10)return{isValid:!1,error:"Ki must be between 0 and 10"};break;case"kd":if(a<0||a>10)return{isValid:!1,error:"Kd must be between 0 and 10"};break;default:return{isValid:!1,error:"Unknown PID parameter"}}return{isValid:!0,error:null}}function m(e){return e&&0!==e.trim().length?e.length>32?{isValid:!1,error:"SSID cannot exceed 32 characters"}:{isValid:!0,error:null}:{isValid:!1,error:"SSID cannot be empty"}}function u(e){return e&&0!==e.trim().length?/^[a-zA-Z0-9.-]+$/.test(e)?{isValid:!0,error:null}:{isValid:!1,error:"Invalid broker address format"}:{isValid:!1,error:"Broker address cannot be empty"}}function y(e){const t=parseInt(e);return isNaN(t)?{isValid:!1,error:"Must be a valid number"}:t<1||t>65535?{isValid:!1,error:"Port must be between 1 and 65535"}:{isValid:!0,error:null}}const x=r.bind(e);function v({isOpen:e,onClose:a,onComplete:r}){const[d,i]=t(0),[l,n]=t(!1),[o,p]=t({ssid:"",password:""}),[v,k]=t({broker:"",port:"1883",username:"",password:""}),[h,f]=t({kp:"2.0",ki:"0.5",kd:"1.0"}),w=[{id:"wifi",title:"WiFi Setup"},{id:"mqtt",title:"MQTT Setup"},{id:"sensors",title:"Sensor Setup"},{id:"review",title:"Review & Save"}];return e?x`
    <div class="fixed inset-0 z-50 overflow-y-auto">
      <!-- Backdrop -->
      <div class="fixed inset-0 bg-black bg-opacity-50 transition-opacity"></div>

      <!-- Modal -->
      <div class="flex min-h-full items-center justify-center p-4">
        <div class="relative bg-white dark:bg-gray-800 rounded-2xl shadow-2xl max-w-3xl w-full p-8 animate-slideUp">
          <!-- Header -->
          <div class="mb-8">
            <h2 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
              Setup Wizard
            </h2>
            <p class="text-gray-600 dark:text-gray-400">
              Let's configure your ESP32 Thermostat step by step
            </p>
          </div>

          <!-- Progress Indicator -->
          <div class="mb-8">
            <div class="flex justify-between mb-4">
              ${w.map((e,t)=>x`
                  <div
                    class="flex flex-col items-center flex-1"
                    key=${e.id}
                  >
                    <!-- Step Icon -->
                    <div
                      class="${t<=d?"bg-primary-500 text-white":"bg-gray-200 dark:bg-gray-700 text-gray-500 dark:text-gray-400"}
                      w-12 h-12 rounded-full flex items-center justify-center text-xl font-bold transition-all duration-300 mb-2"
                    >
                      ${t<d?"✓":t+1}
                    </div>
                    <!-- Step Title -->
                    <span
                      class="${t<=d?"text-gray-900 dark:text-white font-semibold":"text-gray-500 dark:text-gray-400"}
                      text-sm text-center"
                    >
                      ${e.title}
                    </span>
                    <!-- Connector Line -->
                    ${t<w.length-1&&x`
                      <div
                        class="${t<d?"bg-primary-500":"bg-gray-200 dark:bg-gray-700"}
                        h-1 w-full absolute top-6 left-1/2 -z-10 transition-all duration-300"
                        style="width: calc(100% / ${w.length} - 3rem)"
                      ></div>
                    `}
                  </div>
                `)}
            </div>
          </div>

          <!-- Step Content -->
          <div class="mb-8 min-h-[400px]">
            ${0===d&&x`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>WiFi Setup:</strong> Configure your WiFi network credentials
            to connect your ESP32 to the internet. Make sure you're using a
            2.4GHz network as the ESP32 doesn't support 5GHz.
          </p>
        </div>

        <${g}
          label="WiFi Network Name (SSID)"
          type="text"
          value=${o.ssid}
          onChange=${e=>p({...o,ssid:e})}
          validator=${m}
          placeholder="Enter your WiFi network name"
          required=${!0}
          helpText="Maximum 32 characters"
        />

        <div>
          <label
            class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2"
          >
            WiFi Password
          </label>
          <input
            type="password"
            value=${o.password}
            onInput=${e=>p({...o,password:e.target.value})}
            class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border ${o.password.length>=8?"border-green-500":o.password.length>0?"border-red-500":"border-gray-300 dark:border-gray-600"} rounded-lg text-gray-900 dark:text-white"
            placeholder="Enter WiFi password"
          />
          <p class="mt-1 text-xs text-gray-500 dark:text-gray-400">
            Minimum 8 characters
          </p>
          ${o.password.length>0&&o.password.length<8&&x`
            <p class="mt-1 text-xs text-red-600 dark:text-red-400">
              Password must be at least 8 characters
            </p>
          `}
        </div>
      </div>
    `}
            ${1===d&&x`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>MQTT Setup:</strong> Configure your MQTT broker to enable
            communication with Home Assistant and other smart home platforms.
            You can skip this step if you don't use MQTT.
          </p>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div class="md:col-span-2">
            <${g}
              label="MQTT Broker Address"
              type="text"
              value=${v.broker}
              onChange=${e=>k({...v,broker:e})}
              validator=${u}
              placeholder="mqtt.example.com or 192.168.1.100"
              required=${!0}
              helpText="Hostname or IP address of your MQTT broker"
            />
          </div>

          <${g}
            label="MQTT Port"
            type="number"
            value=${v.port}
            onChange=${e=>k({...v,port:e})}
            validator=${y}
            required=${!0}
            helpText="Usually 1883 for MQTT"
          />

          <div>
            <label
              class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2"
            >
              Username (Optional)
            </label>
            <input
              type="text"
              value=${v.username}
              onInput=${e=>k({...v,username:e.target.value})}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="MQTT username"
            />
          </div>

          <div class="md:col-span-2">
            <label
              class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2"
            >
              Password (Optional)
            </label>
            <input
              type="password"
              value=${v.password}
              onInput=${e=>k({...v,password:e.target.value})}
              class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              placeholder="MQTT password"
            />
          </div>
        </div>
      </div>
    `}
            ${2===d&&x`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
          <p class="text-sm text-blue-700 dark:text-blue-300">
            <strong>PID Tuning:</strong> Configure the PID controller parameters
            for optimal temperature regulation. The default values work well for
            most setups, but you can fine-tune them later based on your specific
            radiator and room characteristics.
          </p>
        </div>

        <div class="bg-yellow-50 dark:bg-yellow-900/20 border border-yellow-200 dark:border-yellow-800 rounded-lg p-4">
          <p class="text-sm text-yellow-700 dark:text-yellow-300">
            <strong>Note:</strong> Kp controls proportional response, Ki reduces
            steady-state error, and Kd dampens oscillations. Start with default
            values and adjust if needed.
          </p>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
          <${g}
            label="Kp (Proportional)"
            type="number"
            step="0.1"
            value=${h.kp}
            onChange=${e=>f({...h,kp:e})}
            validator=${e=>c(e,"kp")}
            required=${!0}
            helpText="Range: 0.1 - 100"
          />

          <${g}
            label="Ki (Integral)"
            type="number"
            step="0.01"
            value=${h.ki}
            onChange=${e=>f({...h,ki:e})}
            validator=${e=>c(e,"ki")}
            required=${!0}
            helpText="Range: 0 - 10"
          />

          <${g}
            label="Kd (Derivative)"
            type="number"
            step="0.01"
            value=${h.kd}
            onChange=${e=>f({...h,kd:e})}
            validator=${e=>c(e,"kd")}
            required=${!0}
            helpText="Range: 0 - 10"
          />
        </div>
      </div>
    `}
            ${3===d&&x`
      <div class="space-y-6 animate-fadeIn">
        <div class="bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg p-4">
          <p class="text-sm text-green-700 dark:text-green-300">
            <strong>Review Your Configuration:</strong> Please review your
            settings before saving. You can always change these later in the
            Configuration page.
          </p>
        </div>

        <!-- WiFi Configuration Review -->
        <div class="bg-white dark:bg-gray-700 rounded-lg p-6 border border-gray-200 dark:border-gray-600">
          <h3
            class="text-lg font-semibold text-gray-900 dark:text-white mb-4 flex items-center gap-2"
          >
            <span>📡</span>
            <span>WiFi Configuration</span>
          </h3>
          <div class="space-y-2">
            <div class="flex justify-between">
              <span class="text-gray-600 dark:text-gray-400">Network:</span>
              <span class="text-gray-900 dark:text-white font-medium"
                >${o.ssid}</span
              >
            </div>
            <div class="flex justify-between">
              <span class="text-gray-600 dark:text-gray-400">Password:</span>
              <span class="text-gray-900 dark:text-white font-medium"
                >${"•".repeat(o.password.length)}</span
              >
            </div>
          </div>
        </div>

        <!-- MQTT Configuration Review -->
        <div class="bg-white dark:bg-gray-700 rounded-lg p-6 border border-gray-200 dark:border-gray-600">
          <h3
            class="text-lg font-semibold text-gray-900 dark:text-white mb-4 flex items-center gap-2"
          >
            <span>📨</span>
            <span>MQTT Configuration</span>
          </h3>
          <div class="space-y-2">
            <div class="flex justify-between">
              <span class="text-gray-600 dark:text-gray-400">Broker:</span>
              <span class="text-gray-900 dark:text-white font-medium"
                >${v.broker}:${v.port}</span
              >
            </div>
            ${v.username&&x`
              <div class="flex justify-between">
                <span class="text-gray-600 dark:text-gray-400">Username:</span>
                <span class="text-gray-900 dark:text-white font-medium"
                  >${v.username}</span
                >
              </div>
            `}
          </div>
        </div>

        <!-- PID Configuration Review -->
        <div class="bg-white dark:bg-gray-700 rounded-lg p-6 border border-gray-200 dark:border-gray-600">
          <h3
            class="text-lg font-semibold text-gray-900 dark:text-white mb-4 flex items-center gap-2"
          >
            <span>🎛️</span>
            <span>PID Configuration</span>
          </h3>
          <div class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 gap-4">
            <div class="text-center">
              <div class="text-gray-600 dark:text-gray-400 text-sm mb-1">
                Kp
              </div>
              <div class="text-gray-900 dark:text-white font-bold text-xl">
                ${h.kp}
              </div>
            </div>
            <div class="text-center">
              <div class="text-gray-600 dark:text-gray-400 text-sm mb-1">
                Ki
              </div>
              <div class="text-gray-900 dark:text-white font-bold text-xl">
                ${h.ki}
              </div>
            </div>
            <div class="text-center">
              <div class="text-gray-600 dark:text-gray-400 text-sm mb-1">
                Kd
              </div>
              <div class="text-gray-900 dark:text-white font-bold text-xl">
                ${h.kd}
              </div>
            </div>
          </div>
        </div>
      </div>
    `}
          </div>

          <!-- Navigation Buttons -->
          <div class="flex justify-between items-center pt-6 border-t border-gray-200 dark:border-gray-700">
            <div>
              ${d>0&&x`
                <button
                  onClick=${()=>{d>0&&i(d-1)}}
                  disabled=${l}
                  class="px-6 py-2 text-gray-700 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg font-medium transition-all disabled:opacity-50"
                >
                  ← Back
                </button>
              `}
            </div>

            <div class="flex gap-3">
              <button
                onClick=${()=>{a?.()}}
                disabled=${l}
                class="px-6 py-2 text-gray-600 dark:text-gray-400 hover:text-gray-900 dark:hover:text-white font-medium transition-all disabled:opacity-50"
              >
                Skip Setup
              </button>

              ${d<w.length-1?x`
                    <button
                      onClick=${()=>{d<w.length-1&&i(d+1)}}
                      disabled=${!(()=>{switch(d){case 0:return(()=>{const e=m(o.ssid).isValid,t=o.password.length>=8;return e&&t})();case 1:return(()=>{const e=u(v.broker).isValid,t=y(v.port).isValid;return e&&t})();case 2:return(()=>{const e=c(h.kp,"kp").isValid,t=c(h.ki,"ki").isValid,a=c(h.kd,"kd").isValid;return e&&t&&a})();case 3:return!0;default:return!1}})()||l}
                      class="px-6 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 disabled:cursor-not-allowed text-white rounded-lg font-medium transition-all"
                    >
                      Next →
                    </button>
                  `:x`
                    <button
                      onClick=${async()=>{n(!0);try{await fetch("/api/config",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({section:"wifi",ssid:o.ssid,password:o.password})}),await fetch("/api/config",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({section:"mqtt",broker:v.broker,port:parseInt(v.port),username:v.username,password:v.password})}),await fetch("/api/config",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({section:"pid",kp:parseFloat(h.kp),ki:parseFloat(h.ki),kd:parseFloat(h.kd)})}),s.success("Configuration saved successfully!"),n(!1),r?.(),a?.()}catch(e){s.error(`Failed to save configuration: ${e.message}`),n(!1)}}}
                      disabled=${l}
                      class="px-6 py-2 bg-green-500 hover:bg-green-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all flex items-center gap-2"
                    >
                      ${l?x`<${b} size="sm" color="white" />
                            <span>Saving...</span>`:x`<span>Save Configuration</span>`}
                    </button>
                  `}
            </div>
          </div>
        </div>
      </div>
    </div>
  `:null}const k=r.bind(e);function h(){const[e,r]=t(null),[d,i]=t(!0),[l,o]=t(!1),[p,b]=t("network"),[m,u]=t(!1),[y,x]=t(!1),[h,f]=t({}),[w,_]=t({kp:"2.0",ki:"0.5",kd:"1.0",setpoint:"22.0",deadband:"0.5",adaptation_enabled:!0,adaptation_interval:"60"}),[$,T]=t({kp:!0,ki:!0,kd:!0,setpoint:!0,deadband:!0,adaptation_interval:!0});a(()=>{C()},[]),a(()=>{e&&(f({wifi_ssid:e.network?.wifi_ssid||"",wifi_pass:"",ntp_server:e.network?.ntp_server||"pool.ntp.org",ntp_timezone_offset:e.network?.ntp_timezone_offset||0,ntp_daylight_offset:e.network?.ntp_daylight_offset||0,mqtt_server:e.mqtt?.server||"",mqtt_port:e.mqtt?.port||1883,mqtt_username:e.mqtt?.username||"",mqtt_password:"",mqtt_json_aggregate_enabled:e.mqtt?.json_aggregate_enabled||!1,knx_area:e.knx?.area||0,knx_line:e.knx?.line||0,knx_member:e.knx?.member||0,knx_test:e.knx?.use_test||!1,knx_valve_cmd_area:e.knx?.valve_command?.area||0,knx_valve_cmd_line:e.knx?.valve_command?.line||0,knx_valve_cmd_member:e.knx?.valve_command?.member||0,knx_valve_fb_area:e.knx?.valve_feedback?.area||0,knx_valve_fb_line:e.knx?.valve_feedback?.line||0,knx_valve_fb_member:e.knx?.valve_feedback?.member||0,bme280_address:e.bme280?.address||"0x76",bme280_sda:e.bme280?.sda_pin||21,bme280_scl:e.bme280?.scl_pin||22,bme280_interval:e.bme280?.interval||30,preset_eco:e.presets?.eco||18,preset_comfort:e.presets?.comfort||22,preset_away:e.presets?.away||16,preset_sleep:e.presets?.sleep||19,preset_boost:e.presets?.boost||24,sensor_update_interval:e.timing?.sensor_update_interval||3e4,history_update_interval:e.timing?.history_update_interval||3e4,pid_update_interval:e.timing?.pid_update_interval||1e4,connectivity_check_interval:e.timing?.connectivity_check_interval||3e5,pid_config_write_interval:e.timing?.pid_config_write_interval||3e5,wifi_connect_timeout:e.timing?.wifi_connect_timeout||180,system_watchdog_timeout:e.timing?.system_watchdog_timeout||27e5,wifi_watchdog_timeout:e.timing?.wifi_watchdog_timeout||18e5,max_reconnect_attempts:e.timing?.max_reconnect_attempts||10,webhook_enabled:e.webhook?.enabled||!1,webhook_url:e.webhook?.url||"",webhook_temp_low:e.webhook?.temp_low_threshold||15,webhook_temp_high:e.webhook?.temp_high_threshold||30}),e.pid&&_({kp:e.pid.kp?.toString()||"2.0",ki:e.pid.ki?.toString()||"0.5",kd:e.pid.kd?.toString()||"1.0",setpoint:e.pid.setpoint?.toString()||"22.0",deadband:e.pid.deadband?.toString()||"0.5",adaptation_enabled:e.pid.adaptation_enabled??!0,adaptation_interval:e.pid.adaptation_interval?.toString()||"60"}))},[e]);const C=async()=>{try{const e=await fetch("/api/config"),t=await e.json();r(t),i(!1)}catch(e){s.error(`Failed to load config: ${e.message}`),i(!1)}},I=(e,t)=>{f(a=>({...a,[e]:t}))},S=async(e,t)=>{o(!0);try{const a=await fetch("/api/config",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({[e]:t})});if(!a.ok){const e=await a.json();throw new Error(e.message||"Failed to save configuration")}s.success("Configuration saved successfully"),await C()}catch(a){s.error(`Failed to save config: ${a.message}`)}finally{o(!1)}},P=(e,t)=>{_(a=>({...a,[e]:t}));const a=c(t,e);T(t=>({...t,[e]:a.isValid}))},F=Object.values($).every(e=>e);return d?k`
      <div class="space-y-6">
        ${[...Array(4)].map(()=>k`
          <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6 animate-pulse">
            <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-40 mb-4"></div>
            <div class="space-y-3">
              ${[...Array(3)].map(()=>k`
                <div class="h-10 bg-gray-200 dark:bg-gray-700 rounded"></div>
              `)}
            </div>
          </div>
        `)}
      </div>
    `:k`
    <div class="space-y-6">
      <!-- Getting Started Section -->
      <div class="bg-gradient-to-r from-primary-500 to-blue-600 rounded-xl shadow-lg p-6 text-white">
        <div class="flex flex-col md:flex-row md:items-center md:justify-between gap-4">
          <div>
            <h2 class="text-2xl font-bold mb-2 flex items-center gap-2">
              <span>🚀</span>
              <span>Getting Started</span>
            </h2>
            <p class="text-white/90">
              New to the ESP32 Thermostat? Use our setup wizard to configure your device step-by-step.
            </p>
          </div>
          <button
            onClick=${()=>x(!0)}
            class="px-6 py-3 bg-white text-primary-600 hover:bg-gray-100 rounded-lg font-semibold transition-all flex items-center gap-2 whitespace-nowrap self-start md:self-center"
          >
            <span>✨</span>
            <span>Launch Setup Wizard</span>
          </button>
        </div>
      </div>

      <!-- Tab Navigation -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-4">
        <div class="flex flex-wrap gap-2">
          ${[{id:"network",label:"Network"},{id:"mqtt",label:"MQTT"},{id:"knx",label:"KNX"},{id:"bme280",label:"BME280"},{id:"pid",label:"PID"},{id:"presets",label:"Presets"},{id:"timing",label:"Timing"},{id:"webhook",label:"Webhook"}].map(e=>k`
            <button
              onClick=${()=>b(e.id)}
              class="px-4 py-2 rounded-lg font-medium transition-all ${p===e.id?"bg-primary-500 text-white":"bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 hover:bg-gray-200 dark:hover:bg-gray-600"}"
            >
              ${e.label}
            </button>
          `)}
        </div>
      </div>

      <!-- Network Tab -->
      ${"network"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>📡</span>
            <span>Network Configuration</span>
          </h2>
          <div class="space-y-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                WiFi SSID
              </label>
              <input
                type="text"
                value=${h.wifi_ssid||""}
                onInput=${e=>I("wifi_ssid",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Enter WiFi SSID"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                WiFi Password
              </label>
              <input
                type="password"
                autocomplete="current-password"
                value=${h.wifi_pass||""}
                onInput=${e=>I("wifi_pass",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Leave empty to keep current password"
              />
            </div>
            <div class="border-t border-gray-200 dark:border-gray-700 pt-4 mt-4">
              <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-4">NTP Time Synchronization</h3>
              <div class="space-y-4">
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                    NTP Server
                  </label>
                  <input
                    type="text"
                    value=${h.ntp_server||"pool.ntp.org"}
                    onInput=${e=>I("ntp_server",e.target.value)}
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                    placeholder="pool.ntp.org"
                  />
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                    Timezone Offset (seconds)
                  </label>
                  <input
                    type="number"
                    value=${h.ntp_timezone_offset||0}
                    onInput=${e=>I("ntp_timezone_offset",parseInt(e.target.value))}
                    step="3600"
                    min="-43200"
                    max="43200"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                  <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    UTC offset in seconds (e.g., UTC+1 = 3600, UTC+2 = 7200, UTC-5 = -18000)
                  </p>
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                    Daylight Saving Offset (seconds)
                  </label>
                  <input
                    type="number"
                    value=${h.ntp_daylight_offset||0}
                    onInput=${e=>I("ntp_daylight_offset",parseInt(e.target.value))}
                    step="3600"
                    min="0"
                    max="7200"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                  <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    DST offset in seconds (typically 3600 for 1 hour, 0 = no DST)
                  </p>
                </div>
              </div>
            </div>
            <button
              onClick=${()=>S("network",{wifi_ssid:h.wifi_ssid,wifi_pass:h.wifi_pass||void 0,ntp_server:h.ntp_server,ntp_timezone_offset:h.ntp_timezone_offset,ntp_daylight_offset:h.ntp_daylight_offset})}
              disabled=${l}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
            >
              ${l?"Saving...":"Save Network Settings"}
            </button>
          </div>
        </div>
      `}

      <!-- MQTT Tab -->
      ${"mqtt"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>📨</span>
            <span>MQTT Configuration</span>
          </h2>
          <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Broker Address
              </label>
              <input
                type="text"
                value=${h.mqtt_server||""}
                onInput=${e=>I("mqtt_server",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="mqtt.example.com"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Port
              </label>
              <input
                type="number"
                value=${h.mqtt_port||1883}
                onInput=${e=>I("mqtt_port",parseInt(e.target.value))}
                min="1"
                max="65535"
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Username
              </label>
              <input
                type="text"
                value=${h.mqtt_username||""}
                onInput=${e=>I("mqtt_username",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Leave empty for no authentication"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Password
              </label>
              <input
                type="password"
                autocomplete="current-password"
                value=${h.mqtt_password||""}
                onInput=${e=>I("mqtt_password",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="Leave empty to keep current password"
              />
            </div>
          </div>
          <div class="mt-4 flex items-center gap-3">
            <input
              type="checkbox"
              id="mqtt_json_aggregate_enabled"
              checked=${h.mqtt_json_aggregate_enabled||!1}
              onChange=${e=>I("mqtt_json_aggregate_enabled",e.target.checked)}
              class="w-4 h-4 rounded"
            />
            <label for="mqtt_json_aggregate_enabled" class="text-sm font-medium text-gray-700 dark:text-gray-300">
              Enable JSON Aggregate
            </label>
            <span class="text-xs text-gray-500 dark:text-gray-400">
              Publish all data as JSON on 'telegraph' topic
            </span>
          </div>
          <button
            onClick=${()=>S("mqtt",{server:h.mqtt_server,port:h.mqtt_port,username:h.mqtt_username||void 0,password:h.mqtt_password||void 0,json_aggregate_enabled:h.mqtt_json_aggregate_enabled})}
            disabled=${l}
            class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${l?"Saving...":"Save MQTT Settings"}
          </button>
        </div>
      `}

      <!-- KNX Tab -->
      ${"knx"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🏠</span>
            <span>KNX Configuration</span>
          </h2>
          <div class="space-y-6">
            <div>
              <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-4">Physical Address</h3>
              <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Area</label>
                  <input
                    type="number"
                    value=${h.knx_area||0}
                    onInput=${e=>I("knx_area",parseInt(e.target.value))}
                    min="0"
                    max="15"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Line</label>
                  <input
                    type="number"
                    value=${h.knx_line||0}
                    onInput=${e=>I("knx_line",parseInt(e.target.value))}
                    min="0"
                    max="15"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                </div>
                <div>
                  <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Member</label>
                  <input
                    type="number"
                    value=${h.knx_member||0}
                    onInput=${e=>I("knx_member",parseInt(e.target.value))}
                    min="0"
                    max="255"
                    class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                  />
                </div>
              </div>
            </div>
            <div>
              <div class="flex items-center gap-3 mb-4">
                <input
                  type="checkbox"
                  id="knx_test"
                  checked=${h.knx_test||!1}
                  onChange=${e=>I("knx_test",e.target.checked)}
                  class="w-4 h-4 rounded"
                />
                <label for="knx_test" class="text-sm font-medium text-gray-700 dark:text-gray-300">
                  Use Test Addresses
                </label>
              </div>
              ${h.knx_test&&k`
                <div class="bg-yellow-50 dark:bg-yellow-900/20 border border-yellow-200 dark:border-yellow-800 rounded-lg p-4">
                  <p class="text-sm text-yellow-800 dark:text-yellow-300">
                    <strong>Active Test Addresses:</strong> Valve Command/Feedback: 10/2/2
                  </p>
                </div>
              `}
              ${!h.knx_test&&k`
                <div class="space-y-4">
                  <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4">
                    <h4 class="font-semibold text-blue-900 dark:text-blue-300 mb-3">Valve Command Address</h4>
                    <div class="grid grid-cols-3 gap-3">
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Area</label>
                        <input
                          type="number"
                          value=${h.knx_valve_cmd_area||0}
                          onInput=${e=>I("knx_valve_cmd_area",parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Line</label>
                        <input
                          type="number"
                          value=${h.knx_valve_cmd_line||0}
                          onInput=${e=>I("knx_valve_cmd_line",parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Member</label>
                        <input
                          type="number"
                          value=${h.knx_valve_cmd_member||0}
                          onInput=${e=>I("knx_valve_cmd_member",parseInt(e.target.value))}
                          min="0"
                          max="255"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                    </div>
                  </div>
                  <div class="bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg p-4">
                    <h4 class="font-semibold text-green-900 dark:text-green-300 mb-3">Valve Feedback Address</h4>
                    <div class="grid grid-cols-3 gap-3">
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Area</label>
                        <input
                          type="number"
                          value=${h.knx_valve_fb_area||0}
                          onInput=${e=>I("knx_valve_fb_area",parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Line</label>
                        <input
                          type="number"
                          value=${h.knx_valve_fb_line||0}
                          onInput=${e=>I("knx_valve_fb_line",parseInt(e.target.value))}
                          min="0"
                          max="15"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                      <div>
                        <label class="block text-xs text-gray-600 dark:text-gray-400 mb-1">Member</label>
                        <input
                          type="number"
                          value=${h.knx_valve_fb_member||0}
                          onInput=${e=>I("knx_valve_fb_member",parseInt(e.target.value))}
                          min="0"
                          max="255"
                          class="w-full px-3 py-2 bg-white dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded text-gray-900 dark:text-white text-sm"
                        />
                      </div>
                    </div>
                  </div>
                </div>
              `}
            </div>
            <button
              onClick=${()=>S("knx",{area:h.knx_area,line:h.knx_line,member:h.knx_member,use_test:h.knx_test,valve_command:{area:h.knx_valve_cmd_area,line:h.knx_valve_cmd_line,member:h.knx_valve_cmd_member},valve_feedback:{area:h.knx_valve_fb_area,line:h.knx_valve_fb_line,member:h.knx_valve_fb_member}})}
              disabled=${l}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
            >
              ${l?"Saving...":"Save KNX Settings"}
            </button>
          </div>
        </div>
      `}

      <!-- BME280 Tab -->
      ${"bme280"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🌡️</span>
            <span>BME280 Sensor Configuration</span>
          </h2>
          <div class="space-y-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                I²C Address
              </label>
              <select
                value=${h.bme280_address||"0x76"}
                onInput=${e=>I("bme280_address",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              >
                <option value="0x76">0x76 (default)</option>
                <option value="0x77">0x77</option>
              </select>
            </div>
            <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  SDA Pin
                </label>
                <input
                  type="number"
                  value=${h.bme280_sda||21}
                  onInput=${e=>I("bme280_sda",parseInt(e.target.value))}
                  min="0"
                  max="39"
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  SCL Pin
                </label>
                <input
                  type="number"
                  value=${h.bme280_scl||22}
                  onInput=${e=>I("bme280_scl",parseInt(e.target.value))}
                  min="0"
                  max="39"
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Update Interval (seconds)
              </label>
              <input
                type="number"
                value=${h.bme280_interval||30}
                onInput=${e=>I("bme280_interval",parseInt(e.target.value))}
                min="1"
                max="3600"
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
              <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                Seconds between sensor readings (1-3600)
              </p>
            </div>
            <button
              onClick=${()=>S("bme280",{address:h.bme280_address,sda_pin:h.bme280_sda,scl_pin:h.bme280_scl,interval:h.bme280_interval})}
              disabled=${l}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
            >
              ${l?"Saving...":"Save BME280 Settings"}
            </button>
          </div>
        </div>
      `}

      <!-- PID Tab -->
      ${"pid"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🎛️</span>
            <span>PID Configuration</span>
          </h2>
          <div class="bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg p-4 mb-4">
            <p class="text-sm text-blue-700 dark:text-blue-300">
              <strong>PID Tuning Guide:</strong> Kp controls proportional response, Ki reduces steady-state error, and Kd dampens oscillations. Adjust carefully for optimal temperature control.
            </p>
          </div>
          <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
            <${g}
              label="Kp (Proportional)"
              type="number"
              step="0.1"
              value=${w.kp}
              onChange=${e=>P("kp",e)}
              validator=${e=>c(e,"kp")}
              helpText="Range: 0.1 - 100"
              required=${!0}
            />
            <${g}
              label="Ki (Integral)"
              type="number"
              step="0.01"
              value=${w.ki}
              onChange=${e=>P("ki",e)}
              validator=${e=>c(e,"ki")}
              helpText="Range: 0 - 10"
              required=${!0}
            />
            <${g}
              label="Kd (Derivative)"
              type="number"
              step="0.01"
              value=${w.kd}
              onChange=${e=>P("kd",e)}
              validator=${e=>c(e,"kd")}
              helpText="Range: 0 - 10"
              required=${!0}
            />
          </div>
          <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Default Setpoint (°C)
              </label>
              <input
                type="number"
                step="0.5"
                min="5"
                max="30"
                value=${w.setpoint}
                onInput=${e=>P("setpoint",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Deadband (°C)
              </label>
              <input
                type="number"
                step="0.1"
                min="0"
                max="5"
                value=${w.deadband}
                onInput=${e=>P("deadband",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
              />
            </div>
          </div>

          <!-- Adaptation Settings -->
          <div class="mt-6 p-4 bg-gray-50 dark:bg-gray-900 rounded-lg border border-gray-200 dark:border-gray-700">
            <h4 class="text-sm font-semibold text-gray-900 dark:text-white mb-4">Auto-Tuning & Adaptation</h4>

            <div class="mb-4">
              <label class="flex items-center gap-3 cursor-pointer">
                <input
                  type="checkbox"
                  checked=${w.adaptation_enabled}
                  onChange=${e=>_({...w,adaptation_enabled:e.target.checked})}
                  class="w-5 h-5 text-primary-600 bg-gray-100 border-gray-300 rounded focus:ring-primary-500 dark:focus:ring-primary-600 dark:ring-offset-gray-800 focus:ring-2 dark:bg-gray-700 dark:border-gray-600"
                />
                <div>
                  <span class="text-sm font-medium text-gray-900 dark:text-white">
                    Enable Continuous Adaptation
                  </span>
                  <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    Automatically adjusts PID parameters based on system performance. Disable for manual calibration.
                  </p>
                </div>
              </label>
            </div>

            ${w.adaptation_enabled&&k`
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  Adaptation Interval (seconds)
                </label>
                <input
                  type="number"
                  step="1"
                  min="10"
                  max="600"
                  value=${w.adaptation_interval}
                  onInput=${e=>P("adaptation_interval",e.target.value)}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                  How often the system re-evaluates and adjusts PID parameters (10-600 seconds)
                </p>
              </div>
            `}
          </div>

          <div class="flex items-center gap-3">
            <button
              onClick=${async()=>{Object.values($).every(e=>e)?await S("pid",{kp:parseFloat(w.kp),ki:parseFloat(w.ki),kd:parseFloat(w.kd),setpoint:parseFloat(w.setpoint),deadband:parseFloat(w.deadband),adaptation_enabled:w.adaptation_enabled,adaptation_interval:parseFloat(w.adaptation_interval)}):s.error("Please fix validation errors before saving")}}
              disabled=${l||!F}
              class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 disabled:cursor-not-allowed text-white rounded-lg font-medium transition-all"
            >
              ${l?"Saving...":"Save PID Settings"}
            </button>
            ${!F&&k`
              <span class="text-sm text-red-600 dark:text-red-400 flex items-center gap-1">
                <span>⚠️</span>
                <span>Please fix validation errors</span>
              </span>
            `}
          </div>
        </div>
      `}

      <!-- Presets Tab -->
      ${"presets"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🌡️</span>
            <span>Temperature Presets</span>
          </h2>
          <p class="text-gray-600 dark:text-gray-400 mb-6">
            Configure the temperature for each preset mode. These presets can be quickly selected from the dashboard.
          </p>
          <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
            ${[{key:"preset_eco",label:"Eco Temperature",hint:"Energy-saving temperature (typically 16-18°C)"},{key:"preset_comfort",label:"Comfort Temperature",hint:"Comfortable temperature (typically 20-22°C)"},{key:"preset_away",label:"Away Temperature",hint:"Temperature when away (typically 16-17°C)"},{key:"preset_sleep",label:"Sleep Temperature",hint:"Nighttime temperature (typically 18-19°C)"},{key:"preset_boost",label:"Boost Temperature",hint:"Quick heating temperature (typically 23-24°C)"}].map(({key:e,label:t,hint:a})=>k`
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  ${t} (°C)
                </label>
                <input
                  type="number"
                  step="0.5"
                  min="5"
                  max="30"
                  value=${h[e]||""}
                  onInput=${t=>I(e,parseFloat(t.target.value))}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">${a}</p>
              </div>
            `)}
          </div>
          <button
            onClick=${()=>S("presets",{eco:h.preset_eco,comfort:h.preset_comfort,away:h.preset_away,sleep:h.preset_sleep,boost:h.preset_boost})}
            disabled=${l}
            class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${l?"Saving...":"Save Preset Settings"}
          </button>
        </div>
      `}

      <!-- Timing Tab -->
      ${"timing"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>⏱️</span>
            <span>Timing Intervals</span>
          </h2>
          <p class="text-gray-600 dark:text-gray-400 mb-6">
            Configure how often the system updates sensor data, history, PID calculations, and performs connectivity checks.
          </p>
          <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
            ${[{key:"sensor_update_interval",label:"Sensor Update Interval",unit:"ms",min:1e3,max:3e5,hint:"How often to read sensor (1s - 5min)"},{key:"history_update_interval",label:"History Update Interval",unit:"ms",min:1e3,max:3e5,hint:"How often to save history (1s - 5min)"},{key:"pid_update_interval",label:"PID Update Interval",unit:"ms",min:1e3,max:6e4,hint:"How often to calculate PID (1s - 1min)"},{key:"connectivity_check_interval",label:"Connectivity Check Interval",unit:"ms",min:6e4,max:36e5,hint:"How often to check connectivity (1min - 1hr)"},{key:"pid_config_write_interval",label:"PID Config Write Interval",unit:"ms",min:6e4,max:36e5,hint:"How often to save PID config (1min - 1hr)"},{key:"wifi_connect_timeout",label:"WiFi Connect Timeout",unit:"s",min:10,max:600,hint:"WiFi connection timeout (10s - 10min)"},{key:"system_watchdog_timeout",label:"System Watchdog Timeout",unit:"ms",min:6e4,max:72e5,hint:"System watchdog timeout (1min - 2hr)"},{key:"wifi_watchdog_timeout",label:"WiFi Watchdog Timeout",unit:"ms",min:6e4,max:72e5,hint:"WiFi watchdog timeout (1min - 2hr)"},{key:"max_reconnect_attempts",label:"Max Reconnect Attempts",unit:"",min:1,max:100,hint:"Maximum WiFi reconnection attempts"}].map(({key:e,label:t,unit:a,min:r,max:s,hint:d})=>k`
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  ${t}${a?` (${a})`:""}
                </label>
                <input
                  type="number"
                  value=${h[e]||""}
                  onInput=${t=>I(e,parseInt(t.target.value))}
                  min=${r}
                  max=${s}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">${d}</p>
              </div>
            `)}
          </div>
          <button
            onClick=${()=>S("timing",{sensor_update_interval:h.sensor_update_interval,history_update_interval:h.history_update_interval,pid_update_interval:h.pid_update_interval,connectivity_check_interval:h.connectivity_check_interval,pid_config_write_interval:h.pid_config_write_interval,wifi_connect_timeout:h.wifi_connect_timeout,system_watchdog_timeout:h.system_watchdog_timeout,wifi_watchdog_timeout:h.wifi_watchdog_timeout,max_reconnect_attempts:h.max_reconnect_attempts})}
            disabled=${l}
            class="mt-4 px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
          >
            ${l?"Saving...":"Save Timing Settings"}
          </button>
        </div>
      `}

      <!-- Webhook Tab -->
      ${"webhook"===p&&k`
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
          <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
            <span>🔔</span>
            <span>Webhook Configuration</span>
          </h2>
          <div class="space-y-4">
            <div class="flex items-center gap-3">
              <input
                type="checkbox"
                id="webhook_enabled"
                checked=${h.webhook_enabled||!1}
                onChange=${e=>I("webhook_enabled",e.target.checked)}
                class="w-4 h-4 rounded"
              />
              <label for="webhook_enabled" class="text-sm font-medium text-gray-700 dark:text-gray-300">
                Enable Webhook
              </label>
            </div>
            <div>
              <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Webhook URL
              </label>
              <input
                type="url"
                value=${h.webhook_url||""}
                onInput=${e=>I("webhook_url",e.target.value)}
                class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                placeholder="https://maker.ifttt.com/trigger/event/with/key/YOUR_KEY"
              />
            </div>
            <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  Low Temperature Threshold (°C)
                </label>
                <input
                  type="number"
                  step="0.5"
                  min="-20"
                  max="50"
                  value=${h.webhook_temp_low||15}
                  onInput=${e=>I("webhook_temp_low",parseFloat(e.target.value))}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
              <div>
                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  High Temperature Threshold (°C)
                </label>
                <input
                  type="number"
                  step="0.5"
                  min="-20"
                  max="50"
                  value=${h.webhook_temp_high||30}
                  onInput=${e=>I("webhook_temp_high",parseFloat(e.target.value))}
                  class="w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-900 dark:text-white"
                />
              </div>
            </div>
            <div class="flex gap-3">
              <button
                onClick=${()=>S("webhook",{enabled:h.webhook_enabled,url:h.webhook_url,temp_low_threshold:h.webhook_temp_low,temp_high_threshold:h.webhook_temp_high})}
                disabled=${l}
                class="px-4 py-2 bg-primary-500 hover:bg-primary-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
              >
                ${l?"Saving...":"Save Webhook Settings"}
              </button>
              <button
                onClick=${async()=>{try{if(!(await fetch("/api/webhook/test",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({url:h.webhook_url,temp_low:h.webhook_temp_low,temp_high:h.webhook_temp_high})})).ok)throw new Error("Webhook test failed");s.success("Webhook test sent successfully")}catch(e){s.error(`Webhook test failed: ${e.message}`)}}}
                disabled=${!h.webhook_url}
                class="px-4 py-2 bg-yellow-500 hover:bg-yellow-600 disabled:bg-gray-400 text-white rounded-lg font-medium transition-all"
              >
                Test Webhook
              </button>
            </div>
          </div>
        </div>
      `}

      <!-- Actions Bar -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4 flex items-center gap-2">
          <span>⚙️</span>
          <span>Device Actions</span>
        </h2>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <button
            onClick=${async()=>{try{const e=await fetch("/api/config/export"),t=await e.blob(),a=window.URL.createObjectURL(t),r=document.createElement("a");r.href=a,r.download=`thermostat-config-${(new Date).toISOString().split("T")[0]}.json`,document.body.appendChild(r),r.click(),document.body.removeChild(r),window.URL.revokeObjectURL(a),s.success("Configuration exported")}catch(e){s.error(`Export failed: ${e.message}`)}}}
            class="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>📥</span>
            <span>Export Configuration</span>
          </button>
          <button
            onClick=${()=>{const e=document.createElement("input");e.type="file",e.accept=".json",e.onchange=async e=>{const t=e.target.files[0];if(t)try{const e=await t.text();if(JSON.parse(e),!(await fetch("/api/config/import",{method:"POST",headers:{"Content-Type":"application/json"},body:e})).ok)throw new Error("Import failed");s.success("Configuration imported successfully"),await C()}catch(a){s.error(`Import failed: ${a.message}`)}},e.click()}}
            class="px-4 py-2 bg-yellow-500 hover:bg-yellow-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>📤</span>
            <span>Import Configuration</span>
          </button>
          <button
            onClick=${async()=>{try{if(!(await fetch("/api/reboot",{method:"POST"})).ok)throw new Error("Reboot failed");s.info("Device is rebooting... Please wait."),setTimeout(()=>{window.location.reload()},5e3)}catch(e){s.error(`Reboot failed: ${e.message}`)}}}
            class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>🔄</span>
            <span>Reboot Device</span>
          </button>
          <button
            onClick=${()=>u(!0)}
            class="px-4 py-2 bg-red-500 hover:bg-red-600 text-white rounded-lg font-medium transition-all flex items-center justify-center gap-2"
          >
            <span>⚠️</span>
            <span>Factory Reset</span>
          </button>
        </div>
      </div>

      <!-- Factory Reset Modal -->
      <${n}
        isOpen=${m}
        onClose=${()=>u(!1)}
        onConfirm=${async()=>{try{if(!(await fetch("/api/factory-reset",{method:"POST"})).ok)throw new Error("Factory reset failed");s.info("Device is resetting... Please wait."),setTimeout(()=>{window.location.reload()},5e3)}catch(e){s.error(`Factory reset failed: ${e.message}`)}}}
      />

      <!-- Configuration Wizard -->
      <${v}
        isOpen=${y}
        onClose=${()=>x(!1)}
        onComplete=${()=>{x(!1),C()}}
      />
    </div>
  `}export{h as C};
