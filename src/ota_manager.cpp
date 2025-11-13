#include "ota_manager.h"
#include "serial_monitor.h"
#include "serial_redirect.h"

// Redirect Serial to CapturedSerial for web monitor
#define Serial CapturedSerial

OTAManager::OTAManager() {
}

void OTAManager::begin(WebServerManager* webServerManager) {
    if (!webServerManager) {
        Serial.println("OTA Manager: No web server manager provided");
        return;
    }
    
    // Add handler for firmware update page
    webServerManager->addEndpoint("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", 
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "<head>"
            "    <meta charset=\"UTF-8\">"
            "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "    <title>ESP32 KNX Thermostat - Firmware Update</title>"
            "    <link rel=\"stylesheet\" href=\"style.css\">"
            "    <style>"
            "        .update-container {"
            "            max-width: 600px;"
            "            margin: 0 auto;"
            "        }"
            "        .file-input-container {"
            "            margin: 20px 0;"
            "        }"
            "        progress {"
            "            width: 100%;"
            "            height: 20px;"
            "            margin: 15px 0;"
            "        }"
            "        #update-status {"
            "            margin-top: 20px;"
            "            padding: 10px;"
            "            border-radius: 4px;"
            "            text-align: center;"
            "        }"
            "        .hidden {"
            "            display: none;"
            "        }"
            "        .success {"
            "            background-color: #d4edda;"
            "            color: #155724;"
            "        }"
            "        .error {"
            "            background-color: #f8d7da;"
            "            color: #721c24;"
            "        }"
            "    </style>"
            "</head>"
            "<body>"
            "    <header>"
            "        <h1>ESP32 KNX Thermostat - Firmware Update</h1>"
            "    </header>"
            "    "
            "    <div class=\"container update-container\">"
            "        <div class=\"card\">"
            "            <h2 class=\"card-title\">Upload New Firmware</h2>"
            "            "
            "            <form id=\"update-form\" method=\"POST\" action=\"/doUpdate\" enctype=\"multipart/form-data\">"
            "                <div class=\"file-input-container\">"
            "                    <p>Select the firmware file (.bin) to upload:</p>"
            "                    <input type=\"file\" name=\"update\" id=\"firmware-file\" accept=\".bin\">"
            "                </div>"
            "                "
            "                <progress id=\"upload-progress\" class=\"hidden\" value=\"0\" max=\"100\"></progress>"
            "                "
            "                <div class=\"control-row\">"
            "                    <button type=\"submit\" id=\"update-button\">Update Firmware</button>"
            "                    <a href=\"/\"><button type=\"button\">Back to Dashboard</button></a>"
            "                </div>"
            "            </form>"
            "            "
            "            <div id=\"update-status\" class=\"hidden\"></div>"
            "        </div>"
            "    </div>"
            "    "
            "    <script>"
            "        document.addEventListener('DOMContentLoaded', function() {"
            "            const form = document.getElementById('update-form');"
            "            const fileInput = document.getElementById('firmware-file');"
            "            const progress = document.getElementById('upload-progress');"
            "            const status = document.getElementById('update-status');"
            "            const updateButton = document.getElementById('update-button');"
            "            "
            "            fileInput.addEventListener('change', function() {"
            "                if (this.files.length > 0) {"
            "                    status.textContent = 'File selected: ' + this.files[0].name;"
            "                    status.className = '';"
            "                    status.classList.remove('hidden');"
            "                }"
            "            });"
            "            "
            "            form.addEventListener('submit', function(e) {"
            "                e.preventDefault();"
            "                "
            "                if (fileInput.files.length === 0) {"
            "                    status.textContent = 'Please select a firmware file';"
            "                    status.className = 'error';"
            "                    status.classList.remove('hidden');"
            "                    return;"
            "                }"
            "                "
            "                const file = fileInput.files[0];"
            "                const xhr = new XMLHttpRequest();"
            "                const formData = new FormData();"
            "                "
            "                formData.append('update', file);"
            "                "
            "                xhr.open('POST', '/doUpdate', true);"
            "                "
            "                xhr.upload.addEventListener('progress', function(e) {"
            "                    if (e.lengthComputable) {"
            "                        const percentComplete = (e.loaded / e.total) * 100;"
            "                        progress.value = percentComplete;"
            "                        progress.classList.remove('hidden');"
            "                        status.textContent = 'Uploading: ' + Math.round(percentComplete) + '%';"
            "                        status.className = '';"
            "                        status.classList.remove('hidden');"
            "                    }"
            "                });"
            "                "
            "                xhr.addEventListener('load', function() {"
            "                    if (xhr.status === 200) {"
            "                        status.textContent = 'Update successful! Device is rebooting...';"
            "                        status.className = 'success';"
            "                        updateButton.disabled = true;"
            "                        "
            "                        // Attempt to reconnect after 30 seconds"
            "                        setTimeout(function() {"
            "                            status.textContent = 'Attempting to reconnect...';"
            "                            checkConnection();"
            "                        }, 30000);"
            "                    } else {"
            "                        status.textContent = 'Update failed: ' + xhr.responseText;"
            "                        status.className = 'error';"
            "                    }"
            "                });"
            "                "
            "                xhr.addEventListener('error', function() {"
            "                    status.textContent = 'Error uploading the firmware';"
            "                    status.className = 'error';"
            "                });"
            "                "
            "                xhr.send(formData);"
            "                status.textContent = 'Starting upload...';"
            "                status.className = '';"
            "                status.classList.remove('hidden');"
            "            });"
            "            "
            "            function checkConnection() {"
            "                fetch('/')"
            "                    .then(response => {"
            "                        if (response.ok) {"
            "                            status.textContent = 'Device rebooted successfully!';"
            "                            status.className = 'success';"
            "                            setTimeout(function() {"
            "                                window.location.href = '/';"
            "                            }, 2000);"
            "                        } else {"
            "                            retryConnection();"
            "                        }"
            "                    })"
            "                    .catch(() => {"
            "                        retryConnection();"
            "                    });"
            "            }"
            "            "
            "            function retryConnection() {"
            "                status.textContent = 'Still rebooting, retrying in 5 seconds...';"
            "                setTimeout(checkConnection, 5000);"
            "            }"
            "        });"
            "    </script>"
            "</body>"
            "</html>");
    });
    
    // Handler for the actual update
    webServerManager->addEndpoint("/doUpdate", HTTP_POST, 
        [](AsyncWebServerRequest *request) {
            bool shouldReboot = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", 
                shouldReboot ? "Update successful! Rebooting..." : "Update failed: " + String(Update.errorString()));
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
                    return request->send(400, "text/plain", "OTA could not begin: " + String(Update.errorString()));
                }
            }
            
            // Write data
            if (Update.write(data, len) != len) {
                Serial.println("OTA: Failed to write update");
                Update.printError(Serial);
                return request->send(400, "text/plain", "OTA could not proceed: " + String(Update.errorString()));
            }
            
            // If final, end the update
            if (final) {
                if (!Update.end(true)) {
                    Serial.println("OTA: Failed to complete update");
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA failed to complete: " + String(Update.errorString()));
                }
                Serial.println("OTA: Update complete");
            }
        }
    );
    
    Serial.println("OTA Manager: Handlers registered");
}