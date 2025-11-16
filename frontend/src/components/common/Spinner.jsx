import { h } from 'preact';
import htm from 'htm';

const html = htm.bind(h);

/**
 * Spinner Component
 * Reusable loading spinner with different sizes and colors
 * Implements loading indicators from PREACT_UX_IMPROVEMENTS.md
 *
 * @param {Object} props
 * @param {'sm'|'md'|'lg'|'xl'} [props.size='md'] - Spinner size
 * @param {'primary'|'white'|'gray'} [props.color='primary'] - Spinner color
 * @param {string} [props.label] - Optional label text
 * @param {boolean} [props.fullscreen=false] - Whether to show fullscreen overlay
 */
export function Spinner({ size = 'md', color = 'primary', label, fullscreen = false }) {
  const sizes = {
    sm: 'w-4 h-4 border-2',
    md: 'w-8 h-8 border-2',
    lg: 'w-12 h-12 border-3',
    xl: 'w-16 h-16 border-4',
  };

  const colors = {
    primary: 'border-primary-500 border-t-transparent',
    white: 'border-white border-t-transparent',
    gray: 'border-gray-400 dark:border-gray-600 border-t-transparent',
  };

  const spinner = html`
    <div
      class="${sizes[size]} ${colors[color]} rounded-full animate-spin"
      role="status"
      aria-label=${label || 'Loading...'}
    ></div>
  `;

  if (fullscreen) {
    return html`
      <div class="fixed inset-0 bg-black/50 backdrop-blur-sm flex items-center justify-center z-50">
        <div class="bg-white dark:bg-gray-800 rounded-xl shadow-2xl p-8 flex flex-col items-center gap-4">
          ${spinner}
          ${label && html`
            <p class="text-gray-700 dark:text-gray-300 font-medium">${label}</p>
          `}
        </div>
      </div>
    `;
  }

  if (label) {
    return html`
      <div class="flex items-center gap-3">
        ${spinner}
        <span class="text-gray-700 dark:text-gray-300">${label}</span>
      </div>
    `;
  }

  return spinner;
}

/**
 * LoadingOverlay Component
 * Full-screen loading overlay with message
 *
 * @param {Object} props
 * @param {string} [props.message='Loading...'] - Loading message
 */
export function LoadingOverlay({ message = 'Loading...' }) {
  return html`<${Spinner} size="lg" fullscreen=${true} label=${message} />`;
}

/**
 * InlineSpinner Component
 * Small inline spinner for buttons and compact spaces
 *
 * @param {Object} props
 * @param {string} [props.label] - Optional label
 */
export function InlineSpinner({ label }) {
  return html`<${Spinner} size="sm" color="white" label=${label} />`;
}
