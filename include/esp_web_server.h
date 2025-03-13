#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "config_manager.h"
#include "thermostat_state.h"
#include "pid_controller.h"

/**
 * @brief Web server implementation for ESP32
 * 
 * This class provides a web interface for the thermostat
 */
class ESPWebServer {
public:
    /**
     * @brief Construct a new ESPWebServer object
     * 
     * @param configManager Configuration manager
     * @param state Thermostat state
     */
    ESPWebServer(ConfigManager* configManager, ThermostatState* state);
    
    /**
     * @brief Initialize the web server
     * 
     * @return true if successful
     * @return false if failed
     */
    bool begin();
    
    /**
     * @brief Set the port for the web server
     * 
     * @param port Port number
     */
    void setPort(uint16_t port);
    
    /**
     * @brief Set authentication credentials
     * 
     * @param username Username
     * @param password Password
     */
    void setCredentials(const char* username, const char* password);
    
    /**
     * @brief Set the hostname for the web server
     * 
     * @param hostname Hostname
     */
    void setHostname(const char* hostname);
    
    /**
     * @brief Register components with the web server
     * 
     * @param state Thermostat state
     * @param pidController PID controller
     */
    void registerComponents(ThermostatState* state, PIDController* pidController);
    
    /**
     * @brief Get the last error
     * 
     * @return ThermostatStatus 
     */
    ThermostatStatus getLastError() const;

    /**
     * @brief Generate HTML for the web interface
     * 
     * @return String HTML content
     */
    String generateHtml();
    
    /**
     * @brief Generate JSON representation of status
     * 
     * @return String JSON content
     */
    String generateStatusJson();
    
    /**
     * @brief Generate JSON representation of configuration
     * 
     * @return String JSON content
     */
    String generateConfigJson();

    /**
     * @brief Get the content type for a file
     * 
     * @param filename File name
     * @return String content type
     */
    String getContentType(const String &filename);


private:
    AsyncWebServer server;
    ConfigManager* configManager;
    ThermostatState* thermostatState;
    PIDController* pidController;
    uint16_t port;
    char username[32];
    char password[32];
    char hostname[32];
    ThermostatStatus lastError;
    
    // Setup methods
    void setupRoutes();
    bool isAuthenticated();
    void requestAuthentication();
    
    // Request handlers
    void handleRoot();
    void handleSave();
    void handleSetpoint();
    void handleMode();
    void handleStatus();
    void handleConfig();
    void handleReboot();
    void handleReset();
    void handleNotFound();
    
    // Helper methods
    bool handleFileRead(String path);
    void handleJsonResponse(String& json);
    void handleError(const char* message, int code);
    //String getContentType(const String &filename);
}; 