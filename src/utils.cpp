#include "utils.h"

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
  static String buffer = "";
  static bool messageStarted = false;
  static bool messageComplete = false;
  
  // Process characters from Serial
  while (Serial.available()) {
    char c = Serial.read();
    
    // Look for the start of a KNX debug message
    if (c == '[' && !messageStarted) {
      buffer = c;
      messageStarted = true;
    } 
    else if (messageStarted) {
      buffer += c;
      
      // Check for end of KNX message block
      if (buffer.indexOf("[KNXIP] ==") > 0) {
        messageComplete = true;
      }
    }
    
    // Process complete message
    if (messageComplete) {
      decodeRawKnxDebugMessage(buffer);
      buffer = "";
      messageStarted = false;
      messageComplete = false;
    }
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