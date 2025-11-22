import{d as t,y as e,_ as s,A as a,T as r}from"./v-preact-BAP1wjwx.js";import{h as n}from"./v-misc-CJ9EBB9u.js";import{u as i}from"./v-chart-D5ou7yDp.js";function o(t,e){if(!t||0===t.length)return[];if(e>=t.length||0===e)return t;if(e<3)throw new Error("Threshold must be >= 3");const s=[],a=(t.length-2)/(e-2);let r=0;s.push(t[r]);for(let n=0;n<e-2;n++){let e=0,i=0,o=Math.floor((n+1)*a)+1,l=Math.floor((n+2)*a)+1;l=l<t.length?l:t.length;const u=l-o;for(;o<l;o++)e+=t[o].x,i+=t[o].y;e/=u,i/=u;let d=Math.floor((n+0)*a)+1;const m=Math.floor((n+1)*a)+1,c=t[r].x,h=t[r].y;let g=-1,p=null,v=null;for(;d<m;d++){const s=.5*Math.abs((c-e)*(t[d].y-h)-(c-t[d].x)*(i-h));s>g&&(g=s,p=t[d],v=d)}s.push(p),r=v}return s.push(t[t.length-1]),s}function l(t,e){return t&&e&&t.length===e.length?t.map((t,s)=>({x:t,y:e[s]})):[]}function u(t){return t&&0!==t.length?{timestamps:t.map(t=>t.x),values:t.map(t=>t.y)}:{timestamps:[],values:[]}}const d=n.bind(s);function m({selected:t,onChange:e}){return d`
    <div class="flex gap-2 justify-center mb-4">
      ${[{hours:1,label:"1h"},{hours:4,label:"4h"},{hours:12,label:"12h"},{hours:24,label:"24h"}].map(({hours:s,label:a})=>d`
        <button
          key=${s}
          onClick=${()=>e(s)}
          class=${`\n            px-4 py-2 rounded-lg font-medium transition-all duration-200\n            ${t===s?"bg-primary-500 text-white shadow-md scale-105":"bg-gray-100 text-gray-700 hover:bg-gray-200 dark:bg-gray-700 dark:text-gray-300 dark:hover:bg-gray-600"}\n          `}
        >
          ${a}
        </button>
      `)}
    </div>
  `}const c=n.bind(s);function h({showing:t,total:e,downsampled:s=!1}){const a=t=>t.toLocaleString();return c`
    <div class="flex items-center gap-2 text-sm text-gray-500 dark:text-gray-400">
      <span>
        Showing ${a(t)} of ${a(e)} points
      </span>
      ${s&&c`
        <span
          class="px-2 py-0.5 bg-blue-100 dark:bg-blue-900 text-blue-700 dark:text-blue-300 rounded text-xs font-medium"
          title="Data downsampled using LTTB algorithm for optimal performance while preserving visual fidelity"
        >
          LTTB
        </span>
      `}
    </div>
  `}const g=n.bind(s);function p({timestamps:t,temperatures:s,humidities:r,valvePositions:n,timeRange:o}){const l=a(null),u=a(null),d=()=>{const t=window.innerHeight;return t<600?250:t<900?300:400};return e(()=>{if(!l.current||!t||0===t.length)return;const e=(t,e=null)=>t&&Array.isArray(t)?t.map(t=>null==t||isNaN(t)?null:Number(t)):[],a=e(t),m=e(s),c=e(r),h=e(n),g=Math.max(a.length,m.length,c.length,h.length),p=(t,e)=>{const s=[...t];for(;s.length<e;)s.push(null);return s},v=[p(a,g),p(m,g),p(c,g),p(h,g)],f={width:l.current.offsetWidth,height:d(),plugins:[],scales:{x:{time:!0},y:{auto:!0,range:(t,e,s)=>{const a=.1*(s-e);return[e-a,s+a]}},"%":{auto:!1,range:[0,100]}},axes:[{space:60,values:(t,e)=>e.map(t=>{const e=new Date(1e3*t);return o<=4?e.toLocaleTimeString("en-US",{hour:"2-digit",minute:"2-digit"}):e.toLocaleString("en-US",{month:"short",day:"numeric",hour:"2-digit"})})},{scale:"y",values:(t,e)=>e.map(t=>t.toFixed(1)+"°C"),stroke:"#1e88e5",font:"12px Arial",gap:5},{scale:"%",values:(t,e)=>e.map(t=>t.toFixed(0)+"%"),stroke:"#ff9800",font:"12px Arial",gap:5,side:1}],series:[{},{label:"Temperature",stroke:"#1e88e5",width:2,scale:"y",value:(t,e)=>null==e||isNaN(e)?"--":e.toFixed(1)+"°C"},{label:"Humidity",stroke:"#4caf50",width:2,scale:"%",value:(t,e)=>null==e||isNaN(e)?"--":e.toFixed(1)+"%"},{label:"Valve",stroke:"#ff9800",width:2,scale:"%",value:(t,e)=>null==e||isNaN(e)?"--":e.toFixed(0)+"%",dash:[5,5]}],legend:{show:!0,live:!0},cursor:{drag:{x:!1,y:!1}}};u.current?(u.current.setData(v),u.current.setSize({width:l.current.offsetWidth,height:d()})):u.current=new i(f,v,l.current);const x=()=>{u.current&&l.current&&u.current.setSize({width:l.current.offsetWidth,height:d()})};return window.addEventListener("resize",x),()=>{window.removeEventListener("resize",x),u.current&&(u.current.destroy(),u.current=null)}},[t,s,r,n,o]),g`
    <div class="w-full" ref=${l}></div>
  `}const v=n.bind(s);function f(){const[s,a]=t(24),{data:n,loading:i,error:d}=function(s=3e4){const[a,r]=t(null),[n,i]=t(!0),[o,l]=t(null),u=async t=>{var e;try{const s=await fetch("/api/history?maxPoints=0",{signal:t});if(!s.ok)throw new Error(`HTTP ${s.status}: ${s.statusText}`);const a=await s.json();if(!a.timestamps||!a.temperatures)throw new Error("Invalid data format received from API");r({timestamps:a.timestamps||[],temperatures:a.temperatures||[],humidities:a.humidities||[],pressures:a.pressures||[],valvePositions:a.valvePositions||[],count:(null==(e=a.timestamps)?void 0:e.length)||0}),l(null)}catch(s){if("AbortError"===s.name)return;l(s.message)}finally{i(!1)}};return e(()=>{let t=!0,e=null,a=new AbortController;const r=async()=>{await u(a.signal),t&&(e=setTimeout(r,s))};return r(),()=>{t=!1,a.abort(),e&&clearTimeout(e)}},[s]),{data:a,loading:n,error:o,refetch:u}}(3e4),c=r(()=>{if(!n||!n.timestamps||0===n.timestamps.length)return null;const t=function(t,e){if(!t||!t.timestamps||0===t.timestamps.length)return null;const s=Math.floor(Date.now()/1e3)-3600*e,a=[];for(let r=0;r<t.timestamps.length;r++)t.timestamps[r]>=s&&a.push(r);return 0===a.length?null:{timestamps:a.map(e=>t.timestamps[e]),temperatures:a.map(e=>t.temperatures[e]),humidities:a.map(e=>t.humidities[e]),pressures:a.map(e=>t.pressures?t.pressures[e]:null),valvePositions:a.map(e=>t.valvePositions[e]),count:a.length}}(n,s);if(!t||0===t.count)return null;const e=function(t){switch(t){case 1:return 120;case 4:case 12:return 275;case 24:return 350;default:return 300}}(s);if(!(t.count>e))return{timestamps:t.timestamps,temperatures:t.temperatures,humidities:t.humidities,valvePositions:t.valvePositions,downsampledCount:t.count,originalCount:n.count,downsampled:!1};const a=l(t.timestamps,t.temperatures),r=l(t.timestamps,t.humidities),i=l(t.timestamps,t.valvePositions),d=o(a,e),m=o(r,e),c=o(i,e),{timestamps:h,values:g}=u(d),{values:p}=u(m),{values:v}=u(c);return{timestamps:h,temperatures:g,humidities:p,valvePositions:v,downsampledCount:h.length,originalCount:n.count,downsampled:!0}},[n,s]);return i?v`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="animate-pulse space-y-4">
          <div class="h-10 bg-gray-200 dark:bg-gray-700 rounded w-64 mx-auto"></div>
          <div class="h-64 bg-gray-200 dark:bg-gray-700 rounded"></div>
          <div class="h-6 bg-gray-200 dark:bg-gray-700 rounded w-48"></div>
        </div>
      </div>
    `:d?v`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-red-500 text-5xl mb-4">⚠️</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            Failed to Load History Data
          </h3>
          <p class="text-gray-600 dark:text-gray-400 mb-4">${d}</p>
          <button
            onClick=${()=>window.location.reload()}
            class="px-4 py-2 bg-primary-500 text-white rounded-lg hover:bg-primary-600"
          >
            Retry
          </button>
        </div>
      </div>
    `:c?v`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        Historical Data
      </h2>

      <!-- Time Range Selector -->
      <${m}
        selected=${s}
        onChange=${a}
      />

      <!-- Chart -->
      <div class="mb-4">
        <${p}
          timestamps=${c.timestamps}
          temperatures=${c.temperatures}
          humidities=${c.humidities}
          valvePositions=${c.valvePositions}
          timeRange=${s}
        />
      </div>

      <!-- Data Points Badge -->
      <div class="flex justify-end">
        <${h}
          showing=${c.downsampledCount}
          total=${c.originalCount}
          downsampled=${c.downsampled}
        />
      </div>
    </div>
  `:v`
      <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
        <div class="text-center py-8">
          <div class="text-gray-400 text-5xl mb-4">📊</div>
          <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-2">
            No Data Available
          </h3>
          <p class="text-gray-600 dark:text-gray-400">
            ${s}h time range has no data points yet.
          </p>
        </div>
      </div>
    `}export{f as G};
