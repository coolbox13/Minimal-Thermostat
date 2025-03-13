/**
 * esp-knx-ip library for KNX/IP communication on an ESP32
 * Ported from ESP8266 version
 * Author: Nico Weichbrodt <envy> (Original), Modified for ESP32
 * License: MIT
 */

#include "esp-knx-ip.h"
#include <esp_log.h>
#define DEBUG_TAG "KNXIP"
#define DEBUG_PRINT(fmt, ...) ESP_LOGD(DEBUG_TAG, fmt, ##__VA_ARGS__)
#define DEBUG_PRINTLN(fmt, ...) ESP_LOGD(DEBUG_TAG, fmt "\n", ##__VA_ARGS__)

void ESPKNXIP::__handle_root(AsyncWebServerRequest *request)
{
  String response = "";
  response += "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>KNX</title>";
  response += "<style>body{font-family:Arial;margin:0}h1{margin:0;background-color:#3db9e9;color:white;padding:1em}h2{margin-top:0.5em;margin-bottom:0.5em}form{margin-bottom:1em}label{margin-right:0.5em}input[type=text]{margin-right:0.5em}input[type=submit]{background-color:#3db9e9;color:white;border:0;padding:0.5em;cursor:pointer}table{border-collapse:collapse}td,th{border:1px solid #ddd;padding:8px}tr:nth-child(even){background-color:#f2f2f2}tr:hover{background-color:#ddd}th{padding-top:12px;padding-bottom:12px;text-align:left;background-color:#3db9e9;color:white}</style>";
  response += "</head><body>";
  response += "<h1>KNX</h1>";
  response += "<div style=\"padding:1em\">";
  response += "<h2>Physical Address</h2>";
  response += "<form method=\"post\" action=\"" __PHYS_PATH "\">";
  response += "<input type=\"text\" name=\"area\" value=\"" + String(physaddr.pa.area) + "\" size=\"3\">.";
  response += "<input type=\"text\" name=\"line\" value=\"" + String(physaddr.pa.line) + "\" size=\"3\">.";
  response += "<input type=\"text\" name=\"member\" value=\"" + String(physaddr.pa.member) + "\" size=\"3\">";
  response += "<input type=\"submit\" value=\"Set\">";
  response += "</form>";

  // Feedback
  if (registered_feedbacks > 0)
  {
    response += "<h2>Feedback</h2>";
    for (feedback_id_t i = 0; i < registered_feedbacks; ++i)
    {
      if (feedbacks[i].cond && !feedbacks[i].cond())
      {
        continue;
      }
      response += "<form action=\"" __FEEDBACK_PATH "\" method=\"POST\">";
      response += "<div>";
      response += "<span>" + feedbacks[i].name + ": </span>";
      switch (feedbacks[i].type)
      {
        case FEEDBACK_TYPE_INT:
          response += "<span>" + String(*(int32_t *)feedbacks[i].data) + "</span>";
          break;
        case FEEDBACK_TYPE_FLOAT:
          response += "<span>" + String(*(float *)feedbacks[i].data, (int)feedbacks[i].options.float_options.precision) + "</span>";
          break;
        case FEEDBACK_TYPE_BOOL:
          response += "<span>" + String((*(bool *)feedbacks[i].data) ? "True" : "False") + "</span>";
          break;
        case FEEDBACK_TYPE_ACTION:
          response += "<input type=\"hidden\" name=\"id\" value=\"" + String(i) + "\">";
          response += "<button type=\"submit\">Do this</button>";
          break;
      }
      response += "</div>";
      response += "</form>";
    }
  }

  // Callbacks
  if (registered_callbacks > 0)
    response += "<h2>Callbacks</h2>";

  if (registered_callback_assignments > 0)
  {
    for (uint8_t i = 0; i < registered_callback_assignments; ++i)
    {
      if (callbacks[callback_assignments[i].callback_id].cond && !callbacks[callback_assignments[i].callback_id].cond())
      {
        continue;
      }
      address_t &addr = callback_assignments[i].address;
      response += "<form action=\"" __DELETE_PATH "\" method=\"POST\">";
      response += "<div>";
      response += "<span>" + String(addr.ga.area) + "/" + String(addr.ga.line) + "/" + String(addr.ga.member) + " - ";
      response += callbacks[callback_assignments[i].callback_id].name + "</span>";
      response += "<input type=\"hidden\" name=\"id\" value=\"" + String(i) + "\">";
      response += "<button type=\"submit\">Delete</button>";
      response += "</div>";
      response += "</form>";
    }
  }

  if (registered_callbacks > 0)
  {
    response += "<form action=\"" __REGISTER_PATH "\" method=\"POST\">";
    response += "<div>";
    response += "<input type=\"number\" name=\"area\" min=\"0\" max=\"31\" placeholder=\"Area\">/";
    response += "<input type=\"number\" name=\"line\" min=\"0\" max=\"7\" placeholder=\"Line\">/";
    response += "<input type=\"number\" name=\"member\" min=\"0\" max=\"255\" placeholder=\"Member\"> -> ";
    response += "<select name=\"cb\">";
    for (callback_id_t i = 0; i < registered_callbacks; ++i)
    {
      if (callbacks[i].cond && !callbacks[i].cond())
      {
        continue;
      }
      response += "<option value=\"" + String(i) + "\">" + callbacks[i].name + "</option>";
    }
    response += "</select>";
    response += "<button type=\"submit\">Set</button>";
    response += "</div>";
    response += "</form>";
  }

  // Configuration
  if (registered_configs > 0)
  {
    response += "<h2>Configuration</h2>";
    for (config_id_t i = 0; i < registered_configs; ++i)
    {
      // Check if this config option has a enable condition and if so check that condition
      if (custom_configs[i].cond && !custom_configs[i].cond())
        continue;

      response += "<form action=\"" __CONFIG_PATH "\" method=\"POST\">";
      response += "<div>";
      response += "<span>" + custom_configs[i].name + ": </span>";

      switch (custom_configs[i].type)
      {
        case CONFIG_TYPE_STRING:
          response += "<input type=\"text\" name=\"value\" value=\"" + config_get_string(i) + "\" maxlength=\"" + String(custom_configs[i].len - 1) + "\">";
          break;
        case CONFIG_TYPE_INT:
          response += "<input type=\"number\" name=\"value\" value=\"" + String(config_get_int(i)) + "\">";
          break;
        case CONFIG_TYPE_BOOL:
          response += "<input type=\"checkbox\" name=\"value\"";
          if (config_get_bool(i))
            response += " checked";
          response += ">";
          break;
        case CONFIG_TYPE_OPTIONS:
        {
          response += "<select name=\"value\">";
          option_entry_t *cur = custom_configs[i].data.options;
          while (cur->name != nullptr)
          {
            if (config_get_options(i) == cur->value)
            {
              response += "<option selected value=\"" + String(cur->value) + "\">";
            }
            else
            {
              response += "<option value=\"" + String(cur->value) + "\">";
            }
            response += String(cur->name);
            response += "</option>";
            cur++;
          }
          response += "</select>";
          break;
        }
        case CONFIG_TYPE_GA:
          address_t a = config_get_ga(i);
          response += "<input type=\"number\" name=\"area\" min=\"0\" max=\"31\" value=\"" + String(a.ga.area) + "\">/";
          response += "<input type=\"number\" name=\"line\" min=\"0\" max=\"7\" value=\"" + String(a.ga.line) + "\">/";
          response += "<input type=\"number\" name=\"member\" min=\"0\" max=\"255\" value=\"" + String(a.ga.member) + "\">";
          break;
      }
      response += "<input type=\"hidden\" name=\"id\" value=\"" + String(i) + "\">";
      response += "<button type=\"submit\">Set</button>";
      response += "</div>";
      response += "</form>";
    }
  }

  // Buttons
#if !(DISABLE_EEPROM_BUTTONS && DISABLE_RESTORE_BUTTON && DISABLE_REBOOT_BUTTON)
  response += "<h2>System</h2>";
  response += "<div>";
  // Save to EEPROM
#if !DISABLE_EEPROM_BUTTONS
  response += "<form action=\"" __EEPROM_PATH "\" method=\"POST\" style=\"display:inline-block;margin-right:10px;\">";
  response += "<input type=\"hidden\" name=\"mode\" value=\"1\">";
  response += "<button type=\"submit\">Save to Storage</button>";
  response += "</form>";
  // Restore from EEPROM
  response += "<form action=\"" __EEPROM_PATH "\" method=\"POST\" style=\"display:inline-block;margin-right:10px;\">";
  response += "<input type=\"hidden\" name=\"mode\" value=\"2\">";
  response += "<button type=\"submit\">Restore from Storage</button>";
  response += "</form>";
#endif
#if !DISABLE_RESTORE_BUTTON
  // Load Defaults
  response += "<form action=\"" __RESTORE_PATH "\" method=\"POST\" style=\"display:inline-block;margin-right:10px;\">";
  response += "<button type=\"submit\">Restore defaults</button>";
  response += "</form>";
#endif
#if !DISABLE_REBOOT_BUTTON
  // Reboot
  response += "<form action=\"" __REBOOT_PATH "\" method=\"POST\" style=\"display:inline-block;\">";
  response += "<button type=\"submit\">Reboot</button>";
  response += "</form>";
#endif
  response += "</div>";
#endif

  // End of page
  response += "</div></body></html>";
  request->send(200, "text/html", response);
}

