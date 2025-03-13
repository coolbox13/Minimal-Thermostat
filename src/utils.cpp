#include "utils.h"
#include "config.h"
#include <esp_log.h>

String decodeKnxCommandType(uint8_t ct) {
  switch (ct) {
    case KNX_CT_READ:
      return "Read";
    case KNX_CT_WRITE:
      return "Write";
    case KNX_CT_ANSWER:  // Using KNX_CT_ANSWER instead of KNX_CT_RESPONSE
      return "Response";
    default:
      return "Unknown (" + String(ct, HEX) + ")";
  }
}

String decodeKnxAddress(uint16_t addr, bool isGroupAddress) {
  address_t address;
  address.value = addr;
  
  if (isGroupAddress) {
    // Group address format: main/middle/sub
    return String(address.ga.area) + "/" + 
           String(address.ga.line) + "/" + 
           String(address.ga.member);
  } else {
    // Physical address format: area.line.member
    return String(address.pa.area) + "." + 
           String(address.pa.line) + "." + 
           String(address.pa.member);
  }
}

String decodeKnxData(uint8_t ct, uint8_t* data, uint8_t len) {
  if (len == 0) {
    return "No data";
  }
  
  // For simple 1-byte values (common case)
  if (len == 1) {
    uint8_t value = data[0];
    
    // Check if it's a boolean value (common for switches, etc.)
    if ((value & 0x3F) == 0) {
      return (value & 0x80) ? "ON" : "OFF";
    }
    
    // For scaling values (0-100%)
    if (value <= 100) {
      return String(value) + "%";
    }
    
    // Default to hex representation
    return "0x" + String(value, HEX);
  }
  
  // For 2-byte float values (DPT 9.xxx)
  if (len == 2) {
    // Extract the float value from 2-byte KNX format
    int16_t mantissa = (data[1] & 0x07FF);
    if (mantissa & 0x0400) {
      mantissa |= 0xF800; // Extend sign bit
    }
    
    int8_t exponent = (data[1] >> 3) & 0x0F;
    if (exponent & 0x08) {
      exponent |= 0xF0; // Extend sign bit
    }
    
    float value = mantissa * pow(2, exponent);
    return String(value, 2);
  }
  
  // For other multi-byte values, return hex representation
  String hexData = "";
  for (int i = 0; i < len; i++) {
    if (i > 0) hexData += " ";
    if (data[i] < 0x10) hexData += "0";
    hexData += String(data[i], HEX);
  }
  return "0x" + hexData;
}

// This function is not used in the current implementation but kept for future use
String decodeKnxMessage(knx_command_type_t ct, uint16_t src, uint16_t dst, uint8_t* data, uint8_t len) {
  String message = "";
  
  // Command type
  message += "Cmd: " + decodeKnxCommandType(ct);
  
  // Destination address (group address for most commands)
  message += " To: " + decodeKnxAddress(dst, true);
  
  // Data
  message += " Data: " + decodeKnxData(ct, data, len);
  
  return message;
}

// New function to decode the raw KNX debug messages from the serial output
void monitorKnxDebugMessages() {
    // Check if there's data available on Serial
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        
        // Skip KNX debug messages that contain specific patterns
        if (line.indexOf("[KNXIP]") >= 0 && 
            (line.indexOf("__loop_knx()") >= 0 || 
             line.indexOf("Got packet") >= 0 ||
             line.indexOf("LEN:") >= 0)) {
            // Skip these common debug messages
            continue;
        }
        
        // Process other messages as needed
        // ...
    }
}

