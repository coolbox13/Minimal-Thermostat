#ifndef ELEGANT_OTA_ASYNC_H
#define ELEGANT_OTA_ASYNC_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <esp_log.h>

static const char* ELEGANT_OTA_TAG = "ElegantOTA";

class AsyncElegantOtaClass {
public:
    void begin(AsyncWebServer *server, const char *username = "", const char *password = "") {
        _server = server;
        _username = username;
        _password = password;

        _server->on("/update", HTTP_GET, [&](AsyncWebServerRequest *request) {
            if (_username != "" && _password != "") {
                if (!request->authenticate(_username, _password)) {
                    return request->requestAuthentication();
                }
            }
            AsyncWebServerResponse *response = request->beginResponse(200, "text/html", 
                "<form method='POST' action='/update' enctype='multipart/form-data'>"
                "<input type='file' name='update'>"
                "<input type='submit' value='Update'>"
                "</form>");
            request->send(response);
        });

        _server->on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
            if (_username != "" && _password != "") {
                if (!request->authenticate(_username, _password)) {
                    return request->requestAuthentication();
                }
            }
            // the request handler is triggered after the upload has finished... 
            AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 500 : 200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
            response->addHeader("Connection", "close");
            request->send(response);
            delay(1000);
            ESP.restart();
        }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            //Upload handler chunks in data
            if (_username != "" && _password != "") {
                if (!request->authenticate(_username, _password)) {
                    return request->requestAuthentication();
                }
            }

            if (!index) {
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    ESP_LOGE(ELEGANT_OTA_TAG, "Update begin failed");
                }
            }

            if (Update.write(data, len) != len) {
                ESP_LOGE(ELEGANT_OTA_TAG, "Update write failed");
            }

            if (final) {
                if (!Update.end(true)) {
                    ESP_LOGE(ELEGANT_OTA_TAG, "Update end failed");
                }
            }
        });
    }

private:
    AsyncWebServer *_server;
    const char *_username;
    const char *_password;
};

extern AsyncElegantOtaClass AsyncElegantOta;

#endif // ELEGANT_OTA_ASYNC_H