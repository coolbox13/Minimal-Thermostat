#include "web/web_interface.h"
#include <LittleFS.h>
#include "esp_log.h"
#include <ESPmDNS.h>
#include "web/base64.h"
#include "interfaces/sensor_interface.h"
#include "web/elegant_ota_async.h"

static const char* TAG = "WebInterface";

void WebInterface::listFiles() {
    ESP_LOGI(TAG, "Listing files in LittleFS:");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while(file) {
        ESP_LOGI(TAG, "File: %s, Size: %d bytes", file.name(), file.size());
        file = root.openNextFile();
    }
    root.close();
} 