// Function to decode the raw KNX debug message
void decodeRawKnxDebugMessage(String &message) {
  // Extract key information from the debug message
  String sourceAddr = "";
  String destAddr = "";
  String commandType = "";
  String data = "";
  
  // Extract source address
  int sourceIndex = message.indexOf("Source:");
  if (sourceIndex > 0) {
    sourceAddr = message.substring(sourceIndex + 8, message.indexOf('\n', sourceIndex));
    sourceAddr.trim();
  }
  
  // Extract destination address
  int destIndex = message.indexOf("Dest:");
  if (destIndex > 0) {
    destAddr = message.substring(destIndex + 6, message.indexOf('\n', destIndex));
    destAddr.trim();
  }
  
  // Extract command type
  int ctIndex = message.indexOf("CT:");
  if (ctIndex > 0) {
    commandType = message.substring(ctIndex + 4, message.indexOf('\n', ctIndex));
    commandType.trim();
    
    // Convert hex command type to readable format
    if (commandType == "0x00") {
      commandType = "Read";
    } else if (commandType == "0x01") {
      commandType = "Response";
    } else if (commandType == "0x02") {
      commandType = "Write";
    }
  }
  
  // Extract data bytes
  int dataStartIndex = message.indexOf("[KNXIP]  0x", ctIndex);
  if (dataStartIndex > 0) {
    // Collect all data bytes
    String dataBytes = "";
    int currentIndex = dataStartIndex;
    
    while (true) {
      int nextDataIndex = message.indexOf("[KNXIP]  0x", currentIndex + 1);
      if (nextDataIndex < 0 || message.indexOf("[KNXIP] ==", currentIndex) < nextDataIndex) {
        // End of data section
        String dataPart = message.substring(currentIndex + 10, message.indexOf('\n', currentIndex));
        dataPart.trim();
        dataBytes += dataPart + " ";
        break;
      }
      
      String dataPart = message.substring(currentIndex + 10, message.indexOf('\n', currentIndex));
      dataPart.trim();
      dataBytes += dataPart + " ";
      currentIndex = nextDataIndex;
    }
    
    data = dataBytes;
  }
  
  // Convert source and destination to readable format
  String readableSource = "";
  if (sourceAddr.length() > 0) {
    // Format: "0x11 0x4a" -> "1.1.74"
    int high = strtol(sourceAddr.substring(0, 4).c_str(), NULL, 16);
    int low = strtol(sourceAddr.substring(5, 9).c_str(), NULL, 16);
    
    int area = (high >> 4) & 0x0F;
    int line = high & 0x0F;
    int member = low;
    
    readableSource = String(area) + "." + String(line) + "." + String(member);
  }
  
  String readableDest = "";
  if (destAddr.length() > 0) {
    // Format: "0x55 0x00" -> "5/5/0"
    int high = strtol(destAddr.substring(0, 4).c_str(), NULL, 16);
    int low = strtol(destAddr.substring(5, 9).c_str(), NULL, 16);
    
    int main = (high >> 3) & 0x1F;
    int middle = high & 0x07;
    int sub = low;
    
    readableDest = String(main) + "/" + String(middle) + "/" + String(sub);
  }
  
  // Print decoded message
  if (sourceAddr.length() > 0 && destAddr.length() > 0) {
    Serial.print("KNX Decoded: From ");
    Serial.print(readableSource);
    Serial.print(" To ");
    Serial.print(readableDest);
    Serial.print(" - ");
    Serial.print(commandType);
    
    if (data.length() > 0) {
      Serial.print(" Data: ");
      Serial.print(data);
      
      // Try to interpret the data based on common DPT types
      if (data.indexOf("0x80") >= 0) {
        Serial.print(" (OFF)");
      } else if (data.indexOf("0x81") >= 0) {
        Serial.print(" (ON)");
      }
    }
    
    Serial.println();
  }
}

// Buffer to store the last KNX message
static char lastKnxMessage[256] = "";
static unsigned long lastKnxMessageTime = 0;
static int knxMessageRepeatCount = 0;

// Custom log function that will be called by ESP-IDF logging
// Changed return type from void to int to match vprintf_like_t signature
static int customLogOutput(const char *fmt, va_list args) {
    char buffer[256];
    int result = vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    // Check if this is a KNX message
    if (strstr(buffer, "[KNXIP]") != NULL) {
        processKnxDebugMessage(buffer);
    } else {
        // For non-KNX messages, print directly
        Serial.print(buffer);
    }
    
    // Return the number of characters that would have been written
    return result;
}

void setupCustomLogHandler() {
    // Set our custom log handler
    esp_log_set_vprintf(customLogOutput);
    
    // Set the log level for KNX based on configuration
    if (KNX_DEBUG_ENABLED) {
        esp_log_level_set("KNXIP", ESP_LOG_INFO);
    } else {
        esp_log_level_set("KNXIP", ESP_LOG_ERROR);
    }
}

void processKnxDebugMessage(const char* message) {
    // Check if this is a message we want to process
    if (strstr(message, "__loop_knx()") != NULL) {
        // Compare with last message to detect repeats
        if (strcmp(message, lastKnxMessage) == 0) {
            knxMessageRepeatCount++;
            
            // Only print every 50th repeat or if more than 5 seconds passed
            unsigned long now = millis();
            if (knxMessageRepeatCount % 50 == 0 || now - lastKnxMessageTime > 5000) {
                Serial.print("KNX debug (repeated ");
                Serial.print(knxMessageRepeatCount);
                Serial.print(" times): ");
                
                // Extract the important part of the message
                const char* important = strstr(message, "[KNXIP]");
                if (important) {
                    Serial.println(important);
                } else {
                    Serial.println(message);
                }
                
                lastKnxMessageTime = now;
            }
        } else {
            // New message
            if (knxMessageRepeatCount > 1) {
                Serial.print("Previous KNX debug repeated ");
                Serial.print(knxMessageRepeatCount);
                Serial.println(" times");
            }
            
            // Extract and print only the important part
            const char* important = strstr(message, "[KNXIP]");
            if (important) {
                Serial.print("KNX debug: ");
                Serial.println(important);
            } else {
                Serial.print("KNX debug: ");
                Serial.println(message);
            }
            
            // Store for comparison
            strncpy(lastKnxMessage, message, sizeof(lastKnxMessage) - 1);
            lastKnxMessage[sizeof(lastKnxMessage) - 1] = '\0';
            knxMessageRepeatCount = 1;
            lastKnxMessageTime = millis();
        }
    } else if (strstr(message, "Got packet") != NULL || 
               strstr(message, "LEN:") != NULL) {
        // These are very common messages, just count them without printing
        static int packetCount = 0;
        packetCount++;
        
        // Print a summary every 100 packets
        if (packetCount % 100 == 0) {
            Serial.print("KNX packets processed: ");
            Serial.println(packetCount);
        }
    } else {
        // For other KNX messages, print them directly but with a prefix
        Serial.print("KNX: ");
        Serial.println(message);
    }
}