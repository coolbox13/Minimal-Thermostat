/**
 * Form Validation Utilities
 * Real-time validation for configuration parameters
 * Implements form validation from PREACT_UX_IMPROVEMENTS.md
 */

/**
 * Validates PID parameters
 * @param {number} value - The parameter value
 * @param {string} param - Parameter name (kp, ki, kd)
 * @returns {Object} { isValid: boolean, error: string }
 */
export function validatePID(value, param) {
  const num = parseFloat(value);

  // Check if it's a number
  if (isNaN(num)) {
    return {
      isValid: false,
      error: 'Must be a valid number',
    };
  }

  // Parameter-specific validation
  switch (param.toLowerCase()) {
    case 'kp': // Proportional gain
      if (num < 0.1 || num > 100) {
        return {
          isValid: false,
          error: 'Kp must be between 0.1 and 100',
        };
      }
      break;

    case 'ki': // Integral gain
      if (num < 0 || num > 10) {
        return {
          isValid: false,
          error: 'Ki must be between 0 and 10',
        };
      }
      break;

    case 'kd': // Derivative gain
      if (num < 0 || num > 10) {
        return {
          isValid: false,
          error: 'Kd must be between 0 and 10',
        };
      }
      break;

    default:
      return {
        isValid: false,
        error: 'Unknown PID parameter',
      };
  }

  return {
    isValid: true,
    error: null,
  };
}

/**
 * Validates WiFi SSID
 * @param {string} ssid - WiFi SSID
 * @returns {Object} { isValid: boolean, error: string }
 */
export function validateSSID(ssid) {
  if (!ssid || ssid.trim().length === 0) {
    return {
      isValid: false,
      error: 'SSID cannot be empty',
    };
  }

  if (ssid.length > 32) {
    return {
      isValid: false,
      error: 'SSID cannot exceed 32 characters',
    };
  }

  return {
    isValid: true,
    error: null,
  };
}

/**
 * Validates MQTT broker address
 * @param {string} broker - MQTT broker address
 * @returns {Object} { isValid: boolean, error: string }
 */
export function validateBroker(broker) {
  if (!broker || broker.trim().length === 0) {
    return {
      isValid: false,
      error: 'Broker address cannot be empty',
    };
  }

  // Basic hostname/IP validation
  const hostnameRegex = /^[a-zA-Z0-9.-]+$/;
  if (!hostnameRegex.test(broker)) {
    return {
      isValid: false,
      error: 'Invalid broker address format',
    };
  }

  return {
    isValid: true,
    error: null,
  };
}

/**
 * Validates MQTT port
 * @param {number} port - MQTT port
 * @returns {Object} { isValid: boolean, error: string }
 */
export function validatePort(port) {
  const num = parseInt(port);

  if (isNaN(num)) {
    return {
      isValid: false,
      error: 'Must be a valid number',
    };
  }

  if (num < 1 || num > 65535) {
    return {
      isValid: false,
      error: 'Port must be between 1 and 65535',
    };
  }

  return {
    isValid: true,
    error: null,
  };
}

/**
 * Validates temperature setpoint
 * @param {number} temp - Temperature in Celsius
 * @returns {Object} { isValid: boolean, error: string }
 */
export function validateTemperature(temp) {
  const num = parseFloat(temp);

  if (isNaN(num)) {
    return {
      isValid: false,
      error: 'Must be a valid number',
    };
  }

  if (num < 5 || num > 35) {
    return {
      isValid: false,
      error: 'Temperature must be between 5°C and 35°C',
    };
  }

  return {
    isValid: true,
    error: null,
  };
}
