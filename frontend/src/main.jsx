import { h, render } from 'preact';
import htm from 'htm';
import Router from 'preact-router';
import { Toaster } from 'react-hot-toast';
import { Layout } from './components/layout/Layout.jsx';
import { Dashboard } from './pages/Dashboard.jsx';
import { Status } from './pages/Status.jsx';
import { Config } from './pages/Config.jsx';
import { Logs } from './pages/Logs.jsx';
import { Serial } from './pages/Serial.jsx';
import './index.css'; // Tailwind CSS

const html = htm.bind(h);

/**
 * Main Application Entry Point
 * Implements client-side routing with Preact Router
 */
function App() {
  return html`
    <div>
      <${Layout}>
        <${Router}>
          <${Dashboard} path="/" />
          <${Status} path="/status" />
          <${Config} path="/config" />
          <${Logs} path="/logs" />
          <${Serial} path="/serial" />
        <//>
      <//>
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
