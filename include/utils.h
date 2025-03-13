#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <esp-knx-ip.h>

// Function to decode KNX command type
String decodeKnxCommandType(uint8_t ct);

// Function to decode KNX address (physical or group)
String decodeKnxAddress(uint16_t addr, bool isGroupAddress);

// Function to decode KNX data based on command type and data length
String decodeKnxData(uint8_t ct, uint8_t* data, uint8_t len);

// Main function to decode a KNX message into a readable string
String decodeKnxMessage(knx_command_type_t ct, uint16_t src, uint16_t dst, uint8_t* data, uint8_t len);

// New functions to monitor and decode KNX debug messages from serial output
void monitorKnxDebugMessages();
void decodeRawKnxDebugMessage(String &message);

#endif // UTILS_H