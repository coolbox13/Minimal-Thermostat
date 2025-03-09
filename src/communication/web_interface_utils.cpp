#include "web_interface.h"

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
  
  String contentType;
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";
  else contentType = "text/plain";
  
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

bool WebInterface::isAuthenticated() {
  if (configManager && configManager->getWebUsername()[0] != '\0') {
    return server.authenticate(configManager->getWebUsername(), 
                             configManager->getWebPassword());
  }
  return true; // No authentication required if no credentials set
}

void WebInterface::requestAuthentication() {
  server.requestAuthentication();
}
