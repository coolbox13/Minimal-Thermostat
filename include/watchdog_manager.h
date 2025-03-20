#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "logger.h"

// Define reboot reasons for logging
// Add these new enum values to RebootReason if not already present
enum class RebootReason {
  UNKNOWN = 0,
  MANUAL = 1,
  SYSTEM_WATCHDOG = 2,
  WIFI_WATCHDOG = 3,
  WIFI_RECONNECT_FAILED = 4,
  OTA_UPDATE = 5,
  WIFI_RESET = 6,  // New: for WiFi subsystem reset
  SAFE_MODE = 7    // New: for safe mode boot
};

// Add these new constants for safe mode
#define PREF_CONSECUTIVE_RESETS "consecutive_resets"
#define MAX_CONSECUTIVE_RESETS 3  // Enter safe mode after 3 consecutive watchdog resets

class WatchdogManager {
public:
  WatchdogManager();
  
  // Initialize the watchdog system
  bool begin();
  
  // Update method to be called in the main loop
  void update();
  
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
  
  // New methods for recovery mechanisms
  bool attemptWiFiRecovery();
  bool resetWiFiSubsystem();
  void incrementConsecutiveResets();
  bool shouldEnterSafeMode();
  void enterSafeMode();
  bool testNetworkConnectivity();
  bool isInSafeMode() const { return safeMode; }
  
private:
  // New members for recovery tracking
  uint8_t consecutiveResets;
  bool safeMode;
};

#endif // WATCHDOG_MANAGER_H