import { h } from 'preact';
import { useEffect, useRef } from 'preact/hooks';
import htm from 'htm';
import uPlot from 'uplot';
import 'uplot/dist/uPlot.min.css';

const html = htm.bind(h);

/**
 * HistoryChart Component
 * Renders time-series chart using uPlot with dual Y-axes
 * Implements spec from GRAPH_VISUALIZATION_REFACTOR.md
 *
 * @param {Object} props
 * @param {number[]} props.timestamps - Unix timestamps in seconds
 * @param {number[]} props.temperatures - Temperature values in °C
 * @param {number[]} props.humidities - Humidity values in %
 * @param {number[]} props.valvePositions - Valve position values 0-100%
 * @param {number} props.timeRange - Selected time range in hours
 */
export function HistoryChart({
  timestamps,
  temperatures,
  humidities,
  valvePositions,
  timeRange,
}) {
  const chartRef = useRef(null);
  const plotRef = useRef(null);

  // Calculate responsive chart height based on viewport
  const getResponsiveHeight = () => {
    const vh = window.innerHeight;
    if (vh < 600) return 250;    // Mobile: 250px
    if (vh < 900) return 300;    // Tablet: 300px
    return 400;                   // Desktop: 400px
  };

  useEffect(() => {
    if (!chartRef.current || !timestamps || timestamps.length === 0) {
      return;
    }

    // Validate and sanitize data arrays
    // uPlot requires arrays of equal length, and null/undefined values should be null
    const sanitizeArray = (arr, defaultValue = null) => {
      if (!arr || !Array.isArray(arr)) return [];
      return arr.map(val => (val === undefined || val === null || isNaN(val)) ? null : Number(val));
    };

    const sanitizedTimestamps = sanitizeArray(timestamps);
    const sanitizedTemperatures = sanitizeArray(temperatures);
    const sanitizedHumidities = sanitizeArray(humidities);
    const sanitizedValvePositions = sanitizeArray(valvePositions);

    // Ensure all arrays have the same length (pad with null if needed)
    const maxLength = Math.max(
      sanitizedTimestamps.length,
      sanitizedTemperatures.length,
      sanitizedHumidities.length,
      sanitizedValvePositions.length
    );

    const padArray = (arr, length) => {
      const padded = [...arr];
      while (padded.length < length) {
        padded.push(null);
      }
      return padded;
    };

    // Prepare data in uPlot format [timestamps, temp, humidity, valve]
    const data = [
      padArray(sanitizedTimestamps, maxLength),
      padArray(sanitizedTemperatures, maxLength),
      padArray(sanitizedHumidities, maxLength),
      padArray(sanitizedValvePositions, maxLength),
    ];

    // Debug logging (remove in production if needed)
    if (process.env.NODE_ENV === 'development') {
      console.log('[HistoryChart] Data summary:', {
        timestamps: data[0].length,
        temperatures: data[1].filter(v => v !== null).length,
        humidities: data[2].filter(v => v !== null).length,
        valvePositions: data[3].filter(v => v !== null).length,
        sampleTemp: data[1].slice(0, 3),
        sampleHumid: data[2].slice(0, 3),
        sampleValve: data[3].slice(0, 3),
      });
    }

    // Configure uPlot options
    const opts = {
      width: chartRef.current.offsetWidth,
      height: getResponsiveHeight(),
      plugins: [],
      scales: {
        x: {
          time: true,
          // Fix X-axis to show full selected time range
          range: (self, dataMin, dataMax) => {
            const now = Math.floor(Date.now() / 1000);
            const rangeStart = now - (timeRange * 3600);
            return [rangeStart, now];
          },
        },
        y: {
          auto: true,
          range: (self, dataMin, dataMax) => {
            // Add 10% padding to temperature axis
            const padding = (dataMax - dataMin) * 0.1;
            return [dataMin - padding, dataMax + padding];
          },
        },
        '%': {
          auto: false, // Fixed range for percentage
          range: [0, 100],
        },
      },
      axes: [
        {
          // X-axis (time) - always show 24h clock format, no dates
          space: 60,
          values: (self, ticks) => {
            return ticks.map(t => {
              const date = new Date(t * 1000);
              // Always use 24h clock format (HH:MM), no dates
              const hours = date.getHours().toString().padStart(2, '0');
              const minutes = date.getMinutes().toString().padStart(2, '0');
              return `${hours}:${minutes}`;
            });
          },
        },
        {
          // Left Y-axis (temperature)
          scale: 'y',
          values: (self, ticks) => ticks.map(t => t.toFixed(1) + '°C'),
          stroke: '#1e88e5',
          font: '12px Arial',
          gap: 5,
        },
        {
          // Right Y-axis (humidity & valve)
          scale: '%',
          values: (self, ticks) => ticks.map(t => t.toFixed(0) + '%'),
          stroke: '#ff9800',
          font: '12px Arial',
          gap: 5,
          side: 1, // Right side
        },
      ],
      series: [
        {
          // Timestamps (x-axis data)
        },
        {
          // Temperature
          label: 'Temperature',
          stroke: '#1e88e5',
          width: 2,
          scale: 'y',
          value: (self, rawValue) => {
            if (rawValue === null || rawValue === undefined || isNaN(rawValue)) {
              return '--';
            }
            return rawValue.toFixed(1) + '°C';
          },
        },
        {
          // Humidity
          label: 'Humidity',
          stroke: '#4caf50',
          width: 2,
          scale: '%',
          value: (self, rawValue) => {
            if (rawValue === null || rawValue === undefined || isNaN(rawValue)) {
              return '--';
            }
            return rawValue.toFixed(1) + '%';
          },
        },
        {
          // Valve Position
          label: 'Valve',
          stroke: '#ff9800',
          width: 2,
          scale: '%',
          value: (self, rawValue) => {
            if (rawValue === null || rawValue === undefined || isNaN(rawValue)) {
              return '--';
            }
            return rawValue.toFixed(0) + '%';
          },
          dash: [5, 5], // Dashed line
        },
      ],
      legend: {
        show: true,
        live: true,
      },
      cursor: {
        drag: {
          x: false,
          y: false,
        },
      },
    };

    // Create or update plot
    if (plotRef.current) {
      plotRef.current.setData(data);
      plotRef.current.setSize({
        width: chartRef.current.offsetWidth,
        height: getResponsiveHeight(),
      });
    } else {
      plotRef.current = new uPlot(opts, data, chartRef.current);
    }

    // Handle window resize
    const handleResize = () => {
      if (plotRef.current && chartRef.current) {
        plotRef.current.setSize({
          width: chartRef.current.offsetWidth,
          height: getResponsiveHeight(),
        });
      }
    };

    window.addEventListener('resize', handleResize);

    // Cleanup
    return () => {
      window.removeEventListener('resize', handleResize);
      if (plotRef.current) {
        plotRef.current.destroy();
        plotRef.current = null;
      }
    };
  }, [timestamps, temperatures, humidities, valvePositions, timeRange]);

  return html`
    <div class="w-full" ref=${chartRef}></div>
  `;
}
