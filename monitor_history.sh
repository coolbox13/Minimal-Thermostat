#!/bin/bash
# Monitor script for ESP32 history API debugging
# Run with: ./monitor_history.sh
# Stop with: Ctrl+C

ESP32_IP="${1:-192.168.178.54}"
LOG_FILE="history_monitor.log"
INTERVAL=30  # seconds between checks

echo "=== ESP32 History API Monitor ===" | tee "$LOG_FILE"
echo "Target: $ESP32_IP" | tee -a "$LOG_FILE"
echo "Started: $(date)" | tee -a "$LOG_FILE"
echo "Log file: $LOG_FILE" | tee -a "$LOG_FILE"
echo "Press Ctrl+C to stop" | tee -a "$LOG_FILE"
echo "=================================" | tee -a "$LOG_FILE"

check_count=0
fail_count=0

while true; do
  check_count=$((check_count + 1))
  timestamp=$(date '+%Y-%m-%d %H:%M:%S')

  # Get heap memory from status endpoint
  status=$(curl -s --connect-timeout 5 "http://$ESP32_IP/api/status" 2>/dev/null)
  if [ -n "$status" ]; then
    heap=$(echo "$status" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d['system']['free_heap'])" 2>/dev/null)
    uptime=$(echo "$status" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d['system']['uptime'])" 2>/dev/null)
  else
    heap="N/A"
    uptime="N/A"
  fi

  # Get history data
  response=$(curl -s --connect-timeout 10 -w "\n___HTTP_CODE:%{http_code}___SIZE:%{size_download}___" "http://$ESP32_IP/api/history?maxPoints=200" 2>/dev/null)

  # Parse HTTP code and size
  http_code=$(echo "$response" | grep -o '___HTTP_CODE:[0-9]*___' | sed 's/___HTTP_CODE:\([0-9]*\)___/\1/')
  size=$(echo "$response" | grep -o '___SIZE:[0-9]*___' | sed 's/___SIZE:\([0-9]*\)___/\1/')

  # Get actual JSON content (remove the metadata suffix)
  json_content=$(echo "$response" | sed 's/___HTTP_CODE.*//g')

  # Check for failure
  if [ -z "$size" ] || [ "$size" -lt 100 ]; then
    fail_count=$((fail_count + 1))
    echo "" | tee -a "$LOG_FILE"
    echo "!!! [$timestamp] FAILURE #$fail_count (check #$check_count) !!!" | tee -a "$LOG_FILE"
    echo "    HTTP: $http_code, Size: $size bytes" | tee -a "$LOG_FILE"
    echo "    Heap: $heap, Uptime: ${uptime}s" | tee -a "$LOG_FILE"
    echo "    Response content: '$json_content'" | tee -a "$LOG_FILE"
    echo "" | tee -a "$LOG_FILE"
  else
    # Extract data point count from response
    count=$(echo "$json_content" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('count', 'N/A'))" 2>/dev/null)
    debug_mem=$(echo "$json_content" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('_debug',{}).get('memory_used', 'N/A'))" 2>/dev/null)

    echo "[$timestamp] OK: ${size}b, ${count}pts, heap=${heap}, json_mem=${debug_mem}, up=${uptime}s" | tee -a "$LOG_FILE"
  fi

  sleep $INTERVAL
done
