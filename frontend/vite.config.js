import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';
import { compression } from 'vite-plugin-compression';

export default defineConfig({
  plugins: [
    preact(),
    compression({
      algorithm: 'gzip',
      ext: '.gz',
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
      },
    },
    rollupOptions: {
      output: {
        manualChunks: {
          'vendor': ['preact', 'preact-router', 'htm'],
          'chart': ['uplot'],
          'ui': ['framer-motion', 'react-hot-toast', '@headlessui/react'],
        },
      },
    },
    chunkSizeWarningLimit: 600,
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
});
