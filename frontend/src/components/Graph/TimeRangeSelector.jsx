import { h } from 'preact';
import htm from 'htm';

const html = htm.bind(h);

/**
 * TimeRangeSelector Component
 * Renders button group for selecting time range (1h, 4h, 12h, 24h)
 * Implements design from GRAPH_VISUALIZATION_REFACTOR.md
 *
 * @param {Object} props
 * @param {number} props.selected - Currently selected time range in hours
 * @param {Function} props.onChange - Callback when selection changes
 */
export function TimeRangeSelector({ selected, onChange }) {
  const ranges = [
    { hours: 1, label: '1h' },
    { hours: 4, label: '4h' },
    { hours: 12, label: '12h' },
    { hours: 24, label: '24h' },
  ];

  return html`
    <div class="flex gap-2 justify-center mb-4">
      ${ranges.map(({ hours, label }) => html`
        <button
          key=${hours}
          onClick=${() => onChange(hours)}
          class=${`
            px-4 py-2 rounded-lg font-medium transition-all duration-200
            ${selected === hours
              ? 'bg-primary-500 text-white shadow-md scale-105'
              : 'bg-gray-100 text-gray-700 hover:bg-gray-200 dark:bg-gray-700 dark:text-gray-300 dark:hover:bg-gray-600'
            }
          `}
        >
          ${label}
        </button>
      `)}
    </div>
  `;
}
