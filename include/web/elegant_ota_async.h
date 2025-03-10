#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <esp_log.h>

class AsyncElegantOtaClass {
public:
    void begin(AsyncWebServer *server) {
        server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/html", 
                "<form method='POST' action='/update' enctype='multipart/form-data'>"
                "<input type='file' name='update'>"
                "<input type='submit' value='Update'>"
                "</form>");
            request->send(response);
        });

        server->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
            bool shouldReboot = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
            response->addHeader("Connection", "close");
            request->send(response);
            if (shouldReboot) {
                delay(100);
                ESP.restart();
            }
        }, [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    log_e("Update error: %s", Update.errorString());
                }
                log_i("Update Start: %s", filename.c_str());
            }

            if (Update.write(data, len) != len) {
                log_e("Update error: %s", Update.errorString());
            }

            if (final) {
                if (Update.end(true)) {
                    log_i("Update Success: %uB", index + len);
                } else {
                    log_e("Update error: %s", Update.errorString());
                }
            }
        });
    }
};

extern AsyncElegantOtaClass AsyncElegantOta;