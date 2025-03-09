#include "web/web_interface.h"

void WebInterface::setupMDNS() {
  if (!MDNS.begin("thermostat")) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }
}

bool WebInterface::handleFileRead(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  String contentType = getContentType(path);
  
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    if (file) {
      server.streamFile(file, contentType);
      file.close();
      return true;
    }
  }
  return false;
}

bool WebInterface::isAuthenticated() {
  if (!configManager->getWebUsername() || !configManager->getWebPassword()) {
    return true;
  }
  if (!server.authenticate(configManager->getWebUsername(), configManager->getWebPassword())) {
    return false;
  }
  return true;
}

void WebInterface::requestAuthentication() {
  server.requestAuthentication();
}

void WebInterface::addSecurityHeaders() {
  server.sendHeader("X-Content-Type-Options", "nosniff");
  server.sendHeader("X-XSS-Protection", "1; mode=block");
  server.sendHeader("X-Frame-Options", "DENY");
  server.sendHeader("Content-Security-Policy", "default-src 'self'");
}

bool WebInterface::validateCSRFToken() {
  if (!server.hasHeader("X-CSRF-Token")) {
    return false;
  }
  String token = server.header("X-CSRF-Token");
  return token == generateCSRFToken();
}

String WebInterface::generateCSRFToken() {
  return authManager->generateCSRFToken();
}

String WebInterface::getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