void ESPKNXIP::__handle_register(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Register called");
  if (!request->hasParam("area", true) || !request->hasParam("line", true) || 
      !request->hasParam("member", true) || !request->hasParam("cb", true))
  {
    request->redirect(__ROOT_PATH);
    return;
  }
  
  uint8_t area = request->getParam("area", true)->value().toInt();
  uint8_t line = request->getParam("line", true)->value().toInt();
  uint8_t member = request->getParam("member", true)->value().toInt();
  callback_id_t cb = (callback_id_t)request->getParam("cb", true)->value().toInt();
  
  DEBUG_PRINT("Got args: %d/%d/%d/%d", area, line, member, cb);
  
  if (area > 31 || line > 7)
  {
    DEBUG_PRINTLN("Area or Line wrong");
    request->redirect(__ROOT_PATH);
    return;
  }
  
  if (cb >= registered_callbacks)
  {
    DEBUG_PRINTLN("Invalid callback id");
    request->redirect(__ROOT_PATH);
    return;
  }
  
  address_t ga = {.ga={line, area, member}};
  __callback_register_assignment(ga, cb);
  
  request->redirect(__ROOT_PATH);
}

void ESPKNXIP::__handle_delete(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Delete called");
  if (!request->hasParam("id", true))
  {
    request->redirect(__ROOT_PATH);
    return;
  }
  
  callback_assignment_id_t id = (callback_assignment_id_t)request->getParam("id", true)->value().toInt();
  
  DEBUG_PRINT("Got args: %d", id);
  
  if (id >= registered_callback_assignments)
  {
    DEBUG_PRINTLN("ID wrong");
    request->redirect(__ROOT_PATH);
    return;
  }
  
  __callback_delete_assignment(id);
  request->redirect(__ROOT_PATH);
}

