#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "logger.h"

// Define reboot reasons for logging
enum class RebootReason {
  UNKNOWN,
  MANUAL_RESTART,
  SYSTEM_WATCHDOG,
  WIFI_WATCHDOG,
  SOFTWARE_RESET,
  OTA_UPDATE
};

class WatchdogManager {
public:
  WatchdogManager();
  
  // Initialize the watchdog system
  bool begin();
  
  // Reset the system watchdog timer
  void resetSystemWatchdog();
  
  // Enable/disable WiFi watchdog
  void enableWiFiWatchdog(bool enable);
  
  // Check if WiFi watchdog is enabled
  bool isWiFiWatchdogEnabled() const;
  
  // Reset the WiFi watchdog timer
  void resetWiFiWatchdog();
  
  // Check if WiFi watchdog has timed out
  bool checkWiFiWatchdog();
  
  // Register a reboot reason
  void registerRebootReason(RebootReason reason);
  
  // Get the last reboot reason
  RebootReason getLastRebootReason() const;
  
  // Perform a controlled reboot
  void reboot(RebootReason reason);
  
  // Temporarily disable watchdogs for long operations
  void pauseWatchdogs(uint32_t durationMs);
  
  // Resume watchdogs after pausing
  void resumeWatchdogs();

private:
  // System watchdog state
  bool systemWatchdogEnabled;
  
  // WiFi watchdog state
  bool wifiWatchdogEnabled;
  uint32_t lastWiFiWatchdogReset;
  
  // Reboot tracking
  RebootReason lastRebootReason;
  
  // Watchdog pause state
  bool watchdogsPaused;
  uint32_t watchdogPauseEndTime;
  
  // Persistent storage for reboot reasons
  void saveRebootReason(RebootReason reason);
  void loadRebootReason();
  
  // Tag for logging
  static constexpr const char* TAG = "WATCHDOG";
};

#endif // WATCHDOG_MANAGER_H