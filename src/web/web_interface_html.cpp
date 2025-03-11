#include <Arduino.h>
#include "web/web_interface.h"
#include <ESPmDNS.h>
#include <LittleFS.h>

String WebInterface::generateHtml() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        ESP_LOGE("WebInterface", "Failed to open index.html");
        return "Error: Failed to load web interface";
    }
    String html = file.readString();
    file.close();
    return html;
}