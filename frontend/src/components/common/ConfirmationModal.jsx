import { h } from 'preact';
import htm from 'htm';
import { Dialog } from '@headlessui/react';

const html = htm.bind(h);

/**
 * ConfirmationModal Component
 * Reusable confirmation dialog for dangerous actions
 * Implements modal confirmation from PREACT_UX_IMPROVEMENTS.md
 *
 * @param {Object} props
 * @param {boolean} props.isOpen - Whether modal is open
 * @param {Function} props.onClose - Close handler
 * @param {Function} props.onConfirm - Confirm handler
 * @param {string} props.title - Modal title
 * @param {string} props.message - Modal message
 * @param {string} [props.confirmText='Confirm'] - Confirm button text
 * @param {string} [props.cancelText='Cancel'] - Cancel button text
 * @param {'danger'|'warning'|'info'} [props.variant='danger'] - Visual variant
 * @param {boolean} [props.isLoading=false] - Loading state
 */
export function ConfirmationModal({
  isOpen,
  onClose,
  onConfirm,
  title,
  message,
  confirmText = 'Confirm',
  cancelText = 'Cancel',
  variant = 'danger',
  isLoading = false,
}) {
  const variantStyles = {
    danger: {
      icon: '⚠️',
      iconBg: 'bg-red-100 dark:bg-red-900/20',
      iconText: 'text-red-600 dark:text-red-400',
      button: 'bg-red-500 hover:bg-red-600 focus:ring-red-500',
    },
    warning: {
      icon: '⚠️',
      iconBg: 'bg-orange-100 dark:bg-orange-900/20',
      iconText: 'text-orange-600 dark:text-orange-400',
      button: 'bg-orange-500 hover:bg-orange-600 focus:ring-orange-500',
    },
    info: {
      icon: 'ℹ️',
      iconBg: 'bg-blue-100 dark:bg-blue-900/20',
      iconText: 'text-blue-600 dark:text-blue-400',
      button: 'bg-blue-500 hover:bg-blue-600 focus:ring-blue-500',
    },
  };

  const style = variantStyles[variant];

  return html`
    <${Dialog}
      open=${isOpen}
      onClose=${onClose}
      class="relative z-50"
    >
      <!-- Backdrop -->
      <div class="fixed inset-0 bg-black/30 backdrop-blur-sm" aria-hidden="true"></div>

      <!-- Full-screen container -->
      <div class="fixed inset-0 flex items-center justify-center p-4">
        <!-- Modal panel -->
        <${Dialog.Panel} class="mx-auto max-w-md w-full bg-white dark:bg-gray-800 rounded-xl shadow-2xl p-6 animate-fade-in">
          <!-- Icon -->
          <div class="${style.iconBg} w-12 h-12 rounded-full flex items-center justify-center mb-4">
            <span class="text-2xl">${style.icon}</span>
          </div>

          <!-- Title -->
          <${Dialog.Title} class="text-xl font-bold text-gray-900 dark:text-white mb-2">
            ${title}
          <//>

          <!-- Message -->
          <${Dialog.Description} class="text-gray-600 dark:text-gray-400 mb-6">
            ${message}
          <//>

          <!-- Actions -->
          <div class="flex gap-3 justify-end">
            <button
              onClick=${onClose}
              disabled=${isLoading}
              class="px-4 py-2 bg-gray-200 hover:bg-gray-300 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white rounded-lg font-medium transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
            >
              ${cancelText}
            </button>
            <button
              onClick=${onConfirm}
              disabled=${isLoading}
              class="${style.button} px-4 py-2 text-white rounded-lg font-medium transition-colors focus:outline-none focus:ring-2 focus:ring-offset-2 disabled:opacity-50 disabled:cursor-not-allowed"
            >
              ${isLoading ? 'Processing...' : confirmText}
            </button>
          </div>
        <//>
      </div>
    <//>
  `;
}
