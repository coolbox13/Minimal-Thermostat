#ifndef WIFI_MANAGER_FIX_H
#define WIFI_MANAGER_FIX_H

// This file is a workaround for the WiFiManager issue with WebServer

#include <Arduino.h>

// Include the appropriate WebServer library
#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
  #include <DNSServer.h>
  
  // Define WebServer for ESP32
  typedef WebServer WM_WebServer;
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <DNSServer.h>
  
  // Define WebServer for ESP8266
  typedef ESP8266WebServer WM_WebServer;
#endif

// Now include WiFiManager
#include <WiFiManager.h>

#endif // WIFI_MANAGER_FIX_H 