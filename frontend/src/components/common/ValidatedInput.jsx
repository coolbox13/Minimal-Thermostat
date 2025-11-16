import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import htm from 'htm';

const html = htm.bind(h);

/**
 * ValidatedInput Component
 * Input field with real-time validation feedback
 * Implements real-time form validation from PREACT_UX_IMPROVEMENTS.md
 *
 * @param {Object} props
 * @param {string} props.label - Input label
 * @param {string} props.value - Input value
 * @param {Function} props.onChange - Change handler
 * @param {Function} props.validator - Validation function (value) => { isValid, error }
 * @param {string} [props.type='text'] - Input type
 * @param {string} [props.step] - Step for number inputs
 * @param {string} [props.placeholder] - Placeholder text
 * @param {boolean} [props.required=false] - Whether field is required
 * @param {string} [props.helpText] - Help text shown below input
 */
export function ValidatedInput({
  label,
  value,
  onChange,
  validator,
  type = 'text',
  step,
  placeholder,
  required = false,
  helpText,
}) {
  const [touched, setTouched] = useState(false);
  const [validation, setValidation] = useState({ isValid: true, error: null });

  // Validate on value change
  useEffect(() => {
    if (validator && value !== '' && value !== null && value !== undefined) {
      const result = validator(value);
      setValidation(result);
    } else if (!required && (value === '' || value === null || value === undefined)) {
      // Empty is valid if not required
      setValidation({ isValid: true, error: null });
    }
  }, [value, validator, required]);

  const handleBlur = () => {
    setTouched(true);
  };

  const showError = touched && !validation.isValid;
  const showSuccess = touched && validation.isValid && value !== '' && value !== null;

  return html`
    <div class="mb-4">
      <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
        ${label}
        ${required && html`<span class="text-red-500 ml-1">*</span>`}
      </label>

      <div class="relative">
        <input
          type=${type}
          step=${step}
          value=${value}
          onInput=${(e) => onChange(e.target.value)}
          onBlur=${handleBlur}
          placeholder=${placeholder}
          class="${
            showError
              ? 'border-red-500 focus:ring-red-500'
              : showSuccess
              ? 'border-green-500 focus:ring-green-500'
              : 'border-gray-300 dark:border-gray-600 focus:ring-primary-500'
          } w-full px-4 py-2 bg-gray-50 dark:bg-gray-700 border rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 transition-all"
        />

        <!-- Validation Icon -->
        ${showError && html`
          <div class="absolute right-3 top-1/2 transform -translate-y-1/2 text-red-500">
            <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
              <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z" clip-rule="evenodd"></path>
            </svg>
          </div>
        `}
        ${showSuccess && html`
          <div class="absolute right-3 top-1/2 transform -translate-y-1/2 text-green-500">
            <svg class="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
              <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clip-rule="evenodd"></path>
            </svg>
          </div>
        `}
      </div>

      <!-- Error Message -->
      ${showError && html`
        <p class="mt-1 text-sm text-red-600 dark:text-red-400 flex items-center gap-1">
          <span>⚠️</span>
          <span>${validation.error}</span>
        </p>
      `}

      <!-- Help Text -->
      ${helpText && !showError && html`
        <p class="mt-1 text-sm text-gray-500 dark:text-gray-400">
          ${helpText}
        </p>
      `}
    </div>
  `;
}
