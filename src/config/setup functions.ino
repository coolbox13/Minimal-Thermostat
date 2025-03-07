void setup_serial_connection() {
  Serial.begin(115200);
  Serial.println("Serial connection established at 115200 baud.");
}


void setup_MQTT_connection() {
  // Connect to MQTT
  // TODO!!! make a persistant connection and reconnect if lost
  Serial.print(s+"Connecting to MQTT: "+MQTT_SERVER+" ... ");
  if (client.connect("ESP32Client")) {
    Serial.println("connected");

    mqtt.subscribe("test_esp32/#",  test_esp32_subscriber);
    mqtt.subscribe("tele/BME280/SENSOR",    tele_subscriber);
    mqtt.subscribe("rtl_433", rtl_433_subscriber);
    mqtt.subscribe("esp_thermostat/#", thermostat_settings);

  } else {
    Serial.println(s+"failed, rc="+client.state());
  }
}

void setup_wifimanager() {
WiFiManager wm;
bool res;
res = wm.autoConnect("AutoConnectAP");
if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        Serial.println("Connected to WiFi: ");
        Serial.println(WiFi.SSID());
        Serial.println(WiFi.localIP());
    }
}

void setup_bme_280() {
Serial.println(F("BME280 test"));

    if (!bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    delayTime = 10000;
    
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
                      
    // suggested rate is 1/60Hz (1m)
    delayTime = 10000; // in milliseconds

    Serial.println();
}