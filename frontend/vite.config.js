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
        manualChunks(id) {
          // Vendor chunks
          if (id.includes('node_modules')) {
            if (id.includes('preact')) {
              return 'vendor-preact';
            }
            if (id.includes('uplot')) {
              return 'vendor-chart';
            }
            if (id.includes('headlessui') || id.includes('react-hot-toast')) {
              return 'vendor-ui';
            }
            // Other node_modules go to vendor-misc
            return 'vendor-misc';
          }

          // Page-based chunking for lazy loading
          if (id.includes('/pages/')) {
            const name = id.split('/pages/')[1].split('.')[0];
            return `page-${name.toLowerCase()}`;
          }

          // Component chunks
          if (id.includes('/components/Graph/')) {
            return 'components-graph';
          }
          if (id.includes('/components/Dashboard/')) {
            return 'components-dashboard';
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
        chunkFileNames: 'assets/js/[name]-[hash].js',
        entryFileNames: 'assets/js/[name]-[hash].js',
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
