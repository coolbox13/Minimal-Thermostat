#include "watchdog_manager.h"
#include "config_manager.h"
#include "logger.h"
#include <ArduinoJson.h>  // Add this include for JSON support
#include <Preferences.h>
#include "config.h"
#include <WiFi.h>  // Add this include for WiFi functionality

// Preferences namespace for storing reboot reasons
#define PREF_NAMESPACE "watchdog"
#define PREF_REBOOT_REASON "reboot_reason"

WatchdogManager::WatchdogManager() 
  : systemWatchdogEnabled(false),
    wifiWatchdogEnabled(false),
    lastWiFiWatchdogReset(0),
    lastRebootReason(RebootReason::UNKNOWN),
    watchdogsPaused(false),
    watchdogPauseEndTime(0),
    consecutiveResets(0),
    safeMode(false) {
}

bool WatchdogManager::begin() {
  LOG_I(TAG, "Initializing watchdog manager");
  
  // Load the last reboot reason from persistent storage
  loadRebootReason();
  
  // Check if we should enter safe mode
  if (shouldEnterSafeMode()) {
    enterSafeMode();
  }
  
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

// Add new method to handle watchdog coordination
// Update the update method to include recovery
void WatchdogManager::update() {
  // Skip updates if watchdogs are paused
  if (watchdogsPaused) {
    // Check if pause duration has expired
    if (millis() > watchdogPauseEndTime) {
      resumeWatchdogs();
    }
    return;
  }
  
  // Reset system watchdog (Level 2)
  resetSystemWatchdog();
  
  // Check WiFi watchdog (Level 1)
  if (checkWiFiWatchdog()) {
    LOG_W(TAG, "WiFi watchdog triggered, attempting recovery");
    
    // Implement staged recovery approach
    if (attemptWiFiRecovery()) {
      LOG_I(TAG, "WiFi recovery successful");
      resetWiFiWatchdog();
    } else if (resetWiFiSubsystem()) {
      LOG_I(TAG, "WiFi subsystem reset successful");
      resetWiFiWatchdog();
    } else {
      // If all recovery attempts fail, reboot
      LOG_E(TAG, "WiFi recovery failed, performing controlled reboot");
      incrementConsecutiveResets();
      reboot(RebootReason::WIFI_WATCHDOG);
    }
  }
}

bool WatchdogManager::attemptWiFiRecovery() {
  LOG_I(TAG, "Stage 1: Attempting WiFi reconnection");
  
  // This is a placeholder - the actual implementation will need to
  // coordinate with the WiFiConnectionManager to attempt reconnection
  // Return true if reconnection is successful, false otherwise
  
  // For now, we'll just return false to simulate failed recovery
  return false;
}

bool WatchdogManager::resetWiFiSubsystem() {
  LOG_I(TAG, "Stage 2: Resetting WiFi subsystem");
  
  // Disable WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  
  // Re-enable WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  
  // Wait for connection with timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > 10000) {  // 10 second timeout
      LOG_E(TAG, "WiFi subsystem reset failed");
      return false;
    }
    delay(500);
  }
  
  // Test connectivity
  return testNetworkConnectivity();
}

void WatchdogManager::incrementConsecutiveResets() {
  Preferences preferences;
  if (preferences.begin(PREF_NAMESPACE, false)) {
    consecutiveResets = preferences.getUChar(PREF_CONSECUTIVE_RESETS, 0);
    consecutiveResets++;
    preferences.putUChar(PREF_CONSECUTIVE_RESETS, consecutiveResets);
    LOG_W(TAG, "Consecutive watchdog resets: %d", consecutiveResets);
    preferences.end();
  }
}

bool WatchdogManager::shouldEnterSafeMode() {
  Preferences preferences;
  bool result = false;
  
  if (preferences.begin(PREF_NAMESPACE, true)) {
    consecutiveResets = preferences.getUChar(PREF_CONSECUTIVE_RESETS, 0);
    
    // Check if we've had too many consecutive resets
    if (consecutiveResets >= MAX_CONSECUTIVE_RESETS) {
      LOG_W(TAG, "Too many consecutive resets (%d), should enter safe mode", consecutiveResets);
      result = true;
    }
    
    // Check if the last reboot was due to a watchdog
    if (lastRebootReason == RebootReason::SYSTEM_WATCHDOG || 
        lastRebootReason == RebootReason::WIFI_WATCHDOG) {
      LOG_I(TAG, "Last reboot was due to watchdog");
    } else {
      // If the last reboot was not due to a watchdog, reset the counter
      if (consecutiveResets > 0) {
        preferences.putUChar(PREF_CONSECUTIVE_RESETS, 0);
        LOG_I(TAG, "Reset consecutive watchdog resets counter");
      }
    }
    
    preferences.end();
  }
  
  return result;
}

