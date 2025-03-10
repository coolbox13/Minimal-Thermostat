#ifndef KNX_INTERFACE_FIX_H
#define KNX_INTERFACE_FIX_H

#include <Arduino.h>

// Forward declaration of AsyncWebServer to avoid compilation errors
// This is a workaround for the esp-knx-ip library which expects AsyncWebServer
#if !defined(AsyncWebServer)
class AsyncWebServer;
#endif

// Now include the KNX library
#include <esp-knx-ip.h>

#endif // KNX_INTERFACE_FIX_H 