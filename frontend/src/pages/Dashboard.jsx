import { h } from 'preact';
import htm from 'htm';
import { GraphContainer } from '../components/Graph/GraphContainer.jsx';
import { SensorCard } from '../components/Dashboard/SensorCard.jsx';
import { ControlCard } from '../components/Dashboard/ControlCard.jsx';
import { ManualOverrideCard } from '../components/Dashboard/ManualOverrideCard.jsx';
import { SystemCard } from '../components/Dashboard/SystemCard.jsx';

const html = htm.bind(h);

/**
 * Dashboard Page
 * Main landing page showing sensor data, controls, and history graph
 * Implements skeleton loading from PREACT_UX_IMPROVEMENTS.md
 */
export function Dashboard() {
  return html`
    <div>
      <!-- Two-column layout on desktop, stacked on mobile -->
      <div class="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
        <!-- Left Column: Sensor Readings -->
        <div class="space-y-6">
          <${SensorCard} />
        </div>

        <!-- Right Column: Controls -->
        <div class="space-y-6">
          <${ControlCard} />
          <${ManualOverrideCard} />
          <${SystemCard} />
        </div>
      </div>

      <!-- Full-width Graph Section -->
      <div class="mt-6">
        <${GraphContainer} />
      </div>
    </div>
  `;
}
