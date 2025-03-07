void printValues() {

  // used this function for debugging
  extern float temperature;
  extern float pressure;
  extern float humidity;

  temperature = bme.readTemperature();
  pressure = (bme.readPressure() / 100.0F);
  humidity = bme.readHumidity();

  Serial.println("--------------------------------");
  printVariable("Temperature = ", temperature, "*C");
  printVariable("Pressure = ", pressure, "hPa");
  printVariable("Humidity = ", humidity, "%");
  Serial.println("--------------------------------");
  printVariable("setpoint = ", setpoint, "*C");
  printVariable("previousError = ", previousError, "");
  printVariable("Kproportional = ", Kproportional, "");
  printVariable("Kintegral = ", Kintegral, "");
  printVariable("Kderivative = ", Kderivative, "");
  Serial.println("--------------------------------");
  printVariable("send_interval = ", send_interval, "");
  printVariable("pid_interval = ", pid_interval, "");
  Serial.println("--------------------------------");
  
  publish_temperature(temperature);
  publish_pressure(pressure);
  publish_humidity(humidity);
}

void send_measurements() {
  printValues();
  /*
  if (millis() >= nextExecution) {
      // Execute the function
      printValues();
      // Update the time for the next execution
      nextExecution = millis() + interval;
    }
  */
}

void printVariable(String prefix, float message, String postfix) {
  Serial.print(prefix);
  Serial.print(message); 
  Serial.println(postfix); 
}