#include "watchdog_manager.h"
#include <Preferences.h>
#include "config.h"

// Preferences namespace for storing reboot reasons
#define PREF_NAMESPACE "watchdog"
#define PREF_REBOOT_REASON "reboot_reason"

WatchdogManager::WatchdogManager() 
  : systemWatchdogEnabled(false),
    wifiWatchdogEnabled(false),
    lastWiFiWatchdogReset(0),
    lastRebootReason(RebootReason::UNKNOWN),
    watchdogsPaused(false),
    watchdogPauseEndTime(0) {
}

bool WatchdogManager::begin() {
  LOG_I(TAG, "Initializing watchdog manager");
  
  // Load the last reboot reason from persistent storage
  loadRebootReason();
  
  // Initialize the ESP task watchdog (convert ms to seconds)
  esp_err_t err = esp_task_wdt_init(SYSTEM_WATCHDOG_TIMEOUT / 1000, true);
  if (err != ESP_OK) {
    LOG_E(TAG, "Failed to initialize system watchdog: %d", err);
    return false;
  }
  
  // Add current task to watchdog
  err = esp_task_wdt_add(NULL);
  if (err != ESP_OK) {
    LOG_E(TAG, "Failed to add task to watchdog: %d", err);
    return false;
  }
  
  systemWatchdogEnabled = true;
  LOG_I(TAG, "System watchdog initialized with timeout of %d ms", SYSTEM_WATCHDOG_TIMEOUT);
  
  // Initialize WiFi watchdog
  wifiWatchdogEnabled = true;
  lastWiFiWatchdogReset = millis();
  LOG_I(TAG, "WiFi watchdog initialized with timeout of %d ms", WIFI_WATCHDOG_TIMEOUT);
  
  return true;
}

void WatchdogManager::resetSystemWatchdog() {
  if (systemWatchdogEnabled && !watchdogsPaused) {
    esp_task_wdt_reset();
  }
}

void WatchdogManager::enableWiFiWatchdog(bool enable) {
  if (wifiWatchdogEnabled != enable) {
    wifiWatchdogEnabled = enable;
    LOG_I(TAG, "WiFi watchdog %s", enable ? "enabled" : "disabled");
    
    if (enable) {
      // Reset the timer when enabling
      lastWiFiWatchdogReset = millis();
    }
  }
}

bool WatchdogManager::isWiFiWatchdogEnabled() const {
  return wifiWatchdogEnabled && !watchdogsPaused;
}

void WatchdogManager::resetWiFiWatchdog() {
  lastWiFiWatchdogReset = millis();
}

bool WatchdogManager::checkWiFiWatchdog() {
  if (!wifiWatchdogEnabled || watchdogsPaused) {
    return false;
  }
  
  uint32_t currentTime = millis();
  uint32_t elapsed = currentTime - lastWiFiWatchdogReset;
  
  // Check if WiFi watchdog has timed out
  if (elapsed >= WIFI_WATCHDOG_TIMEOUT) {
    LOG_W(TAG, "WiFi watchdog timeout after %d ms", elapsed);
    return true;
  }
  
  return false;
}

void WatchdogManager::registerRebootReason(RebootReason reason) {
  saveRebootReason(reason);
}

RebootReason WatchdogManager::getLastRebootReason() const {
  return lastRebootReason;
}

void WatchdogManager::reboot(RebootReason reason) {
  LOG_W(TAG, "Performing controlled reboot, reason: %d", static_cast<int>(reason));
  
  // Save the reboot reason before rebooting
  saveRebootReason(reason);
  
  // Small delay to ensure logs are written
  delay(100);
  
  // Perform the reboot
  ESP.restart();
}

void WatchdogManager::pauseWatchdogs(uint32_t durationMs) {
  watchdogsPaused = true;
  watchdogPauseEndTime = millis() + durationMs;
  LOG_I(TAG, "Watchdogs paused for %d ms", durationMs);
}

void WatchdogManager::resumeWatchdogs() {
  if (watchdogsPaused) {
    watchdogsPaused = false;
    // Reset both watchdogs when resuming
    resetSystemWatchdog();
    resetWiFiWatchdog();
    LOG_I(TAG, "Watchdogs resumed");
  }
}

void WatchdogManager::saveRebootReason(RebootReason reason) {
  Preferences preferences;
  if (preferences.begin(PREF_NAMESPACE, false)) {
    preferences.putUInt(PREF_REBOOT_REASON, static_cast<uint32_t>(reason));
    preferences.end();
  }
}

void WatchdogManager::loadRebootReason() {
  Preferences preferences;
  if (preferences.begin(PREF_NAMESPACE, true)) {
    uint32_t reason = preferences.getUInt(PREF_REBOOT_REASON, static_cast<uint32_t>(RebootReason::UNKNOWN));
    lastRebootReason = static_cast<RebootReason>(reason);
    preferences.end();
    
    LOG_I(TAG, "Last reboot reason: %d", static_cast<int>(lastRebootReason));
  }
}