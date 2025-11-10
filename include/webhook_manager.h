#ifndef WEBHOOK_MANAGER_H
#define WEBHOOK_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
 * @brief Manages webhook integrations for IFTTT, Zapier, and custom automation
 *
 * Supports sending events to external services via HTTP POST with JSON payloads.
 * Useful for notifications, alerts, and integration with automation platforms.
 */
class WebhookManager {
public:
    /**
     * @brief Construct a new Webhook Manager
     */
    WebhookManager();

    /**
     * @brief Configure webhook URL and enable/disable
     *
     * @param url Webhook endpoint URL (e.g., "https://maker.ifttt.com/trigger/{event}/with/key/{key}")
     * @param enabled Whether webhooks are enabled
     */
    void configure(const String& url, bool enabled);

    /**
     * @brief Send a webhook event
     *
     * @param eventName Name of the event (e.g., "temperature_alert")
     * @param value1 First value to send
     * @param value2 Second value to send (optional)
     * @param value3 Third value to send (optional)
     * @return true if webhook was sent successfully, false otherwise
     */
    bool sendEvent(const String& eventName, const String& value1,
                   const String& value2 = "", const String& value3 = "");

    /**
     * @brief Send temperature threshold alert
     *
     * @param temperature Current temperature
     * @param threshold Threshold that was crossed
     * @param isLow True if temperature is below threshold, false if above
     */
    void sendTemperatureAlert(float temperature, float threshold, bool isLow);

    /**
     * @brief Send valve position alert
     *
     * @param valvePosition Current valve position (0-100%)
     * @param duration How long valve has been at this position (seconds)
     */
    void sendValveAlert(int valvePosition, unsigned long duration);

    /**
     * @brief Send WiFi connection status alert
     *
     * @param connected True if WiFi connected, false if disconnected
     * @param ssid WiFi SSID
     */
    void sendWiFiAlert(bool connected, const String& ssid);

    /**
     * @brief Send sensor error alert
     *
     * @param sensorType Type of sensor (e.g., "BME280")
     * @param errorMessage Error description
     */
    void sendSensorError(const String& sensorType, const String& errorMessage);

    /**
     * @brief Check if webhooks are enabled
     *
     * @return true if webhooks are enabled, false otherwise
     */
    bool isEnabled() const { return _enabled; }

    /**
     * @brief Get configured webhook URL
     *
     * @return Webhook URL
     */
    String getUrl() const { return _url; }

private:
    String _url;
    bool _enabled;

    /**
     * @brief Send HTTP POST request to webhook URL
     *
     * @param payload JSON payload to send
     * @return true if request was successful, false otherwise
     */
    bool sendRequest(const String& payload);
};

#endif // WEBHOOK_MANAGER_H
