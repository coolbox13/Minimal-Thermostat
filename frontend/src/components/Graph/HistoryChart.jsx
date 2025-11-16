import { h } from 'preact';
import { useEffect, useRef } from 'preact/hooks';
import htm from 'htm';
import uPlot from 'uplot';

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

  useEffect(() => {
    if (!chartRef.current || !timestamps || timestamps.length === 0) {
      return;
    }

    // Prepare data in uPlot format [timestamps, temp, humidity, valve]
    const data = [
      timestamps,
      temperatures,
      humidities,
      valvePositions,
    ];

    // Configure uPlot options
    const opts = {
      width: chartRef.current.offsetWidth,
      height: 300,
      plugins: [],
      scales: {
        x: {
          time: true,
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
          auto: true,
          range: [0, 100],
        },
      },
      axes: [
        {
          // X-axis (time)
          space: 60,
          values: (self, ticks) => {
            return ticks.map(t => {
              const date = new Date(t * 1000);
              if (timeRange <= 4) {
                // Short range: show time only
                return date.toLocaleTimeString('en-US', {
                  hour: '2-digit',
                  minute: '2-digit',
                });
              } else {
                // Long range: show date + time
                return date.toLocaleString('en-US', {
                  month: 'short',
                  day: 'numeric',
                  hour: '2-digit',
                });
              }
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
          value: (self, rawValue) => rawValue?.toFixed(1) + '°C',
        },
        {
          // Humidity
          label: 'Humidity',
          stroke: '#4caf50',
          width: 2,
          scale: '%',
          value: (self, rawValue) => rawValue?.toFixed(1) + '%',
        },
        {
          // Valve Position
          label: 'Valve',
          stroke: '#ff9800',
          width: 2,
          scale: '%',
          value: (self, rawValue) => rawValue?.toFixed(0) + '%',
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
        height: 300,
      });
    } else {
      plotRef.current = new uPlot(opts, data, chartRef.current);
    }

    // Handle window resize
    const handleResize = () => {
      if (plotRef.current && chartRef.current) {
        plotRef.current.setSize({
          width: chartRef.current.offsetWidth,
          height: 300,
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
