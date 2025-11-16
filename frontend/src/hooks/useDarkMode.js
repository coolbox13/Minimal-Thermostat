import { useState, useEffect } from 'preact/hooks';

/**
 * useDarkMode Hook
 * Manages dark mode state with localStorage persistence
 * Implements dark mode from PREACT_UX_IMPROVEMENTS.md
 */
export function useDarkMode() {
  // Check system preference or localStorage
  const [isDark, setIsDark] = useState(() => {
    // First check localStorage
    const stored = localStorage.getItem('darkMode');
    if (stored !== null) {
      return stored === 'true';
    }

    // Fall back to system preference
    if (window.matchMedia) {
      return window.matchMedia('(prefers-color-scheme: dark)').matches;
    }

    return false;
  });

  // Apply dark mode class to HTML element
  useEffect(() => {
    if (isDark) {
      document.documentElement.classList.add('dark');
    } else {
      document.documentElement.classList.remove('dark');
    }

    // Persist to localStorage
    localStorage.setItem('darkMode', isDark.toString());
  }, [isDark]);

  const toggle = () => setIsDark(!isDark);

  return { isDark, toggle };
}
