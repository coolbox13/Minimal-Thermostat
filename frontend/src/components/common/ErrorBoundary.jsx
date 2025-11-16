import { h, Component } from 'preact';
import htm from 'htm';

const html = htm.bind(h);

/**
 * ErrorBoundary Component
 * Catches JavaScript errors in child components and displays fallback UI
 * Implements error boundaries from PREACT_UX_IMPROVEMENTS.md
 */
export class ErrorBoundary extends Component {
  constructor(props) {
    super(props);
    this.state = {
      hasError: false,
      error: null,
      errorInfo: null,
    };
  }

  componentDidCatch(error, errorInfo) {
    // Log error to console
    console.error('ErrorBoundary caught an error:', error, errorInfo);

    // Update state to display fallback UI
    this.setState({
      hasError: true,
      error,
      errorInfo,
    });

    // Optional: Send error to logging service
    // logErrorToService(error, errorInfo);
  }

  resetError = () => {
    this.setState({
      hasError: false,
      error: null,
      errorInfo: null,
    });
  };

  render({ children, fallback }, { hasError, error, errorInfo }) {
    if (hasError) {
      // Custom fallback UI if provided
      if (fallback) {
        return fallback({ error, errorInfo, reset: this.resetError });
      }

      // Default fallback UI
      return html`
        <div class="min-h-screen bg-gray-50 dark:bg-gray-900 flex items-center justify-center p-4">
          <div class="max-w-2xl w-full bg-white dark:bg-gray-800 rounded-xl shadow-xl p-8">
            <!-- Error Icon -->
            <div class="text-center mb-6">
              <div class="inline-flex items-center justify-center w-20 h-20 bg-red-100 dark:bg-red-900/20 rounded-full mb-4">
                <span class="text-5xl">💥</span>
              </div>
              <h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
                Something Went Wrong
              </h1>
              <p class="text-gray-600 dark:text-gray-400">
                The application encountered an unexpected error.
              </p>
            </div>

            <!-- Error Details -->
            <div class="bg-red-50 dark:bg-red-900/10 border border-red-200 dark:border-red-800 rounded-lg p-4 mb-6">
              <h3 class="font-semibold text-red-800 dark:text-red-300 mb-2">
                Error Details:
              </h3>
              <p class="text-sm text-red-700 dark:text-red-400 font-mono break-all">
                ${error?.message || 'Unknown error'}
              </p>

              ${error?.stack && html`
                <details class="mt-3">
                  <summary class="cursor-pointer text-sm text-red-600 dark:text-red-400 hover:text-red-700 dark:hover:text-red-300">
                    Show stack trace
                  </summary>
                  <pre class="mt-2 text-xs text-red-700 dark:text-red-400 overflow-x-auto bg-red-100 dark:bg-red-900/20 p-3 rounded">
${error.stack}
                  </pre>
                </details>
              `}
            </div>

            <!-- Actions -->
            <div class="flex flex-col sm:flex-row gap-3">
              <button
                onClick=${this.resetError}
                class="flex-1 px-6 py-3 bg-primary-500 hover:bg-primary-600 text-white font-medium rounded-lg transition-all shadow-md hover:shadow-lg"
              >
                🔄 Try Again
              </button>
              <button
                onClick=${() => window.location.reload()}
                class="flex-1 px-6 py-3 bg-gray-500 hover:bg-gray-600 text-white font-medium rounded-lg transition-all"
              >
                ↻ Reload Page
              </button>
              <a
                href="/"
                class="flex-1 px-6 py-3 bg-gray-200 hover:bg-gray-300 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white font-medium rounded-lg text-center transition-all"
              >
                🏠 Go Home
              </a>
            </div>

            <!-- Help Text -->
            <div class="mt-6 p-4 bg-blue-50 dark:bg-blue-900/20 border border-blue-200 dark:border-blue-800 rounded-lg">
              <p class="text-sm text-blue-700 dark:text-blue-300">
                <strong>💡 Troubleshooting Tips:</strong>
              </p>
              <ul class="mt-2 text-sm text-blue-600 dark:text-blue-400 list-disc list-inside space-y-1">
                <li>Check your internet connection</li>
                <li>Clear browser cache and reload</li>
                <li>Ensure ESP32 device is powered on</li>
                <li>Check browser console for detailed errors</li>
              </ul>
            </div>
          </div>
        </div>
      `;
    }

    return children;
  }
}
