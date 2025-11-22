var e=Object.defineProperty;import{_ as r,f as t,d as o,y as a,g as s,G as l,i}from"./v-preact-BAP1wjwx.js";import{h as n}from"./v-misc-CJ9EBB9u.js";import{F as d}from"./v-ui-BuZZnogH.js";import{D as c}from"./p-dashboard-C2zxQ0uV.js";import{S as g}from"./p-status-CSMg8qM0.js";import{C as m}from"./p-config-DkR3-lzJ.js";import{L as p}from"./p-logs-BJWuevom.js";import{S as h}from"./p-serial-_8KRU5lh.js";import"./c-graph-Dum04kY0.js";import"./v-chart-BlXMEoXN.js";import"./c-dash-yfXJLQcn.js";!function(){const e=document.createElement("link").relList;if(!(e&&e.supports&&e.supports("modulepreload"))){for(const e of document.querySelectorAll('link[rel="modulepreload"]'))r(e);new MutationObserver(e=>{for(const t of e)if("childList"===t.type)for(const e of t.addedNodes)"LINK"===e.tagName&&"modulepreload"===e.rel&&r(e)}).observe(document,{childList:!0,subtree:!0})}function r(e){if(e.ep)return;e.ep=!0;const r=function(e){const r={};return e.integrity&&(r.integrity=e.integrity),e.referrerPolicy&&(r.referrerPolicy=e.referrerPolicy),"use-credentials"===e.crossOrigin?r.credentials="include":"anonymous"===e.crossOrigin?r.credentials="omit":r.credentials="same-origin",r}(e);fetch(e.href,r)}}();const u=n.bind(r);class x extends t{constructor(r){var t;super(r),((r,t,o)=>{t in r?e(r,t,{enumerable:!0,configurable:!0,writable:!0,value:o}):r[t]=o})(this,"symbol"!=typeof(t="resetError")?t+"":t,()=>{this.setState({hasError:!1,error:null,errorInfo:null})}),this.state={hasError:!1,error:null,errorInfo:null}}componentDidCatch(e,r){this.setState({hasError:!0,error:e,errorInfo:r})}render({children:e,fallback:r},{hasError:t,error:o,errorInfo:a}){return t?r?r({error:o,errorInfo:a,reset:this.resetError}):u`
        <div class="min-h-screen bg-gray-50 dark:bg-gray-900 flex items-center justify-center p-4">
          <div class="max-w-2xl w-full bg-white dark:bg-gray-800 rounded-xl shadow-xl p-8">
            <!-- Error Icon -->
            <div class="text-center mb-6">
              <div class="inline-flex items-center justify-center w-20 h-20 bg-red-100 dark:bg-red-900/20 rounded-full mb-4">
                <span class="text-5xl">💥</span>
              </div>
              <h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
                Something Went Wrong
              </h1>
              <p class="text-gray-600 dark:text-gray-400">
                The application encountered an unexpected error.
              </p>
            </div>

            <!-- Error Details -->
            <div class="bg-red-50 dark:bg-red-900/10 border border-red-200 dark:border-red-800 rounded-lg p-4 mb-6">
              <h3 class="font-semibold text-red-800 dark:text-red-300 mb-2">
                Error Details:
              </h3>
              <p class="text-sm text-red-700 dark:text-red-400 font-mono break-all">
                ${(null==o?void 0:o.message)||"Unknown error"}
              </p>

              ${(null==o?void 0:o.stack)&&u`
                <details class="mt-3">
                  <summary class="cursor-pointer text-sm text-red-600 dark:text-red-400 hover:text-red-700 dark:hover:text-red-300">
                    Show stack trace
                  </summary>
                  <pre class="mt-2 text-xs text-red-700 dark:text-red-400 overflow-x-auto bg-red-100 dark:bg-red-900/20 p-3 rounded">
${o.stack}
                  </pre>
                </details>
              `}
            </div>

            <!-- Actions -->
            <div class="flex flex-col sm:flex-row gap-3">
              <button
                onClick=${this.resetError}
                class="flex-1 px-6 py-3 bg-primary-500 hover:bg-primary-600 text-white font-medium rounded-lg transition-all shadow-md hover:shadow-lg"
              >
                🔄 Try Again
              </button>
              <button
                onClick=${()=>window.location.reload()}
                class="flex-1 px-6 py-3 bg-gray-500 hover:bg-gray-600 text-white font-medium rounded-lg transition-all"
              >
                ↻ Reload Page
              </button>
              <a
                href="/"
                class="flex-1 px-6 py-3 bg-gray-200 hover:bg-gray-300 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white font-medium rounded-lg text-center transition-all"
              >
                🏠 Go Home
              </a>
            </div>

            <!-- Help Text -->
            <div class="mt-6 p-4 bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg">
              <p class="text-sm text-blue-700 dark:text-blue-300">
                <strong>💡 Troubleshooting Tips:</strong>
              </p>
              <ul class="mt-2 text-sm text-blue-600 dark:text-blue-400 list-disc list-inside space-y-1">
                <li>Check your internet connection</li>
                <li>Clear browser cache and reload</li>
                <li>Ensure ESP32 device is powered on</li>
                <li>Check browser console for detailed errors</li>
              </ul>
            </div>
          </div>
        </div>
      `:e}}const b=n.bind(r);function f({children:e}){const{isDark:r,toggle:t}=function(){const[e,r]=o(()=>{const e=localStorage.getItem("darkMode");return null!==e?"true"===e:!!window.matchMedia&&window.matchMedia("(prefers-color-scheme: dark)").matches});return a(()=>{e?document.documentElement.classList.add("dark"):document.documentElement.classList.remove("dark"),localStorage.setItem("darkMode",e.toString())},[e]),{isDark:e,toggle:()=>r(!e)}}(),l=[{path:"/",label:"Dashboard"},{path:"/status",label:"Status"},{path:"/config",label:"Config"},{path:"/logs",label:"Logs"},{path:"/serial",label:"Serial"}];return b`
    <div class="min-h-screen bg-gray-50 dark:bg-gray-900">
      <!-- Header with Navigation -->
      <header class="bg-white dark:bg-gray-800 shadow-sm border-b border-gray-200 dark:border-gray-700">
        <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div class="flex justify-between items-center py-4">
            <!-- Logo/Title -->
            <div class="flex items-center">
              <h1 class="text-xl sm:text-2xl font-bold text-gray-900 dark:text-white">
                ESP32 Thermostat
              </h1>
            </div>

            <!-- Desktop Navigation + Dark Mode -->
            <div class="hidden md:flex items-center gap-2">
              <nav class="flex gap-2">
                ${l.map(({path:e,label:r})=>b`
                  <${s}
                    key=${e}
                    href=${e}
                    activeClassName="bg-primary-500 text-white"
                    class="px-4 py-2 rounded-lg font-medium transition-all duration-200 hover:bg-primary-100 dark:hover:bg-gray-700 text-gray-700 dark:text-gray-300"
                  >
                    ${r}
                  <//>
                `)}
              </nav>

              <!-- Dark Mode Toggle -->
              <button
                onClick=${t}
                class="p-2 text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-all ml-2"
                title=${r?"Switch to light mode":"Switch to dark mode"}
              >
                ${r?b`
                  <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path>
                  </svg>
                `:b`
                  <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"></path>
                  </svg>
                `}
              </button>
            </div>

            <!-- Mobile Dark Mode Toggle -->
            <div class="md:hidden">
              <button
                onClick=${t}
                class="p-2 text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-all"
                title=${r?"Switch to light mode":"Switch to dark mode"}
              >
                ${r?b`
                  <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path>
                  </svg>
                `:b`
                  <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"></path>
                  </svg>
                `}
              </button>
            </div>
          </div>

          <!-- Mobile Navigation (Visible on small screens) -->
          <nav class="md:hidden flex overflow-x-auto pb-3 gap-2 -mx-4 px-4">
            ${l.map(({path:e,label:r})=>b`
              <${s}
                key=${e}
                href=${e}
                activeClassName="bg-primary-500 text-white"
                class="flex-shrink-0 px-3 py-2 rounded-lg font-medium transition-all duration-200 text-gray-700 dark:text-gray-300 text-sm whitespace-nowrap"
              >
                ${r}
              <//>
            `)}
          </nav>
        </div>
      </header>

      <!-- Main Content -->
      <main class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        ${e}
      </main>

      <!-- Footer -->
      <footer class="bg-white dark:bg-gray-800 border-t border-gray-200 dark:border-gray-700 mt-12">
        <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
          <p class="text-center text-sm text-gray-500 dark:text-gray-400">
            ESP32 KNX Thermostat © ${(new Date).getFullYear()}
          </p>
        </div>
      </footer>
    </div>
  `}const v=n.bind(r);l(v`<${function(){return v`
    <${x}>
      <div>
        <${f}>
          <${i}>
            <${c} path="/" />
            <${g} path="/status" />
            <${m} path="/config" />
            <${p} path="/logs" />
            <${h} path="/serial" />
          <//>
        <//>
        <${d}
          position="top-center"
          reverseOrder=${!1}
          gutter=${8}
          toastOptions=${{duration:3e3,style:{borderRadius:"12px",padding:"12px 16px",fontSize:"14px",fontWeight:"500",maxWidth:"500px"}}}
        />
      </div>
    <//>
  `}} />`,document.getElementById("app"));
