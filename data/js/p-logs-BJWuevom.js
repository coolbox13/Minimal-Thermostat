import{_ as e,d as a,y as r}from"./v-preact-BAP1wjwx.js";import{h as t}from"./v-misc-CJ9EBB9u.js";const d=t.bind(e);function s(){const[e,t]=a([]),[s,l]=a(!0),[o,i]=a(null),[n,g]=a("all");r(()=>{c();const e=setInterval(c,5e3);return()=>clearInterval(e)},[]);const c=async()=>{try{const e=await fetch("/api/logs"),a=await e.json();t(a.logs||[]),l(!1)}catch(e){i(e.message),l(!1)}},b="all"===n?e:e.filter(e=>e.level===n);return s?d`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="animate-pulse space-y-4">
          <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-32"></div>
          ${[...Array(5)].map(()=>d`
            <div class="border border-gray-200 dark:border-gray-700 rounded-lg p-4">
              <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-full"></div>
              <div class="h-3 bg-gray-200 dark:bg-gray-700 rounded w-24 mt-2"></div>
            </div>
          `)}
        </div>
      </div>
    `:d`
    <div class="space-y-4">
      <!-- Header with Filters and Actions -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-4">
        <div class="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
          <!-- Filter Buttons -->
          <div class="flex gap-2 flex-wrap">
            ${[{value:"all",label:"All",icon:"📋"},{value:"info",label:"Info",icon:"ℹ️"},{value:"warning",label:"Warning",icon:"⚠️"},{value:"error",label:"Error",icon:"❌"}].map(({value:e,label:a,icon:r})=>d`
              <button
                key=${e}
                onClick=${()=>g(e)}
                class="${n===e?"bg-primary-500 text-white":"bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300"} px-3 py-2 rounded-lg font-medium transition-all duration-200 hover:shadow-md flex items-center gap-1"
              >
                <span>${r}</span>
                <span>${a}</span>
              </button>
            `)}
          </div>

          <!-- Actions -->
          <div class="flex gap-2">
            <button
              onClick=${c}
              class="px-4 py-2 bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 rounded-lg font-medium hover:bg-gray-200 dark:hover:bg-gray-600 transition-all"
            >
              🔄 Refresh
            </button>
            <button
              onClick=${async()=>{try{await fetch("/api/logs",{method:"DELETE"}),t([])}catch(e){i(e.message)}}}
              class="px-4 py-2 bg-red-500 text-white rounded-lg font-medium hover:bg-red-600 transition-all"
            >
              🗑️ Clear
            </button>
          </div>
        </div>
      </div>

      <!-- Logs List -->
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
          Event Logs (${b.length})
        </h2>

        ${o&&d`
          <div class="mb-4 p-4 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
            <p class="text-red-600 dark:text-red-400">${o}</p>
          </div>
        `}

        ${0===b.length?d`
          <div class="text-center py-12">
            <div class="text-gray-400 text-5xl mb-4">📋</div>
            <p class="text-gray-600 dark:text-gray-400">No logs available</p>
          </div>
        `:d`
          <div class="space-y-2 max-h-[400px] sm:max-h-[600px] overflow-y-auto">
            ${b.map((e,a)=>{const r=(e=>{switch(e){case"error":return{bg:"bg-red-50 dark:bg-red-900/20 border-red-200 dark:border-red-800",text:"text-red-700 dark:text-red-300",icon:"❌"};case"warning":return{bg:"bg-orange-50 dark:bg-orange-900/20 border-orange-200 dark:border-orange-800",text:"text-orange-700 dark:text-orange-300",icon:"⚠️"};default:return{bg:"bg-blue-50 dark:bg-blue-900/20 border-blue-200 dark:border-blue-800",text:"text-blue-700 dark:text-blue-300",icon:"ℹ️"}}})(e.level);return d`
                <div
                  key=${a}
                  class="${r.bg} border ${r.text} rounded-lg p-4 transition-all hover:shadow-md"
                >
                  <div class="flex items-start gap-3">
                    <span class="text-xl flex-shrink-0">${r.icon}</span>
                    <div class="flex-1 min-w-0">
                      <p class="font-mono text-sm break-words">${e.message}</p>
                      <p class="text-xs opacity-70 mt-1">
                        ${e.timestamp||(new Date).toLocaleString()}
                      </p>
                    </div>
                  </div>
                </div>
              `})}
          </div>
        `}
      </div>
    </div>
  `}export{s as L};
