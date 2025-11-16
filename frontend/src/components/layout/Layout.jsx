import { h } from 'preact';
import htm from 'htm';
import { Link } from 'preact-router/match';

const html = htm.bind(h);

/**
 * Layout Component
 * Shared layout with navigation for all pages
 * Implements responsive navigation with mobile menu
 *
 * @param {Object} props
 * @param {preact.ComponentChildren} props.children - Page content
 */
export function Layout({ children }) {
  const navItems = [
    { path: '/', label: 'Dashboard', icon: '🏠' },
    { path: '/status', label: 'Status', icon: '📊' },
    { path: '/config', label: 'Config', icon: '⚙️' },
    { path: '/logs', label: 'Logs', icon: '📋' },
    { path: '/serial', label: 'Serial', icon: '🖥️' },
  ];

  return html`
    <div class="min-h-screen bg-gray-50 dark:bg-gray-900">
      <!-- Header with Navigation -->
      <header class="bg-white dark:bg-gray-800 shadow-sm border-b border-gray-200 dark:border-gray-700">
        <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div class="flex justify-between items-center py-4">
            <!-- Logo/Title -->
            <div class="flex items-center gap-3">
              <span class="text-3xl">🌡️</span>
              <h1 class="text-xl sm:text-2xl font-bold text-gray-900 dark:text-white">
                ESP32 Thermostat
              </h1>
            </div>

            <!-- Desktop Navigation -->
            <nav class="hidden md:flex gap-2">
              ${navItems.map(({ path, label, icon }) => html`
                <${Link}
                  key=${path}
                  href=${path}
                  activeClassName="bg-primary-500 text-white"
                  class="px-4 py-2 rounded-lg font-medium transition-all duration-200 hover:bg-primary-100 dark:hover:bg-gray-700 text-gray-700 dark:text-gray-300 flex items-center gap-2"
                >
                  <span>${icon}</span>
                  <span>${label}</span>
                <//>
              `)}
            </nav>

            <!-- Mobile Menu Button (Future Enhancement) -->
            <div class="md:hidden">
              <button class="p-2 text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg">
                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16"></path>
                </svg>
              </button>
            </div>
          </div>

          <!-- Mobile Navigation (Visible on small screens) -->
          <nav class="md:hidden flex overflow-x-auto pb-3 gap-2 -mx-4 px-4">
            ${navItems.map(({ path, label, icon }) => html`
              <${Link}
                key=${path}
                href=${path}
                activeClassName="bg-primary-500 text-white"
                class="flex-shrink-0 px-3 py-2 rounded-lg font-medium transition-all duration-200 text-gray-700 dark:text-gray-300 flex items-center gap-2 text-sm whitespace-nowrap"
              >
                <span>${icon}</span>
                <span>${label}</span>
              <//>
            `)}
          </nav>
        </div>
      </header>

      <!-- Main Content -->
      <main class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        ${children}
      </main>

      <!-- Footer -->
      <footer class="bg-white dark:bg-gray-800 border-t border-gray-200 dark:border-gray-700 mt-12">
        <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
          <p class="text-center text-sm text-gray-500 dark:text-gray-400">
            ESP32 KNX Thermostat © ${new Date().getFullYear()}
          </p>
        </div>
      </footer>
    </div>
  `;
}
