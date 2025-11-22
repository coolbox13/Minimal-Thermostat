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
    { label: 'Event Logs', href: '/logs' },
    { label: 'Serial Monitor', href: '/serial' },
    { label: 'Firmware Update', href: '/update' },
    { label: 'Configuration', href: '/config' },
    { label: 'System Status', href: '/status' },
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
          class="w-full px-4 py-2 bg-primary-500 hover:bg-primary-600 text-white font-medium rounded-lg transition-all duration-200 shadow-sm hover:shadow-md"
        >
          Refresh Data
        </button>

        <!-- Navigation Links -->
        ${navItems.map(({ label, href }) => html`
          <a
            href=${href}
            class="block w-full px-4 py-2 bg-gray-100 hover:bg-gray-200 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white font-medium rounded-lg transition-all duration-200 text-center shadow-sm hover:shadow-md"
          >
            ${label}
          </a>
        `)}
      </div>
    </div>
  `;
}