void ESPKNXIP::__handle_set(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Set called");
  if (!request->hasParam("area", true) || !request->hasParam("line", true) || !request->hasParam("member", true))
  {
    request->redirect(__ROOT_PATH);
    return;
  }
  
  uint8_t area = request->getParam("area", true)->value().toInt();
  uint8_t line = request->getParam("line", true)->value().toInt();
  uint8_t member = request->getParam("member", true)->value().toInt();
  
  DEBUG_PRINT("Got args: %d.%d.%d", area, line, member);
  
  if (area > 31 || line > 7)
  {
    DEBUG_PRINTLN("Area or Line wrong");
    request->redirect(__ROOT_PATH);
    return;
  }
  
  physaddr.bytes.high = (area << 4) | line;
  physaddr.bytes.low = member;
  
  request->redirect(__ROOT_PATH);
}

void ESPKNXIP::__handle_config(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Config called");
  if (!request->hasParam("id", true))
  {
    request->redirect(__ROOT_PATH);
    return;
  }
  
  config_id_t id = request->getParam("id", true)->value().toInt();
  
  DEBUG_PRINT("Got args: %d", id);
  
  if (id < 0 || id >= registered_configs)
  {
    DEBUG_PRINTLN("ID wrong");
    request->redirect(__ROOT_PATH);
    return;
  }
  
  switch (custom_configs[id].type)
  {
    case CONFIG_TYPE_STRING:
    {
      if (!request->hasParam("value", true))
      {
        request->redirect(__ROOT_PATH);
        return;
      }
      String v = request->getParam("value", true)->value();
      if (v.length() >= custom_configs[id].len)
      {
        request->redirect(__ROOT_PATH);
        return;
      }
      __config_set_flags(id, CONFIG_FLAGS_VALUE_SET);
      __config_set_string(id, v);
      break;
    }
    case CONFIG_TYPE_INT:
    {
      if (!request->hasParam("value", true))
      {
        request->redirect(__ROOT_PATH);
        return;
      }
      __config_set_flags(id, CONFIG_FLAGS_VALUE_SET);
      __config_set_int(id, request->getParam("value", true)->value().toInt());
      break;
    }
    case CONFIG_TYPE_BOOL:
    {
      __config_set_flags(id, CONFIG_FLAGS_VALUE_SET);
      __config_set_bool(id, request->hasParam("value", true) && request->getParam("value", true)->value().equals("on"));
      break;
    }
    case CONFIG_TYPE_OPTIONS:
    {
      if (!request->hasParam("value", true))
      {
        request->redirect(__ROOT_PATH);
        return;
      }
      uint8_t val = (uint8_t)request->getParam("value", true)->value().toInt();
      DEBUG_PRINT("Value: %d", val);
      config_set_options(id, val);
      break;
    }
    case CONFIG_TYPE_GA:
    {
      if (!request->hasParam("area", true) || !request->hasParam("line", true) || !request->hasParam("member", true))
      {
        request->redirect(__ROOT_PATH);
        return;
      }
      uint8_t area = request->getParam("area", true)->value().toInt();
      uint8_t line = request->getParam("line", true)->value().toInt();
      uint8_t member = request->getParam("member", true)->value().toInt();
      if (area > 31 || line > 7)
      {
        DEBUG_PRINTLN("Area or Line wrong");
        request->redirect(__ROOT_PATH);
        return;
      }
      address_t tmp;
      tmp.bytes.high = (area << 3) | line;
      tmp.bytes.low = member;
      __config_set_flags(id, CONFIG_FLAGS_VALUE_SET);
      __config_set_ga(id, tmp);
      break;
    }
  }
  
  request->redirect(__ROOT_PATH);
}

