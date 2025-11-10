#include "webhook_manager.h"
#include "logger.h"

static const char* TAG = "WEBHOOK";

WebhookManager::WebhookManager() : _enabled(false) {
    LOG_I(TAG, "Webhook manager initialized");
}

void WebhookManager::configure(const String& url, bool enabled) {
    _url = url;
    _enabled = enabled;
    LOG_I(TAG, "Webhook configured: enabled=%d, url=%s", enabled, url.c_str());
}

bool WebhookManager::sendEvent(const String& eventName, const String& value1,
                                const String& value2, const String& value3) {
    if (!_enabled || _url.isEmpty()) {
        LOG_D(TAG, "Webhooks disabled or URL not configured");
        return false;
    }

    // Create JSON payload compatible with IFTTT format
    StaticJsonDocument<256> doc;
    doc["event"] = eventName;
    doc["value1"] = value1;
    if (!value2.isEmpty()) {
        doc["value2"] = value2;
    }
    if (!value3.isEmpty()) {
        doc["value3"] = value3;
    }

    String payload;
    serializeJson(doc, payload);

    LOG_I(TAG, "Sending webhook event: %s", eventName.c_str());
    LOG_D(TAG, "Payload: %s", payload.c_str());

    return sendRequest(payload);
}

void WebhookManager::sendTemperatureAlert(float temperature, float threshold, bool isLow) {
    String eventName = isLow ? "temperature_low" : "temperature_high";
    String tempStr = String(temperature, 1) + "°C";
    String thresholdStr = String(threshold, 1) + "°C";
    String message = isLow ?
        "Temperature dropped below threshold" :
        "Temperature exceeded threshold";

    sendEvent(eventName, tempStr, thresholdStr, message);
}

void WebhookManager::sendValveAlert(int valvePosition, unsigned long duration) {
    String valveStr = String(valvePosition) + "%";
    String durationStr = String(duration / 3600) + "h";
    String message = "Valve at " + valveStr + " for " + durationStr;

    sendEvent("valve_alert", valveStr, durationStr, message);
}

void WebhookManager::sendWiFiAlert(bool connected, const String& ssid) {
    String status = connected ? "connected" : "disconnected";
    String message = "WiFi " + status;
    if (connected) {
        message += " to " + ssid;
    }

    sendEvent("wifi_status", status, ssid, message);
}

void WebhookManager::sendSensorError(const String& sensorType, const String& errorMessage) {
    sendEvent("sensor_error", sensorType, errorMessage, "Sensor failure detected");
}

bool WebhookManager::sendRequest(const String& payload) {
    HTTPClient http;

    // Configure HTTP client
    http.begin(_url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);  // 5 second timeout

    // Send POST request
    int httpCode = http.POST(payload);

    // Check response
    bool success = (httpCode >= 200 && httpCode < 300);

    if (success) {
        LOG_I(TAG, "Webhook sent successfully (HTTP %d)", httpCode);
    } else {
        LOG_E(TAG, "Webhook failed (HTTP %d): %s", httpCode, http.errorToString(httpCode).c_str());
    }

    http.end();
    return success;
}
