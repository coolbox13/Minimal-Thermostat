import { h } from 'preact';
import htm from 'htm';

const html = htm.bind(h);

/**
 * DataPointsBadge Component
 * Shows transparency indicator for downsampled data
 * Implements design from GRAPH_VISUALIZATION_REFACTOR.md
 *
 * @param {Object} props
 * @param {number} props.showing - Number of points currently displayed
 * @param {number} props.total - Total number of points in dataset
 * @param {boolean} props.downsampled - Whether data was downsampled
 */
export function DataPointsBadge({ showing, total, downsampled = false }) {
  const formatNumber = (num) => num.toLocaleString();

  return html`
    <div class="flex items-center gap-2 text-sm text-gray-500 dark:text-gray-400">
      <span>
        Showing ${formatNumber(showing)} of ${formatNumber(total)} points
      </span>
      ${downsampled && html`
        <span
          class="px-2 py-0.5 bg-blue-100 dark:bg-blue-900 text-blue-700 dark:text-blue-300 rounded text-xs font-medium"
          title="Data downsampled using LTTB algorithm for optimal performance while preserving visual fidelity"
        >
          LTTB
        </span>
      `}
    </div>
  `;
}
