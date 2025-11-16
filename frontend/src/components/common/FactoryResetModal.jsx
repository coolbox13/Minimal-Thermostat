import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';
import { Dialog } from '@headlessui/react';
import toast from '../../utils/toast.js';

const html = htm.bind(h);

const CONFIRMATION_TEXT = 'RESET';
const COUNTDOWN_SECONDS = 10;

/**
 * FactoryResetModal Component
 * Advanced confirmation modal for factory reset with type-to-confirm and countdown
 * Implements destructive action confirmation from PREACT_UX_IMPROVEMENTS.md
 *
 * @param {Object} props
 * @param {boolean} props.isOpen - Whether modal is open
 * @param {Function} props.onClose - Close handler
 * @param {Function} props.onConfirm - Confirm handler (async)
 */
export function FactoryResetModal({ isOpen, onClose, onConfirm }) {
  const [inputValue, setInputValue] = useState('');
  const [countdown, setCountdown] = useState(COUNTDOWN_SECONDS);
  const [isProcessing, setIsProcessing] = useState(false);

  // Reset state when modal opens/closes
  useEffect(() => {
    if (isOpen) {
      setInputValue('');
      setCountdown(COUNTDOWN_SECONDS);
      setIsProcessing(false);
    }
  }, [isOpen]);

  // Countdown timer
  useEffect(() => {
    if (!isOpen || countdown === 0) return;

    const interval = setInterval(() => {
      setCountdown(prev => Math.max(0, prev - 1));
    }, 1000);

    return () => clearInterval(interval);
  }, [isOpen, countdown]);

  const isConfirmEnabled =
    inputValue === CONFIRMATION_TEXT &&
    countdown === 0 &&
    !isProcessing;

  const handleConfirm = async () => {
    if (!isConfirmEnabled) return;

    setIsProcessing(true);

    try {
      await onConfirm();
      toast.success('Factory reset completed successfully');
      onClose();
    } catch (err) {
      toast.error(`Factory reset failed: ${err.message}`);
    } finally {
      setIsProcessing(false);
    }
  };

  return html`
    <${Dialog}
      open=${isOpen}
      onClose=${() => !isProcessing && onClose()}
      class="relative z-50"
    >
      <!-- Backdrop -->
      <div class="fixed inset-0 bg-black/50 backdrop-blur-sm" aria-hidden="true"></div>

      <!-- Full-screen container -->
      <div class="fixed inset-0 flex items-center justify-center p-4">
        <!-- Modal panel -->
        <${Dialog.Panel} class="mx-auto max-w-lg w-full bg-white dark:bg-gray-800 rounded-xl shadow-2xl p-6 animate-fade-in">
          <!-- Warning Icon -->
          <div class="bg-red-100 dark:bg-red-900/20 w-16 h-16 rounded-full flex items-center justify-center mb-4 mx-auto">
            <span class="text-4xl">🚨</span>
          </div>

          <!-- Title -->
          <${Dialog.Title} class="text-2xl font-bold text-center text-gray-900 dark:text-white mb-3">
            Factory Reset
          <//>

          <!-- Warning Message -->
          <div class="bg-red-50 dark:bg-red-900/10 border-2 border-red-200 dark:border-red-800 rounded-lg p-4 mb-4">
            <p class="text-red-800 dark:text-red-200 font-semibold mb-2">
              ⚠️ This action cannot be undone!
            </p>
            <p class="text-red-700 dark:text-red-300 text-sm">
              Factory reset will:
            </p>
            <ul class="text-red-700 dark:text-red-300 text-sm list-disc list-inside mt-2 space-y-1">
              <li>Delete all configuration settings</li>
              <li>Clear WiFi and MQTT credentials</li>
              <li>Reset PID parameters to defaults</li>
              <li>Clear all preset configurations</li>
              <li>Erase temperature history data</li>
            </ul>
          </div>

          <!-- Countdown Timer -->
          ${countdown > 0 && html`
            <div class="bg-orange-50 dark:bg-orange-900/10 border border-orange-200 dark:border-orange-800 rounded-lg p-3 mb-4 text-center">
              <p class="text-orange-700 dark:text-orange-300 font-medium">
                Please wait ${countdown} second${countdown !== 1 ? 's' : ''} before confirming...
              </p>
              <div class="mt-2 h-2 bg-orange-200 dark:bg-orange-800 rounded-full overflow-hidden">
                <div
                  class="h-full bg-orange-500 transition-all duration-1000 ease-linear"
                  style="width: ${((COUNTDOWN_SECONDS - countdown) / COUNTDOWN_SECONDS) * 100}%"
                ></div>
              </div>
            </div>
          `}

          <!-- Type-to-Confirm Input -->
          <div class="mb-6">
            <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              Type <span class="font-mono font-bold text-red-600 dark:text-red-400">${CONFIRMATION_TEXT}</span> to confirm:
            </label>
            <input
              type="text"
              value=${inputValue}
              onInput=${(e) => setInputValue(e.target.value.toUpperCase())}
              disabled=${isProcessing || countdown > 0}
              placeholder=${countdown > 0 ? 'Waiting for countdown...' : `Type "${CONFIRMATION_TEXT}"`}
              class="w-full px-4 py-3 bg-gray-50 dark:bg-gray-700 border-2 ${
                inputValue === CONFIRMATION_TEXT
                  ? 'border-red-500 dark:border-red-400'
                  : 'border-gray-300 dark:border-gray-600'
              } rounded-lg text-gray-900 dark:text-white font-mono font-bold text-center text-lg focus:outline-none focus:ring-2 focus:ring-red-500 disabled:opacity-50 disabled:cursor-not-allowed transition-all"
            />
            ${inputValue && inputValue !== CONFIRMATION_TEXT && html`
              <p class="mt-2 text-sm text-red-600 dark:text-red-400">
                Text does not match. Please type exactly: ${CONFIRMATION_TEXT}
              </p>
            `}
            ${inputValue === CONFIRMATION_TEXT && countdown === 0 && html`
              <p class="mt-2 text-sm text-green-600 dark:text-green-400 flex items-center gap-1">
                <span>✓</span>
                <span>Confirmed. You may now proceed.</span>
              </p>
            `}
          </div>

          <!-- Actions -->
          <div class="flex gap-3">
            <button
              onClick=${onClose}
              disabled=${isProcessing}
              class="flex-1 px-4 py-3 bg-gray-200 hover:bg-gray-300 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-900 dark:text-white rounded-lg font-medium transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
            >
              Cancel
            </button>
            <button
              onClick=${handleConfirm}
              disabled=${!isConfirmEnabled}
              class="flex-1 px-4 py-3 ${
                isConfirmEnabled
                  ? 'bg-red-500 hover:bg-red-600'
                  : 'bg-gray-400 dark:bg-gray-600'
              } text-white rounded-lg font-medium transition-colors disabled:cursor-not-allowed"
            >
              ${isProcessing ? 'Resetting...' : 'Factory Reset'}
            </button>
          </div>
        <//>
      </div>
    <//>
  `;
}
