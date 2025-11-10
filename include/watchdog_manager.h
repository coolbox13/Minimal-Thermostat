#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "logger.h"

// Define reboot reasons for logging
// Add these new enum values to RebootReason if not already present
// Add this enum before the WatchdogManager class definition

/**
 * @brief Reasons for system reboot
 */
enum class RebootReason {
    NORMAL_RESTART,
    WATCHDOG_TIMEOUT,
    WIFI_WATCHDOG,  // Use this consistently instead of WIFI_WATCHDOG_TIMEOUT
    SYSTEM_WATCHDOG, // Add this missing value
    OTA_UPDATE,
    USER_REQUESTED,
    EXCEPTION,
    BROWNOUT,
    WIFI_RECONNECT_FAILED,
    SAFE_MODE,      // Add this missing value
    UNKNOWN
};

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
  /**
   * @brief Register a reboot reason with additional details
   * @param reason The reason for the reboot
   * @param details Additional details about the reboot
   */
  void registerRebootReason(RebootReason reason, const char* details = nullptr);
  
  /**
   * @brief Get the string representation of a reboot reason
   * @param reason The reboot reason
   * @return String representation of the reboot reason
   */
  const char* getRebootReasonName(RebootReason reason);
  
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
  uint32_t wifiWatchdogTimeout;

  // System watchdog timeout
  uint32_t systemWatchdogTimeout;

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
  
  // Add reboot count tracking
  int _rebootCount = 0;
};

#endif // WATCHDOG_MANAGER_H