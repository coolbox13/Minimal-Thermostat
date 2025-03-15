#include "ota_manager.h"

OTAManager::OTAManager() : _server(nullptr) {
}

void OTAManager::begin(AsyncWebServer* server) {
    _server = server;
    
    if (!_server) {
        Serial.println("OTA Manager: No web server provided");
        return;
    }
    
    // Add handler for firmware update page
    _server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", 
            "<html><body>"
            "<h1>ESP32 KNX Thermostat Firmware Update</h1>"
            "<form method='POST' action='/doUpdate' enctype='multipart/form-data'>"
            "<input type='file' name='update'><br><br>"
            "<input type='submit' value='Update Firmware'>"
            "</form>"
            "</body></html>");
    });
    
    // Handler for the actual update
    _server->on("/doUpdate", HTTP_POST, 
        [](AsyncWebServerRequest *request) {
            bool shouldReboot = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", 
                shouldReboot ? "Update successful! Rebooting..." : "Update failed!");
            response->addHeader("Connection", "close");
            request->send(response);
            if (shouldReboot) {
                delay(1000);
                ESP.restart();
            }
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                Serial.printf("OTA: Update start: %s\n", filename.c_str());
                // Calculate space required
                int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
                
                if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
                    Serial.println("OTA: Failed to begin update");
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA could not begin");
                }
            }
            
            // Write data
            if (Update.write(data, len) != len) {
                Serial.println("OTA: Failed to write update");
                Update.printError(Serial);
                return request->send(400, "text/plain", "OTA could not proceed");
            }
            
            // If final, end the update
            if (final) {
                if (!Update.end(true)) {
                    Serial.println("OTA: Failed to complete update");
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA failed to complete");
                }
                Serial.println("OTA: Update complete");
            }
        }
    );
    
    Serial.println("OTA Manager: Handlers registered");
}