import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';
import viteCompression from 'vite-plugin-compression';

export default defineConfig({
  plugins: [
    preact(),
    // Gzip compression for smaller file sizes
    viteCompression({
      algorithm: 'gzip',
      ext: '.gz',
      threshold: 1024, // Only compress files > 1KB
      deleteOriginFile: false,
    }),
  ],
  build: {
    outDir: 'dist',
    assetsDir: 'assets',
    minify: 'terser',
    terserOptions: {
      compress: {
        drop_console: true,
        drop_debugger: true,
        pure_funcs: ['console.log', 'console.info', 'console.debug'],
        passes: 2, // Multiple passes for better minification
      },
      mangle: {
        safari10: true, // Safari 10 compatibility
      },
    },
    rollupOptions: {
      output: {
        // Optimize chunk splitting for better caching
        // Use SHORT names to avoid LittleFS path length limit (~31 chars including .gz)
        // Format: /js/[name]-[hash].js.gz must be < 31 chars
        // So [name]-[hash] must be < 23 chars (8 chars for /js/.js.gz)
        manualChunks(id) {
          // Vendor chunks - shortened names
          if (id.includes('node_modules')) {
            if (id.includes('preact')) {
              return 'v-preact';  // Was 'vendor-preact' (13) -> now 7 chars
            }
            if (id.includes('uplot')) {
              return 'v-chart';   // Was 'vendor-chart' (12) -> now 6 chars
            }
            if (id.includes('headlessui') || id.includes('react-hot-toast')) {
              return 'v-ui';      // Was 'vendor-ui' (9) -> now 3 chars
            }
            // Other node_modules go to vendor-misc
            return 'v-misc';      // Was 'vendor-misc' (11) -> now 5 chars
          }

          // Page-based chunking for lazy loading - shortened names
          if (id.includes('/pages/')) {
            const name = id.split('/pages/')[1].split('.')[0];
            // Use single letter prefix: p-config, p-dashboard, etc.
            return `p-${name.toLowerCase()}`;  // Was 'page-...' -> now 'p-...'
          }

          // Component chunks - shortened names
          if (id.includes('/components/Graph/')) {
            return 'c-graph';      // Was 'components-graph' (16) -> now 6 chars
          }
          if (id.includes('/components/Dashboard/')) {
            return 'c-dash';      // Was 'components-dashboard' (19) -> now 5 chars
          }
        },

        // Optimize asset file names for better caching
        assetFileNames: (assetInfo) => {
          const info = assetInfo.name.split('.');
          const ext = info[info.length - 1];
          if (/png|jpe?g|svg|gif|tiff|bmp|ico/i.test(ext)) {
            return `assets/images/[name]-[hash][extname]`;
          } else if (/woff|woff2|eot|ttf|otf/i.test(ext)) {
            return `assets/fonts/[name]-[hash][extname]`;
          }
          return `assets/[name]-[hash][extname]`;
        },
        // Use shorter paths to avoid LittleFS path length limit (~31 chars)
        // Changed from 'assets/js/' to 'js/' to reduce path length
        chunkFileNames: 'js/[name]-[hash].js',
        entryFileNames: 'js/[name]-[hash].js',
      },
    },
    chunkSizeWarningLimit: 600,
    cssCodeSplit: true, // Split CSS per component
    assetsInlineLimit: 4096, // Inline assets < 4KB as base64
    reportCompressedSize: true, // Show gzip sizes in build
    sourcemap: false, // Disable sourcemaps for production (smaller size)
  },
  server: {
    port: 3000,
    proxy: {
      '/api': {
        target: 'http://192.168.178.54', // ESP32 IP - update as needed
        changeOrigin: true,
      },
    },
  },
  // Performance optimizations
  optimizeDeps: {
    include: ['preact', 'preact/hooks', 'htm'], // Pre-bundle these
  },
});
