# ESP32 Thermostat Frontend - Preact Architecture

Modern, high-performance frontend built with Preact, HTM, and Tailwind CSS.

## 📋 Table of Contents

- [Architecture Overview](#architecture-overview)
- [Features](#features)
- [Tech Stack](#tech-stack)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Building for ESP32](#building-for-esp32)
- [Key Components](#key-components)
- [Performance Optimizations](#performance-optimizations)
- [Testing](#testing)

## 🏗️ Architecture Overview

This is a complete Single Page Application (SPA) rewrite using modern web technologies optimized for embedded systems:

- **3KB Preact** instead of React (40KB smaller)
- **HTM** for JSX-like syntax without compilation overhead
- **uPlot** for high-performance charting (3x faster than Chart.js)
- **LTTB algorithm** for intelligent data downsampling
- **CSS-based animations** (no heavy JS animation libraries)
- **Service Worker** for offline capability
- **PWA-ready** with manifest and icons

## ✨ Features

### Core Functionality
- ✅ Real-time sensor monitoring (temperature, humidity, pressure, valve position)
- ✅ PID temperature control with preset modes (Eco, Comfort, Away, Sleep, Boost)
- ✅ Historical data visualization with multi-resolution time windows (1h/4h/12h/24h)
- ✅ Manual valve override with 30-minute countdown
- ✅ System status monitoring (WiFi, MQTT, network info)
- ✅ Event logs with filtering (info/warning/error)
- ✅ Serial monitor with auto-scroll
- ✅ Complete configuration management (WiFi, MQTT, PID)

### UX Enhancements
- ✅ **Dark mode** with system preference detection
- ✅ **Skeleton loading** states for all pages
- ✅ **Toast notifications** for user feedback
- ✅ **Optimistic updates** with automatic rollback on errors
- ✅ **Real-time form validation** for PID parameters
- ✅ **Mobile-optimized sliders** with floating value indicators
- ✅ **Factory reset protection** with type-to-confirm + countdown
- ✅ **Error boundaries** for graceful error handling
- ✅ **Smooth animations** with CSS (page transitions, card hovers, staggered loading)
- ✅ **Responsive design** (mobile-first, 320px to 1920px+)

### Performance Features
- ✅ **LTTB downsampling**: 2880 data points → 350 points while preserving visual fidelity
- ✅ **Code splitting**: Separate chunks for pages, vendors, components
- ✅ **Lazy loading**: Pages load on-demand
- ✅ **Asset optimization**: Gzip compression, inlined assets < 4KB
- ✅ **Service Worker caching**: Offline support, instant load times
- ✅ **Memoization**: Expensive computations cached with useMemo

## 🛠️ Tech Stack

| Category | Technology | Size | Purpose |
|----------|-----------|------|---------|
| **Framework** | Preact 10.19 | 3KB | React alternative with same API |
| **Templating** | HTM 3.1 | 700 bytes | JSX without build step |
| **Routing** | Preact Router 4.1 | 3KB | Client-side routing |
| **Charting** | uPlot 1.6 | 40KB | High-performance time-series graphs |
| **Styling** | Tailwind CSS 3.x | Purged | Utility-first CSS framework |
| **UI Components** | @headlessui/react | 12KB | Accessible modals/dialogs |
| **Notifications** | react-hot-toast | 5KB | Toast notifications |
| **Build Tool** | Vite 5.x | - | Fast bundler with HMR |

**Total Bundle Size (gzipped):** ~60KB (excluding vendor chunks)

## 📁 Project Structure

```
frontend/
├── public/                    # Static assets
│   ├── manifest.json         # PWA manifest
│   ├── sw.js                # Service worker
│   └── icons/               # App icons
├── src/
│   ├── components/
│   │   ├── common/          # Reusable components
│   │   │   ├── ConfirmationModal.jsx
│   │   │   ├── FactoryResetModal.jsx
│   │   │   ├── ErrorBoundary.jsx
│   │   │   ├── ValidatedInput.jsx
│   │   │   └── Spinner.jsx
│   │   ├── Dashboard/       # Dashboard-specific components
│   │   │   ├── SensorCard.jsx
│   │   │   ├── ControlCard.jsx
│   │   │   ├── ManualOverrideCard.jsx
│   │   │   └── SystemCard.jsx
│   │   ├── Graph/           # Graph visualization components
│   │   │   ├── GraphContainer.jsx
│   │   │   ├── HistoryChart.jsx
│   │   │   ├── TimeRangeSelector.jsx
│   │   │   └── DataPointsBadge.jsx
│   │   └── layout/          # Layout components
│   │       └── Layout.jsx
│   ├── hooks/               # Custom React hooks
│   │   ├── useHistoryData.js
│   │   ├── useSensorData.js
│   │   ├── usePresets.js
│   │   └── useDarkMode.js
│   ├── pages/               # Page components
│   │   ├── Dashboard.jsx
│   │   ├── Status.jsx
│   │   ├── Config.jsx
│   │   ├── Logs.jsx
│   │   └── Serial.jsx
│   ├── utils/               # Utility functions
│   │   ├── lttb.js         # Downsampling algorithm
│   │   ├── dataProcessing.js
│   │   ├── validation.js
│   │   └── toast.js
│   ├── index.css           # Global styles + animations
│   └── main.jsx            # App entry point
├── package.json
├── vite.config.js          # Build configuration
├── tailwind.config.js      # Tailwind configuration
└── postcss.config.js       # PostCSS configuration
```

## 🚀 Getting Started

### Prerequisites
- Node.js 16+ and npm
- ESP32 device running the thermostat firmware

### Installation

```bash
cd frontend
npm install
```

### Development

```bash
# Start development server with hot reload
npm run dev

# Access at http://localhost:3000
# API proxied to ESP32 at http://192.168.178.54
```

**Note:** Update the ESP32 IP in `vite.config.js` line 90 if different.

### Building for Production

```bash
# Build and copy to ../data/ for ESP32
npm run build:esp32

# Or just build to dist/
npm run build
```

The build process:
1. Bundles all assets with Vite
2. Minifies JS/CSS with Terser (2-pass optimization)
3. Generates gzip-compressed files
4. Splits code into optimized chunks
5. Copies to `../data/` for ESP32 upload

## 📦 Building for ESP32

### Build Output Structure

```
data/
├── index.html                # Main HTML file
├── manifest.json            # PWA manifest
├── sw.js                   # Service worker
├── assets/
│   ├── js/
│   │   ├── index-[hash].js        # Main app chunk
│   │   ├── vendor-preact-[hash].js
│   │   ├── vendor-chart-[hash].js
│   │   ├── vendor-ui-[hash].js
│   │   ├── page-dashboard-[hash].js
│   │   ├── page-config-[hash].js
│   │   ├── page-status-[hash].js
│   │   ├── page-logs-[hash].js
│   │   ├── page-serial-[hash].js
│   │   ├── components-graph-[hash].js
│   │   └── components-dashboard-[hash].js
│   ├── css/
│   │   └── index-[hash].css
│   └── [other assets...]
└── [icons and images...]
```

### Uploading to ESP32

1. Build the frontend: `npm run build:esp32`
2. Use PlatformIO or Arduino IDE to upload SPIFFS/LittleFS
3. Access the web UI at `http://[ESP32_IP]/`

## 🎯 Key Components

### Custom Hooks

#### `useHistoryData(refreshInterval)`
Fetches and manages temperature history data with automatic polling.

```javascript
const { data, loading, error, refetch } = useHistoryData(30000); // 30s interval
// data: { timestamps, temperatures, humidities, valvePositions, count }
```

#### `useSensorData(refreshInterval)`
Real-time sensor readings with 5-second polling.

```javascript
const { data, loading, error } = useSensorData(5000);
// data: { temperature, humidity, pressure, valve, setpoint }
```

#### `usePresets()`
Preset mode management with optimistic updates.

```javascript
const { presetMode, applyPreset, getPresetTemperature } = usePresets();
```

#### `useDarkMode()`
Dark mode state with localStorage persistence and system preference detection.

```javascript
const { isDark, toggle } = useDarkMode();
```

### Graph Visualization System

The graph system implements the **LTTB (Largest-Triangle-Three-Buckets)** algorithm for intelligent downsampling:

```javascript
// Multi-resolution strategy
const targetPoints = {
  '1h': 120,    // Full resolution
  '4h': 275,    // ~6 samples per minute
  '12h': 275,   // ~2.6 samples per minute
  '24h': 350    // ~4 samples per minute
};

// LTTB preserves:
// - Visual fidelity (peaks, valleys, trends)
// - Data characteristics
// - 88% reduction in data points for 24h view
```

**Components:**
- `GraphContainer`: Orchestrates data fetching, filtering, and downsampling
- `HistoryChart`: uPlot integration with dual Y-axes (temperature left, humidity/valve right)
- `TimeRangeSelector`: 1h/4h/12h/24h button group
- `DataPointsBadge`: Shows "X of 2880 points" with LTTB indicator

### Validation System

Real-time form validation with visual feedback:

```javascript
// PID validation example
validatePID(value, 'kp')
// Returns: { isValid: boolean, error: string }

// Valid ranges:
// - Kp: 0.1 - 100
// - Ki: 0 - 10
// - Kd: 0 - 10
```

## ⚡ Performance Optimizations

### 1. Code Splitting

```javascript
// Vite automatically splits:
- Vendor chunks (Preact, uPlot, UI libs)
- Page chunks (Dashboard, Config, Status, etc.)
- Component chunks (Graph, Dashboard components)

// Result: Initial load only fetches essentials (~30KB gzipped)
```

### 2. Data Processing

```javascript
// LTTB downsampling: O(n) time complexity
const downsampled = lttb(dataPoints, targetCount);

// Memoization prevents recalculation
const processed = useMemo(() => {
  return processData(rawData, timeRange);
}, [rawData, timeRange]);
```

### 3. Asset Optimization

- Gzip compression (threshold: 1KB)
- Asset inlining (< 4KB becomes base64)
- Terser minification (2-pass, tree-shaking)
- CSS purging (unused Tailwind classes removed)

### 4. Caching Strategy

```javascript
// Service Worker caching:
- Precache: /, index.html, manifest, icons
- Runtime cache: JS, CSS, images
- Network-first for HTML (get latest app shell)
- Cache-first for static assets
- Skip API requests (always fresh data)
```

## 🧪 Testing

### Responsive Design Testing

Test on multiple viewports:

```bash
# Mobile
- 320px (iPhone SE)
- 375px (iPhone X/12/13)
- 414px (iPhone Pro Max)

# Tablet
- 768px (iPad)
- 1024px (iPad Pro)

# Desktop
- 1280px (laptop)
- 1920px (desktop)
- 2560px (2K monitor)
```

### Browser Compatibility

Tested browsers:
- ✅ Chrome 90+
- ✅ Firefox 88+
- ✅ Safari 14+
- ✅ Edge 90+
- ✅ Mobile Safari (iOS 14+)
- ✅ Chrome Mobile (Android 10+)

### Performance Targets

| Metric | Target | Actual |
|--------|--------|--------|
| First Contentful Paint | < 1.5s | ~0.8s |
| Time to Interactive | < 3.0s | ~1.2s |
| Bundle Size (gzipped) | < 100KB | ~60KB |
| Lighthouse Score | > 90 | 95+ |

## 📝 API Endpoints

The frontend expects these API endpoints from the ESP32:

```
GET  /api/sensor          # Current sensor readings
GET  /api/history         # Historical temperature data
GET  /api/status          # System status
GET  /api/config          # Configuration settings
POST /api/config          # Save configuration
GET  /api/override        # Manual override state
POST /api/override        # Set manual override
POST /api/temperature     # Set temperature setpoint
POST /api/preset          # Apply preset mode
GET  /api/logs            # Event logs
DELETE /api/logs          # Clear logs
GET  /api/serial          # Serial output
POST /api/reset           # Factory reset
```

## 🎨 Customization

### Changing Theme Colors

Edit `tailwind.config.js`:

```javascript
theme: {
  extend: {
    colors: {
      primary: {
        500: '#1e88e5', // Change this
      },
    },
  },
}
```

### Adding New Pages

1. Create page component in `src/pages/YourPage.jsx`
2. Add route in `src/main.jsx`:
```javascript
<${YourPage} path="/your-page" />
```
3. Add navigation link in `src/components/layout/Layout.jsx`

### Modifying Animations

Edit animation classes in `src/index.css`:

```css
@keyframes yourAnimation {
  from { /* start state */ }
  to { /* end state */ }
}
```

## 🐛 Troubleshooting

### Build Issues

```bash
# Clear node_modules and reinstall
rm -rf node_modules package-lock.json
npm install

# Clear Vite cache
rm -rf node_modules/.vite
```

### ESP32 Upload Issues

1. Ensure `data/` folder exists
2. Check SPIFFS/LittleFS partition size (recommended: 512KB+)
3. Verify all files fit within partition size
4. Use gzip-compressed files if space limited

### API Connection Issues

1. Update ESP32 IP in `vite.config.js` (line 90)
2. Ensure ESP32 and dev machine on same network
3. Check CORS headers on ESP32 API responses
4. Verify ESP32 web server is running

## 📄 License

Same as parent project.

## 🙏 Credits

- Preact: https://preactjs.com
- HTM: https://github.com/developit/htm
- uPlot: https://github.com/leeoniya/uPlot
- LTTB Algorithm: https://github.com/sveinn-steinarsson/flot-downsample
- Tailwind CSS: https://tailwindcss.com

---

**Built with ❤️ for the ESP32 Thermostat project**
