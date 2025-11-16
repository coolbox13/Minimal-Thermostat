import toast from 'react-hot-toast';

/**
 * Toast Notification Utilities
 * Wrapper around react-hot-toast with custom styling
 * Implements toast notifications from PREACT_UX_IMPROVEMENTS.md
 */

// Default toast options
const defaultOptions = {
  duration: 3000,
  position: 'top-center',
  style: {
    borderRadius: '12px',
    padding: '12px 16px',
    fontSize: '14px',
    maxWidth: '500px',
  },
};

/**
 * Success toast notification
 * @param {string} message - Success message to display
 */
export function showSuccess(message) {
  return toast.success(message, {
    ...defaultOptions,
    icon: '✅',
    style: {
      ...defaultOptions.style,
      background: '#10b981',
      color: '#ffffff',
    },
  });
}

/**
 * Error toast notification
 * @param {string} message - Error message to display
 */
export function showError(message) {
  return toast.error(message, {
    ...defaultOptions,
    duration: 4000, // Errors stay longer
    icon: '❌',
    style: {
      ...defaultOptions.style,
      background: '#ef4444',
      color: '#ffffff',
    },
  });
}

/**
 * Info toast notification
 * @param {string} message - Info message to display
 */
export function showInfo(message) {
  return toast(message, {
    ...defaultOptions,
    icon: 'ℹ️',
    style: {
      ...defaultOptions.style,
      background: '#3b82f6',
      color: '#ffffff',
    },
  });
}

/**
 * Warning toast notification
 * @param {string} message - Warning message to display
 */
export function showWarning(message) {
  return toast(message, {
    ...defaultOptions,
    icon: '⚠️',
    style: {
      ...defaultOptions.style,
      background: '#f59e0b',
      color: '#ffffff',
    },
  });
}

/**
 * Loading toast notification
 * @param {string} message - Loading message to display
 * @returns {string} Toast ID (use with toast.dismiss(id))
 */
export function showLoading(message) {
  return toast.loading(message, {
    ...defaultOptions,
    duration: Infinity, // Loading toasts don't auto-dismiss
    style: {
      ...defaultOptions.style,
      background: '#6b7280',
      color: '#ffffff',
    },
  });
}

/**
 * Promise toast - shows loading/success/error based on promise state
 * @param {Promise} promise - Promise to track
 * @param {Object} messages - Messages for each state {loading, success, error}
 */
export function showPromise(promise, messages) {
  return toast.promise(
    promise,
    {
      loading: messages.loading,
      success: messages.success,
      error: messages.error,
    },
    {
      ...defaultOptions,
      success: {
        icon: '✅',
        style: {
          ...defaultOptions.style,
          background: '#10b981',
          color: '#ffffff',
        },
      },
      error: {
        icon: '❌',
        style: {
          ...defaultOptions.style,
          background: '#ef4444',
          color: '#ffffff',
        },
      },
    }
  );
}

/**
 * Dismiss a specific toast or all toasts
 * @param {string} [id] - Optional toast ID to dismiss specific toast
 */
export function dismissToast(id) {
  if (id) {
    toast.dismiss(id);
  } else {
    toast.dismiss();
  }
}

export default {
  success: showSuccess,
  error: showError,
  info: showInfo,
  warning: showWarning,
  loading: showLoading,
  promise: showPromise,
  dismiss: dismissToast,
};