void ESPKNXIP::__handle_feedback(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Feedback called");
  if (!request->hasParam("id", true))
  {
    request->redirect(__ROOT_PATH);
    return;
  }
  
  config_id_t id = request->getParam("id", true)->value().toInt();
  
  DEBUG_PRINT("Got args: %d", id);
  
  if (id < 0 || id >= registered_feedbacks)
  {
    DEBUG_PRINTLN("ID wrong");
    request->redirect(__ROOT_PATH);
    return;
  }
  
  switch (feedbacks[id].type)
  {
    case FEEDBACK_TYPE_ACTION:
    {
      feedback_action_fptr_t func = (feedback_action_fptr_t)feedbacks[id].data;
      void *arg = feedbacks[id].options.action_options.arg;
      func(arg);
      break;
    }
    default:
      DEBUG_PRINTLN("Feedback has no action");
      break;
  }
  
  request->redirect(__ROOT_PATH);
}

#if !DISABLE_RESTORE_BUTTON
void ESPKNXIP::__handle_restore(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Restore called");
  memcpy(custom_config_data, custom_config_default_data, MAX_CONFIG_SPACE);
  request->redirect(__ROOT_PATH);
}
#endif

#if !DISABLE_REBOOT_BUTTON
void ESPKNXIP::__handle_reboot(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Rebooting!");
  request->redirect(__ROOT_PATH);
  // Use a timer to delay the reboot so the response can be sent
  delay(1000);
  ESP.restart();
}
#endif

#if !DISABLE_EEPROM_BUTTONS
void ESPKNXIP::__handle_eeprom(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Storage options called");
  if (!request->hasParam("mode", true))
  {
    request->redirect(__ROOT_PATH);
    return;
  }
  
  uint8_t mode = request->getParam("mode", true)->value().toInt();
  
  DEBUG_PRINT("Got args: %d", mode);
  
  if (mode == 1)
  {
    // save
    save_to_preferences();
  }
  else if (mode == 2)
  {
    // restore
    restore_from_preferences();
  }
  
  request->redirect(__ROOT_PATH);
}
#endif