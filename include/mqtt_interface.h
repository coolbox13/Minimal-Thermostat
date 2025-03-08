/**
 * esp-knx-ip library for KNX/IP communication on an ESP32
 * Ported from ESP8266 version
 * Author: Nico Weichbrodt <envy> (Original), Modified for ESP32
 * License: MIT
 */

 #include "esp-knx-ip.h"
 #include <WiFi.h>
 #include <WebServer.h>
 #include <Preferences.h>
 #include <esp_log.h>
 #include <esp_wifi.h>
 
 #define DEBUG_TAG "KNXIP"
 #define DEBUG_PRINT(fmt, ...) ESP_LOGD(DEBUG_TAG, fmt, ##__VA_ARGS__)
 #define DEBUG_PRINTLN(fmt, ...) ESP_LOGD(DEBUG_TAG, fmt "\n", ##__VA_ARGS__)
 
 ESPKNXIP knx;
 
 ESPKNXIP::ESPKNXIP() : server(nullptr),
                       registered_callback_assignments(0),
                       registered_callbacks(0),
                       registered_configs(0),
                       registered_feedbacks(0)
 {
   DEBUG_PRINTLN("ESPKNXIP starting up");
   // Default physical address is 1.1.0
   physaddr.bytes.high = (1 << 4) | 1;
   physaddr.bytes.low = 0;
   memset(callback_assignments, 0, MAX_CALLBACK_ASSIGNMENTS * sizeof(callback_assignment_t));
   memset(callbacks, 0, MAX_CALLBACKS * sizeof(callback_t));
   memset(custom_config_data, 0, MAX_CONFIG_SPACE * sizeof(uint8_t));
   memset(custom_config_default_data, 0, MAX_CONFIG_SPACE * sizeof(uint8_t));
   memset(custom_configs, 0, MAX_CONFIGS * sizeof(config_t));
 }
 
 void ESPKNXIP::load()
 {
   memcpy(custom_config_default_data, custom_config_data, MAX_CONFIG_SPACE);
   restore_from_preferences();
 }
 
 void ESPKNXIP::start(WebServer *srv)
 {
   server = srv;
   if (server == nullptr) {
     ESP_LOGW(DEBUG_TAG, "No WebServer provided - web interface disabled");
   }
   __start();
 }
 
 void ESPKNXIP::start()
 {
   // Don't automatically create a server, just start without web interface
   ESP_LOGW(DEBUG_TAG, "Starting without WebServer - web interface disabled");
   server = nullptr;
   __start();
 }
 
 void ESPKNXIP::__start()
 {
   // Disable WiFi sleep mode for better UDP multicast performance
   WiFi.setSleep(false);
   esp_wifi_set_ps(WIFI_PS_NONE);
 
   if (server != nullptr) {
     try {
       // Register handlers
       server->on(ROOT_PREFIX, HTTP_GET, [this](){ __handle_root(); });
       server->on(__ROOT_PATH, HTTP_GET, [this](){ __handle_root(); });
       server->on(__REGISTER_PATH, HTTP_POST, [this](){ __handle_register(); });
       server->on(__DELETE_PATH, HTTP_POST, [this](){ __handle_delete(); });
       server->on(__PHYS_PATH, HTTP_POST, [this](){ __handle_set(); });
 #if !DISABLE_EEPROM_BUTTONS
       server->on(__EEPROM_PATH, HTTP_POST, [this](){ __handle_eeprom(); });
 #endif
       server->on(__CONFIG_PATH, HTTP_POST, [this](){ __handle_config(); });
       server->on(__FEEDBACK_PATH, HTTP_POST, [this](){ __handle_feedback(); });
 #if !DISABLE_RESTORE_BUTTON
       server->on(__RESTORE_PATH, HTTP_POST, [this](){ __handle_restore(); });
 #endif
 #if !DISABLE_REBOOT_BUTTON
       server->on(__REBOOT_PATH, HTTP_POST, [this](){ __handle_reboot(); });
 #endif
       
       // Start server
       server->begin();
       ESP_LOGI(DEBUG_TAG, "WebServer started successfully");
     } catch (const std::exception& e) {
       ESP_LOGE(DEBUG_TAG, "Error starting WebServer: %s", e.what());
       server = nullptr; // Clear the server reference to prevent further attempts
     } catch (...) {
       ESP_LOGE(DEBUG_TAG, "Unknown error starting WebServer");
       server = nullptr; // Clear the server reference to prevent further attempts
     }
   }
   
   // Start UDP multicast properly for ESP32
   IPAddress localIP = WiFi.localIP();
   if (!udp.beginMulticast(localIP, MULTICAST_IP, MULTICAST_PORT)) {
     ESP_LOGE(DEBUG_TAG, "Failed to start KNX/IP UDP multicast");
   } else {
     ESP_LOGI(DEBUG_TAG, "KNX/IP UDP multicast started successfully on %s", localIP.toString().c_str());
   }
 }
 
 void ESPKNXIP::save_to_preferences()
 {
   prefs.begin("KNX", false);
   uint64_t magic = EEPROM_MAGIC;
   prefs.putBytes("magic", &magic, sizeof(magic));
   prefs.putUChar("reg_cb_assign", registered_callback_assignments);
   prefs.putBytes("cb_assign", callback_assignments, MAX_CALLBACK_ASSIGNMENTS * sizeof(callback_assignment_t));
   prefs.putBytes("physaddr", &physaddr, sizeof(address_t));
   prefs.putBytes("config", custom_config_data, sizeof(custom_config_data));
   prefs.end();
   DEBUG_PRINT("Saved to Preferences");
 }
 
 void ESPKNXIP::restore_from_preferences()
 {
   prefs.begin("KNX", true);
   uint64_t magic = 0;
   size_t len = sizeof(magic);
   prefs.getBytes("magic", &magic, len);
   if (magic != EEPROM_MAGIC)
   {
     DEBUG_PRINTLN("No valid magic in Preferences, aborting restore.");
     prefs.end();
     return;
   }
   registered_callback_assignments = prefs.getUChar("reg_cb_assign", 0);
   prefs.getBytes("cb_assign", callback_assignments, MAX_CALLBACK_ASSIGNMENTS * sizeof(callback_assignment_t));
   prefs.getBytes("physaddr", &physaddr, sizeof(address_t));
   prefs.getBytes("config", custom_config_data, sizeof(custom_config_data));
   prefs.end();
   DEBUG_PRINT("Restored from Preferences");
 }
 
 uint16_t ESPKNXIP::__ntohs(uint16_t n)
 {
   return (uint16_t)((((uint8_t*)&n)[0] << 8) | (((uint8_t*)&n)[1]));
 }
 
 callback_assignment_id_t ESPKNXIP::__callback_register_assignment(address_t address, callback_id_t id)
 {
   if (registered_callback_assignments >= MAX_CALLBACK_ASSIGNMENTS)
     return -1;
 
   callback_assignment_id_t aid = registered_callback_assignments;
   callback_assignments[aid].address = address;
   callback_assignments[aid].callback_id = id;
   registered_callback_assignments++;
   return aid;
 }
 
 void ESPKNXIP::__callback_delete_assignment(callback_assignment_id_t id)
 {
   if (id >= registered_callback_assignments)
     return;
 
   uint32_t dest_offset = 0;
   uint32_t src_offset = 0;
   uint32_t len = 0;
   if (id == 0)
   {
     src_offset = 1;
     len = (registered_callback_assignments - 1);
   }
   else if (id == registered_callback_assignments - 1)
   {
     // Last element; nothing to move.
   }
   else
   {
     dest_offset = id;
     src_offset = dest_offset + 1;
     len = (registered_callback_assignments - 1 - id);
   }
   if (len > 0)
   {
     memmove(callback_assignments + dest_offset, callback_assignments + src_offset, len * sizeof(callback_assignment_t));
   }
   registered_callback_assignments--;
 }
 
 callback_id_t ESPKNXIP::callback_register(String name, callback_fptr_t cb, void *arg, enable_condition_t cond)
 {
   if (registered_callbacks >= MAX_CALLBACKS)
     return -1;
 
   callback_id_t id = registered_callbacks;
   callbacks[id].name = name;
   callbacks[id].fkt = cb;
   callbacks[id].cond = cond;
   callbacks[id].arg = arg;
   registered_callbacks++;
   return id;
 }
 
 void ESPKNXIP::callback_assign(callback_id_t id, address_t val)
 {
   if (id >= registered_callbacks)
     return;
   __callback_register_assignment(val, id);
 }
 
 /* Feedback Functions */
 
 feedback_id_t ESPKNXIP::feedback_register_int(String name, int32_t *value, enable_condition_t cond)
 {
   if (registered_feedbacks >= MAX_FEEDBACKS)
     return -1;
   feedback_id_t id = registered_feedbacks;
   feedbacks[id].type = FEEDBACK_TYPE_INT;
   feedbacks[id].name = name;
   feedbacks[id].cond = cond;
   feedbacks[id].data = (void *)value;
   registered_feedbacks++;
   return id;
 }
 
 feedback_id_t ESPKNXIP::feedback_register_float(String name, float *value, uint8_t precision, enable_condition_t cond)
 {
   if (registered_feedbacks >= MAX_FEEDBACKS)
     return -1;
   feedback_id_t id = registered_feedbacks;
   feedbacks[id].type = FEEDBACK_TYPE_FLOAT;
   feedbacks[id].name = name;
   feedbacks[id].cond = cond;
   feedbacks[id].data = (void *)value;
   feedbacks[id].options.float_options.precision = precision;
   registered_feedbacks++;
   return id;
 }
 
 feedback_id_t ESPKNXIP::feedback_register_bool(String name, bool *value, enable_condition_t cond)
 {
   if (registered_feedbacks >= MAX_FEEDBACKS)
     return -1;
   feedback_id_t id = registered_feedbacks;
   feedbacks[id].type = FEEDBACK_TYPE_BOOL;
   feedbacks[id].name = name;
   feedbacks[id].cond = cond;
   feedbacks[id].data = (void *)value;
   registered_feedbacks++;
   return id;
 }
 
 feedback_id_t ESPKNXIP::feedback_register_action(String name, feedback_action_fptr_t value, void *arg, enable_condition_t cond)
 {
   if (registered_feedbacks >= MAX_FEEDBACKS)
     return -1;
   feedback_id_t id = registered_feedbacks;
   feedbacks[id].type = FEEDBACK_TYPE_ACTION;
   feedbacks[id].name = name;
   feedbacks[id].cond = cond;
   feedbacks[id].data = (void *)value;
   feedbacks[id].options.action_options.arg = arg;
   registered_feedbacks++;
   return id;
 }
 
 void ESPKNXIP::loop()
 {
   __loop_knx();
   
   // Only call WebServer if it exists and is properly initialized
   if (server != nullptr) {
     // Wrap in try/catch to prevent crashes
     try {
       server->handleClient();
     } catch (...) {
       ESP_LOGE(DEBUG_TAG, "Error in WebServer handling");
     }
   }
 }
 
 void ESPKNXIP::__loop_webserver()
 {
   if (server != nullptr) {
     server->handleClient();
   }
 }
 
 void ESPKNXIP::__loop_knx()
 {
   int read = udp.parsePacket();
   if (!read)
     return;
   
   // Safety check for packet size
   if (read > 1024) {
     ESP_LOGE(DEBUG_TAG, "Received oversized packet (%d bytes), dropping", read);
     udp.flush();
     return;
   }
 
   uint8_t buf[1024]; // Pre-allocated fixed buffer with safe size
   int bytesRead = udp.read(buf, sizeof(buf));
   udp.flush();
 
   if (bytesRead <= 0) {
     ESP_LOGE(DEBUG_TAG, "Error reading UDP packet");
     return;
   }
 
   DEBUG_PRINT("Got KNX packet: %d bytes", bytesRead);
   
   #ifdef ESP_KNX_DEBUG_VERBOSE
   for (int i = 0; i < bytesRead; ++i) {
     DEBUG_PRINT(" 0x%02X", buf[i]);
     if ((i+1) % 16 == 0) DEBUG_PRINTLN("");
   }
   DEBUG_PRINTLN("");
   #endif
 
   // Check if packet is large enough to contain headers
   if (bytesRead < sizeof(knx_ip_pkt_t)) {
     ESP_LOGE(DEBUG_TAG, "Packet too small to be valid KNX packet");
     return;
   }
 
   knx_ip_pkt_t *knx_pkt = (knx_ip_pkt_t *)buf;
   DEBUG_PRINT("Service Type: 0x%04X", __ntohs(knx_pkt->service_type));
 
   // Packet validation checks
   if (knx_pkt->header_len != 0x06 || 
       knx_pkt->protocol_version != 0x10 || 
       __ntohs(knx_pkt->service_type) != KNX_ST_ROUTING_INDICATION) {
     DEBUG_PRINTLN("Invalid KNX packet format");
     return;
   }
 
   // Check if CEMI message fits within the received data
   size_t headerSize = sizeof(knx_ip_pkt_t);
   if (bytesRead <= headerSize) {
     ESP_LOGE(DEBUG_TAG, "Packet too small to contain CEMI data");
     return;
   }
 
   cemi_msg_t *cemi_msg = (cemi_msg_t *)knx_pkt->pkt_data;
   DEBUG_PRINT("Message Code: 0x%02X", cemi_msg->message_code);
   
   if (cemi_msg->message_code != KNX_MT_L_DATA_IND) {
     DEBUG_PRINTLN("Not a L_DATA_IND message, ignoring");
     return;
   }
 
   DEBUG_PRINT("Additional Info Length: 0x%02X", cemi_msg->additional_info_len);
   
   // Calculate pointer to service info considering additional info length
   cemi_service_t *cemi_data = &cemi_msg->data.service_information;
   if (cemi_msg->additional_info_len > 0) {
     // Make sure additional info doesn't exceed packet boundaries
     if (headerSize + sizeof(cemi_msg_t) + cemi_msg->additional_info_len > bytesRead) {
       ESP_LOGE(DEBUG_TAG, "Additional info exceeds packet size");
       return;
     }
     cemi_data = (cemi_service_t *)(((uint8_t *)cemi_data) + cemi_msg->additional_info_len);
   }
 
   // Ensure we have a valid service data section
   if ((uint8_t*)cemi_data + sizeof(cemi_service_t) > buf + bytesRead) {
     ESP_LOGE(DEBUG_TAG, "Packet too small to contain service data");
     return;
   }
 
   DEBUG_PRINT("Control1: 0x%02X", cemi_data->control_1.byte);
   DEBUG_PRINT("Control2: 0x%02X", cemi_data->control_2.byte);
   DEBUG_PRINT("Dest Addr Type: 0x%02X", cemi_data->control_2.bits.dest_addr_type);
   
   if (cemi_data->control_2.bits.dest_addr_type != 0x01) {
     DEBUG_PRINTLN("Not a group address destination, ignoring");
     return;
   }
 
   DEBUG_PRINT("Source: 0x%02X%02X", cemi_data->source.bytes.high, cemi_data->source.bytes.low);
   DEBUG_PRINT("Destination: 0x%02X%02X", cemi_data->destination.bytes.high, cemi_data->destination.bytes.low);
 
   // Check if the data length field is valid
   if (cemi_data->data_len > bytesRead - ((uint8_t*)&cemi_data->data[0] - buf)) {
     ESP_LOGE(DEBUG_TAG, "Data length exceeds packet size");
     return;
   }
 
   knx_command_type_t ct = (knx_command_type_t)(((cemi_data->data[0] & 0xC0) >> 6) | ((cemi_data->pci.apci & 0x03) << 2));
   DEBUG_PRINT("Command Type: 0x%02X", ct);
 
   // Process through registered callbacks
   for (int i = 0; i < registered_callback_assignments; ++i) {
     if (cemi_data->destination.value == callback_assignments[i].address.value) {
       DEBUG_PRINT("Found matching callback for address 0x%02X%02X", 
                  callback_assignments[i].address.bytes.high,
                  callback_assignments[i].address.bytes.low);
       
       if (callbacks[callback_assignments[i].callback_id].cond && 
           !callbacks[callback_assignments[i].callback_id].cond()) {
         DEBUG_PRINTLN("Callback condition check failed, skipping");
 #if ALLOW_MULTIPLE_CALLBACKS_PER_ADDRESS
         continue;
 #else
         return;
 #endif
       }
       
       // Safely copy data for callback
       uint8_t data[cemi_data->data_len];
       memcpy(data, cemi_data->data, cemi_data->data_len);
       data[0] = data[0] & 0x3F;
       
       message_t msg = {};
       msg.ct = ct;
       msg.received_on = cemi_data->destination;
       msg.data_len = cemi_data->data_len;
       msg.data = data;
       
       // Call the callback
       callbacks[callback_assignments[i].callback_id].fkt(msg, callbacks[callback_assignments[i].callback_id].arg);
       
 #if ALLOW_MULTIPLE_CALLBACKS_PER_ADDRESS
       continue;
 #else
       return;
 #endif