
// publishing the BME280 measurements

void publish_temperature(float temperature) {
  dtostrf(temperature,2,2,msg_out);
  mqtt.publish("test_esp32/temperature", msg_out);
}

void publish_pressure(float pressure) {
  dtostrf(pressure,2,2,msg_out);
  mqtt.publish("test_esp32/pressure", msg_out);
}

void publish_humidity(float humidity) {
  dtostrf(humidity,2,2,msg_out);
  mqtt.publish("test_esp32/humidity", msg_out);
}


// ---------------------------------------- dealing with test and check  and error messages -----------


void test_esp32_subscriber(String topic, String message) {
  Serial.println(s+"Message arrived in function 1 ["+topic+"] "+message);
}

void tele_subscriber(String topic, String message) {
  Serial.println(s+"Message arrived in function 2 ["+topic+"] "+message);
}

void rtl_433_subscriber(String topic, String message) {
  Serial.println(s+"Message arrived in rtl_433 ["+topic+"] "+message);
}

void error_no_float(String variable) {
  Serial.print(variable);
  Serial.println(" - Invalid floating-point number");
}

void error_no_int(String variable) {
  Serial.print(variable);
  Serial.println(" - Invalid integer");
}

// ------------------------------------------ dealing with settings-------------------------

void thermostat_settings(String topic, String message) {
  //Serial.println(s+"Message arrived in function thermostat_settings ["+topic+"] "+message);
  if (topic == "esp_thermostat/setpoint") {
  handleSetpointMessage(topic, message);
  }
  else if (topic == "esp_thermostat/settings/proportional") {
  handleProportionalMessage(topic, message);
  }
  else if (topic == "esp_thermostat/settings/integral") {
  handleIntegralMessage(topic, message);
  }
  else if (topic == "esp_thermostat/settings/derivative") {
  handleDerivativeMessage(topic, message);
  }
   else if (topic == "esp_thermostat/settings/send_interval") {
  handleSendIntervalMessage(topic, message);
  }
   else if (topic == "esp_thermostat/settings/pid_interval") {
  handlePidIntervaleMessage(topic, message);
  }
  else {
  Serial.println("Subtopic not recognized");
  }
}

void handleSetpointMessage(String topic, String message) {
  extern float setpoint;
  Serial.println(s+"Message arrived in function setpoint ["+topic+"] "+message);
  if (message.toFloat()) {
    // Convert the string to a float using atof()
    setpoint = atof(message.c_str());

    // Print the float value to the serial monitor
    Serial.println(setpoint);
  } else {
    // Print an error message to the serial monitor
    error_no_float(topic);
  }
}

void handleProportionalMessage(String topic, String message) {
  extern float Kproportional;
  Kproportional = handleMqttMessageFloat ("kproportional", topic, message);
  Serial.print ("Kproportional : ");
  Serial.println(Kproportional);
  dtostrf(Kproportional,2,2,msg_out);
  mqtt.publish("esp32_thermostat/settings/kproportional", msg_out);
}

void handleIntegralMessage(String topic, String message) {
  extern float Kintegral;
  Kintegral = handleMqttMessageFloat ("kintegral", topic, message);
  Serial.print ("Kintegral : ");
  Serial.println(Kintegral);
  dtostrf(Kintegral,2,2,msg_out);
  mqtt.publish("esp32_thermostat/settings/kintegral", msg_out);
}

void handleDerivativeMessage(String topic, String message) {
  extern float Kderivative;
  Kderivative = handleMqttMessageFloat ("kderivative", topic, message);
  Serial.print ("Kderivative : ");
  Serial.println(Kderivative);
  dtostrf(Kderivative,2,2,msg_out);
  mqtt.publish("esp32_thermostat/settings/kderivative", msg_out);
}

void handleSendIntervalMessage(String topic, String message) {
  extern int send_interval;
  send_interval = handleMqttMessageInt ("send_interval", topic, message);
  Serial.print ("Send interval : ");
  Serial.println(send_interval);
  dtostrf(send_interval,2,2,msg_out);
  mqtt.publish("esp32_thermostat/settings/sent_interval", msg_out);
}

void handlePidIntervaleMessage(String topic, String message) {
  extern int pid_interval;
  pid_interval = handleMqttMessageInt ("pid_interval", topic, message);
  Serial.print ("PID interval : ");
  Serial.println(pid_interval);
  dtostrf(pid_interval,2,2,msg_out);
  mqtt.publish("esp32_thermostat/settings/pid_interval", msg_out);
}

// ------------------------------------------ dealing with PID output-------------------------

// Function to set the thermostat to a specified temperature
void sendPID_Output(float output) {
  dtostrf(output,2,2,msg_out);
  Serial.println("--------------------------------");
  printVariable("Valve Output : ", output, "");
  Serial.println("--------------------------------");
  mqtt.publish("test_esp32/valvePosition", msg_out);
  mqtt.publish("esp32_thermostat/valvePosition", msg_out);
  // Implement this function with code to set the thermostat to the specified temperature
}

// ------------------------------------------ dealing incoming json -------------------------

String getValueFromJson(String Key, String json) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    return "Error parsing JSON";
  }
  JsonObject obj = doc.as<JsonObject>();
  return obj[Key];
}

// ------------------------------------------ dealing with generic mqtt float msg -------------------------

//call should be handelMqttMessage("kproportional", "{kproportional":"0.5"}", "float"

float handleMqttMessageFloat (String varName, String topic, String message) {
  String value;
  float output;
  Serial.println(s+"Message arrived in function setting/["+varName+"] ["+topic+"] "+message);
  value = getValueFromJson(varName, message);
  if (value.toFloat()) {
    // Convert the string to a float using atof()
    output = atof(value.c_str());
    // Print the float value to the serial monitor
    //Serial.println(Kproportional);
    Serial.print("output : ");
    Serial.println(output);
  } else {
      // Print an error message to the serial monitor
      error_no_float(topic);
      output = 0;
    }
  return (output);
  }

// ------------------------------------------ dealing with generic mqtt int msg -------------------------

//call should be handelMqttMessage("kproportional", "{kproportional":"0.5"}", "float"

int handleMqttMessageInt (String varName, String topic, String message) {
  String value;
  int output;
  Serial.println(s+"Message arrived in function setting/["+varName+"] ["+topic+"] "+message);
  value = getValueFromJson(varName, message);
  Serial.print("Value : ");
  Serial.println(value);
  if (value.toInt()) {
    // Convert the string to a float using atof()
    output = atoi(value.c_str());
    // Print the float value to the serial monitor
    //Serial.println(Kproportional);
    Serial.print("output : ");
    Serial.println(output);
  } else {
      // Print an error message to the serial monitor
      error_no_int(topic);
      output = 0;
    }
  return (output);
  }
