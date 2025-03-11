#ifndef SKIP_WEB_INTERFACE_EXTRA
// implementation here
#endif

#include "web/web_interface.h"
#include "esp_log.h"
#include <ESPmDNS.h>

static const char* TAG = "WebInterface";

void WebInterface::setupMDNS() {
  if (!MDNS.begin("thermostat")) {
    ESP_LOGE(TAG, "Error setting up MDNS responder!");
  } else {
    ESP_LOGI(TAG, "mDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }
}

bool WebInterface::handleFileRead(AsyncWebServerRequest *request, String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  String contentType = getContentType(path);
  
  if (LittleFS.exists(path)) {
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, contentType);
    addSecurityHeaders(response);
    request->send(response);
    ESP_LOGD(TAG, "Serving file: %s", path.c_str());
    return true;
  }
  ESP_LOGW(TAG, "File not found: %s", path.c_str());
  return false;
}

bool WebInterface::isAuthenticated(AsyncWebServerRequest *request) {
  if (configManager->getWebUsername()[0] == '\0') {
    return true;
  }

  if (!request->authenticate(configManager->getWebUsername(), configManager->getWebPassword())) {
    ESP_LOGW(TAG, "Authentication failed for IP: %s", request->client()->remoteIP().toString().c_str());
    return false;
  }

  return true;
}

void WebInterface::requestAuthentication(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(401);
  response->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
  request->send(response);
  ESP_LOGI(TAG, "Requesting authentication from IP: %s", request->client()->remoteIP().toString().c_str());
}

void WebInterface::addSecurityHeaders(AsyncWebServerResponse *response) {
  response->addHeader("X-Content-Type-Options", "nosniff");
  response->addHeader("X-Frame-Options", "DENY");
  response->addHeader("X-XSS-Protection", "1; mode=block");
  response->addHeader("Strict-Transport-Security", "max-age=31536000; includeSubDomains");
  response->addHeader("Content-Security-Policy", "default-src 'self'");
  response->addHeader("Referrer-Policy", "same-origin");
}

bool WebInterface::validateCSRFToken(AsyncWebServerRequest *request) {
  if (!request->hasHeader("X-CSRF-Token")) {
    ESP_LOGW(TAG, "Missing CSRF token from IP: %s", request->client()->remoteIP().toString().c_str());
    return false;
  }

  String token = request->header("X-CSRF-Token");
  // TODO: Implement proper CSRF token validation
  return true;
}

String WebInterface::generateCSRFToken(AsyncWebServerRequest *request) {
  // TODO: Implement proper CSRF token generation
  return String(random(0xFFFFFFFF));
}

String WebInterface::getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
