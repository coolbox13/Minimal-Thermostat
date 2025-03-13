/**
 * esp-knx-ip library for KNX/IP communication on an ESP32
 * Ported from ESP8266 version
 * Author: Nico Weichbrodt <envy> (Original), Modified for ESP32
 * License: MIT
 */

 #include "esp-knx-ip.h"
 #include <esp_log.h>
 #include "config.h"
 #include "utils.h"

 // Change these debug macros to use ESP_LOGD directly with conditional compilation
 #define DEBUG_TAG "KNXIP"
 #ifdef KNX_DEBUG_ENABLED
   #define DEBUG_PRINT(fmt, ...) ESP_LOGD(DEBUG_TAG, fmt, ##__VA_ARGS__)
   #define DEBUG_PRINTLN(fmt, ...) ESP_LOGD(DEBUG_TAG, fmt "\n", ##__VA_ARGS__)
 #else
   #define DEBUG_PRINT(fmt, ...)
   #define DEBUG_PRINTLN(fmt, ...)
 #endif

 /**
  * Send functions
  */
 
 void ESPKNXIP::send(address_t const &receiver, knx_command_type_t ct, uint8_t data_len, uint8_t *data)
 {
   if (receiver.value == 0)
	 return;
 #if SEND_CHECKSUM
   uint32_t len = 6 + 2 + 8 + data_len + 1; // knx_pkt + cemi_msg + cemi_service + data + checksum
 #else
   uint32_t len = 6 + 2 + 8 + data_len; // knx_pkt + cemi_msg + cemi_service + data
 #endif
   DEBUG_PRINT("Creating packet with len %d", len);
   uint8_t buf[len];
   knx_ip_pkt_t *knx_pkt = (knx_ip_pkt_t *)buf;
   knx_pkt->header_len = 0x06;
   knx_pkt->protocol_version = 0x10;
   knx_pkt->service_type = __ntohs(KNX_ST_ROUTING_INDICATION);
   knx_pkt->total_len.len = __ntohs(len);
   cemi_msg_t *cemi_msg = (cemi_msg_t *)knx_pkt->pkt_data;
   cemi_msg->message_code = KNX_MT_L_DATA_IND;
   cemi_msg->additional_info_len = 0;
   cemi_service_t *cemi_data = &cemi_msg->data.service_information;
   cemi_data->control_1.bits.confirm = 0;
   cemi_data->control_1.bits.ack = 0;
   cemi_data->control_1.bits.priority = B11;
   cemi_data->control_1.bits.system_broadcast = 0x01;
   cemi_data->control_1.bits.repeat = 0x01;
   cemi_data->control_1.bits.reserved = 0;
   cemi_data->control_1.bits.frame_type = 0x01;
   cemi_data->control_2.bits.extended_frame_format = 0x00;
   cemi_data->control_2.bits.hop_count = 0x06;
   cemi_data->control_2.bits.dest_addr_type = 0x01;
   cemi_data->source = physaddr;
   cemi_data->destination = receiver;
   cemi_data->data_len = data_len;
   cemi_data->pci.apci = (ct & 0x0C) >> 2;
   cemi_data->pci.tpci_seq_number = 0x00;
   cemi_data->pci.tpci_comm_type = KNX_COT_UDP;
   memcpy(cemi_data->data, data, data_len);
   cemi_data->data[0] = (cemi_data->data[0] & 0x3F) | ((ct & 0x03) << 6);
 
 #if SEND_CHECKSUM
   uint8_t cs = buf[0] ^ buf[1];
   for (uint32_t i = 2; i < len - 1; ++i)
   {
	 cs ^= buf[i];
   }
   buf[len - 1] = cs;
 #endif
 
   // Replace the debug output with our custom logging
 #ifdef KNX_DEBUG_ENABLED
   DEBUG_PRINT("Sending packet:");
   // Instead of logging each byte separately, create a summary
   char packetSummary[64];
   snprintf(packetSummary, sizeof(packetSummary), "KNX packet to %d.%d.%d, len=%d", 
            (receiver.value >> 12) & 0x0F, 
            (receiver.value >> 8) & 0x0F, 
            receiver.value & 0xFF, 
            len);
   logKnxMessage(packetSummary);
 #endif
 
   // ESP32 UDP multicast: use beginPacket() instead of specifying the local IP.
   udp.beginPacket(MULTICAST_IP, MULTICAST_PORT);
   udp.write(buf, len);
   udp.endPacket();
 }
 
 void ESPKNXIP::send_1bit(address_t const &receiver, knx_command_type_t ct, uint8_t bit)
 {
   uint8_t buf[] = {(uint8_t)(bit & 0b00000001)};
   send(receiver, ct, 1, buf);
 }
 
 void ESPKNXIP::send_2bit(address_t const &receiver, knx_command_type_t ct, uint8_t twobit)
 {
   uint8_t buf[] = {(uint8_t)(twobit & 0b00000011)};
   send(receiver, ct, 1, buf);
 }
 
 void ESPKNXIP::send_4bit(address_t const &receiver, knx_command_type_t ct, uint8_t fourbit)
 {
   uint8_t buf[] = {(uint8_t)(fourbit & 0b00001111)};
   send(receiver, ct, 1, buf);
 }
 
 void ESPKNXIP::send_1byte_int(address_t const &receiver, knx_command_type_t ct, int8_t val)
 {
   uint8_t buf[] = {0x00, (uint8_t)val};
   send(receiver, ct, 2, buf);
 }
 
 void ESPKNXIP::send_1byte_uint(address_t const &receiver, knx_command_type_t ct, uint8_t val)
 {
   uint8_t buf[] = {0x00, val};
   send(receiver, ct, 2, buf);
 }
 
 void ESPKNXIP::send_2byte_int(address_t const &receiver, knx_command_type_t ct, int16_t val)
 {
   uint8_t buf[] = {0x00, (uint8_t)(val >> 8), (uint8_t)(val & 0x00FF)};
   send(receiver, ct, 3, buf);
 }
 
 void ESPKNXIP::send_2byte_uint(address_t const &receiver, knx_command_type_t ct, uint16_t val)
 {
   uint8_t buf[] = {0x00, (uint8_t)(val >> 8), (uint8_t)(val & 0x00FF)};
   send(receiver, ct, 3, buf);
 }
 
 void ESPKNXIP::send_2byte_float(address_t const &receiver, knx_command_type_t ct, float val)
 {
   float v = val * 100.0f;
   int e = 0;
   for (; v < -2048.0f; v /= 2)
   ++e;
   for (; v > 2047.0f; v /= 2)
   ++e;
   long m = (long)round(v) & 0x7FF;
   short msb = (short) (e << 3 | m >> 8);
   if (val < 0.0f)
   msb |= 0x80;
   uint8_t buf[] = {0x00, (uint8_t)msb, (uint8_t)m};
   send(receiver, ct, 3, buf);
 }
 
 void ESPKNXIP::send_3byte_time(address_t const &receiver, knx_command_type_t ct, uint8_t weekday, uint8_t hours, uint8_t minutes, uint8_t seconds)
 {
   weekday <<= 5;
   uint8_t buf[] = {0x00, (uint8_t)(((weekday << 5) & 0xE0) | (hours & 0x1F)), (uint8_t)(minutes & 0x3F), (uint8_t)(seconds & 0x3F)};
   send(receiver, ct, 4, buf);
 }
 
 void ESPKNXIP::send_3byte_date(address_t const &receiver, knx_command_type_t ct, uint8_t day, uint8_t month, uint8_t year)
 {
   uint8_t buf[] = {0x00, (uint8_t)(day & 0x1F), (uint8_t)(month & 0x0F), year};
   send(receiver, ct, 4, buf);
 }
 
 void ESPKNXIP::send_3byte_color(address_t const &receiver, knx_command_type_t ct, uint8_t red, uint8_t green, uint8_t blue)
 {
   uint8_t buf[] = {0x00, red, green, blue};
   send(receiver, ct, 4, buf);
 }
 
 void ESPKNXIP::send_4byte_int(address_t const &receiver, knx_command_type_t ct, int32_t val)
 {
   uint8_t buf[] = {0x00,
					(uint8_t)((val & 0xFF000000) >> 24),
					(uint8_t)((val & 0x00FF0000) >> 16),
					(uint8_t)((val & 0x0000FF00) >> 8),
					(uint8_t)((val & 0x000000FF) >> 0)};
   send(receiver, ct, 5, buf);
 }
 
 void ESPKNXIP::send_4byte_uint(address_t const &receiver, knx_command_type_t ct, uint32_t val)
 {
   uint8_t buf[] = {0x00,
					(uint8_t)((val & 0xFF000000) >> 24),
					(uint8_t)((val & 0x00FF0000) >> 16),
					(uint8_t)((val & 0x0000FF00) >> 8),
					(uint8_t)((val & 0x000000FF) >> 0)};
   send(receiver, ct, 5, buf);
 }
 
 void ESPKNXIP::send_4byte_float(address_t const &receiver, knx_command_type_t ct, float val)
 {
   uint8_t buf[] = {0x00, ((uint8_t *)&val)[3], ((uint8_t *)&val)[2], ((uint8_t *)&val)[1], ((uint8_t *)&val)[0]};
   send(receiver, ct, 5, buf);
 }
 
 void ESPKNXIP::send_14byte_string(address_t const &receiver, knx_command_type_t ct, const char *val)
 {
   // DPT16 strings are always 14 bytes long, however the data array is one larger due to the telegram structure.
   // The first byte needs to be zero, string start after that.
   uint8_t buf[15] = {0x00};
   int len = strlen(val);
   if (len > 14)
   {
	 len = 14;
   }
   memcpy(buf+1, val, len);
   send(receiver, ct, 15, buf);
 }