void WatchdogManager::enterSafeMode() {
  LOG_W(TAG, "Entering safe mode due to repeated watchdog resets");
  safeMode = true;
  
  // In safe mode, we'll disable the WiFi watchdog but keep the system watchdog
  wifiWatchdogEnabled = false;
  
  // Reset the consecutive resets counter
  Preferences preferences;
  if (preferences.begin(PREF_NAMESPACE, false)) {
    preferences.putUChar(PREF_CONSECUTIVE_RESETS, 0);
    preferences.end();
  }
  
  // Register that we entered safe mode
  registerRebootReason(RebootReason::SAFE_MODE);
}

bool WatchdogManager::testNetworkConnectivity() {
  LOG_I(TAG, "Testing network connectivity");
  
  // Simple connectivity test - try to resolve a DNS name
  IPAddress ip;
  bool success = WiFi.hostByName("www.google.com", ip);
  
  if (success) {
    LOG_I(TAG, "Network connectivity test passed");
  } else {
    LOG_W(TAG, "Network connectivity test failed");
  }
  
  return success;
}

// Add to the registerRebootReason method
void WatchdogManager::registerRebootReason(RebootReason reason, const char* details) {
    // Get current time if available
    String timestamp = "";
    // Store reboot reason with timestamp in persistent storage
    
    // Create a JSON document to store the reboot information
    StaticJsonDocument<256> doc;
    doc["reason"] = getRebootReasonName(reason);
    doc["details"] = details ? details : "";
    doc["timestamp"] = millis();
    doc["count"] = ++_rebootCount;
    
    // Convert to string
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    // Log the reboot reason
    LOG_W(TAG_WATCHDOG, "Registering reboot reason: %s - %s", 
          getRebootReasonName(reason), details ? details : "");
    
    // Store in persistent storage using ConfigManager
    ConfigManager::getInstance()->setLastRebootReason(jsonStr);
    ConfigManager::getInstance()->setRebootCount(_rebootCount);
    
    // If this is a watchdog-triggered reboot, increment the consecutive watchdog reboot counter
    if (reason == RebootReason::WATCHDOG_TIMEOUT || 
        reason == RebootReason::WIFI_WATCHDOG) {
        
        int consecutiveWatchdogReboots = ConfigManager::getInstance()->getConsecutiveWatchdogReboots();
        ConfigManager::getInstance()->setConsecutiveWatchdogReboots(consecutiveWatchdogReboots + 1);
        
        LOG_W(TAG_WATCHDOG, "Consecutive watchdog reboots: %d", consecutiveWatchdogReboots + 1);
    } else {
        // Reset the counter for non-watchdog reboots
        ConfigManager::getInstance()->setConsecutiveWatchdogReboots(0);
    }
}

// Helper method to get reboot reason name
const char* WatchdogManager::getRebootReasonName(RebootReason reason) {
    switch (reason) {
        case RebootReason::NORMAL_RESTART: return "Normal Restart";
        case RebootReason::WATCHDOG_TIMEOUT: return "System Watchdog Timeout";
        case RebootReason::WIFI_WATCHDOG: return "WiFi Watchdog Timeout";
        case RebootReason::SYSTEM_WATCHDOG: return "System Watchdog Timeout";
        case RebootReason::OTA_UPDATE: return "OTA Update";
        case RebootReason::USER_REQUESTED: return "User Requested";
        case RebootReason::EXCEPTION: return "System Exception";
        case RebootReason::BROWNOUT: return "Power Brownout";
        case RebootReason::WIFI_RECONNECT_FAILED: return "WiFi Reconnection Failed";
        case RebootReason::SAFE_MODE: return "Safe Mode Activated";
        default: return "Unknown";
    }
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