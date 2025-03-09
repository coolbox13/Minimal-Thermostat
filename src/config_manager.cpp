bool ConfigManager::setupWiFi() {
    AsyncWebServer server(80);
    DNSServer dns;
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.setConfigPortalTimeout(180); // Auto-exit config portal after 3 minutes
    
    // Try to connect to WiFi
    if (!wifiManager.autoConnect(deviceName)) {
        Serial.println("Failed to connect to WiFi - restarting");
        delay(1000);
        ESP.restart();
        return false;
    }
    
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    return true;
} 