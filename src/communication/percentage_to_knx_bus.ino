// Optimized and standardized version for KNX communication compatible with ESP8266 and ESP32

#include <esp-knx-ip.h>
#include <WiFiManager.h> // Include the WiFiManager library

#ifdef ESP8266
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#endif

// Constants
const uint32_t SERIAL_BAUD_RATE = 115200;        // Serial communication baud rate
const uint32_t LOOP_DELAY = 10000;               // Delay in ms for the main loop

// Global variables
address_t ph_addr = {};                          // Physical address for KNX
uint8_t counter = 40;                            // Initial value for dummy percentage

// Function to set the KNX physical address
void setKNXPhysicalAddress(int area, int line, int member) {
    address_t receiving_pa;
    receiving_pa.pa.area = area;
    receiving_pa.pa.line = line;
    receiving_pa.pa.member = member;
    knx.physical_address_set(receiving_pa);      // Set the physical address for KNX
}

// Function to calculate the group address for KNX
address_t calculate_ga(int area, int line, int member) {
    address_t tmp;
    tmp.ga.area = area;
    tmp.ga.line = line;
    tmp.ga.member = member;
    return tmp;
}

// Setup function
void setup() {
    // Start serial communication
    Serial.begin(SERIAL_BAUD_RATE);

    // WiFiManager setup
    WiFiManager wifiManager;
    wifiManager.autoConnect("AutoConnectAP"); // Name of the Access Point when in configuration mode
    Serial.println("Connected to WiFi!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start KNX communication
    knx.start(nullptr);                          // Start KNX loop without webserver

    // Set KNX physical address
    setKNXPhysicalAddress(1, 1, 201);
}

// Main loop function
void loop() {
    knx.loop();                                  // Continue KNX communication loop

    // Send counter value to KNX address
    knx.write_1byte_uint(calculate_ga(3, 0, 15), counter);

    counter++;                                   // Increment counter

    delay(LOOP_DELAY);                           // Wait for the specified delay
}
