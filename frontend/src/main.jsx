import { h, render } from 'preact';
import htm from 'htm';
import { Toaster } from 'react-hot-toast';
import { Dashboard } from './pages/Dashboard.jsx';
import './index.css'; // Tailwind CSS

const html = htm.bind(h);

/**
 * Main Application Entry Point
 * Renders the Dashboard page with toast notifications
 */
function App() {
  return html`
    <div>
      <${Dashboard} />
      <${Toaster}
        position="top-center"
        reverseOrder=${false}
        gutter=${8}
        toastOptions=${{
          duration: 3000,
          style: {
            borderRadius: '12px',
            padding: '12px 16px',
            fontSize: '14px',
            fontWeight: '500',
            maxWidth: '500px',
          },
        }}
      />
    </div>
  `;
}

// Mount app to DOM
render(html`<${App} />`, document.getElementById('app'));
