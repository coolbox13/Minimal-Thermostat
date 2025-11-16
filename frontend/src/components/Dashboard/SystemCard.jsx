import { h } from 'preact';
import htm from 'htm';

const html = htm.bind(h);

/**
 * SystemCard Component
 * System navigation and actions
 */
export function SystemCard() {
  const handleRefresh = () => {
    window.location.reload();
  };

  const navItems = [
    { label: 'Event Logs', href: '/logs', icon: '📋' },
    { label: 'Serial Monitor', href: '/serial', icon: '🖥️' },
    { label: 'Firmware Update', href: '/update', icon: '⬆️' },
    { label: 'Configuration', href: '/config', icon: '⚙️' },
    { label: 'System Status', href: '/status', icon: '📊' },
  ];

  return html`
    <div class="bg-white dark:bg-gray-800 rounded-xl shadow-lg p-6">
      <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-4">
        System
      </h2>

      <div class="space-y-2">
        <!-- Refresh Button -->
        <button
          onClick=${handleRefresh}
          class="w-full px-4 py-2 bg-primary-500 hover:bg-primary-600 text-white font-medium rounded-lg transition-all duration-200 shadow-sm hover:shadow-md flex items-center justify-center gap-2"
        >
          <span>🔄</span>
          <span>Refresh Data</span>
        </button>

        <!-- Navigation Links -->
        ${navItems.map(({ label, href, icon }) => html`
          <a
            href=${href}
            class="block w-full px-4 py-2 bg-gray-100 hover:bg-gray-200 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white font-medium rounded-lg transition-all duration-200 text-center shadow-sm hover:shadow-md"
          >
            <span class="mr-2">${icon}</span>
            ${label}
          </a>
        `)}
      </div>
    </div>
  `;
}
