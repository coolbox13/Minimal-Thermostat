#include <String.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <PubSubClientTools.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Thread.h>             // https://github.com/ivanseidel/ArduinoThread
#include <ThreadController.h>
#include "ArduinoJson.h"
#include <iostream>
using namespace std;

#define MQTT_SERVER "192.168.178.32"

Adafruit_BME280 bme; // I2C

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, 1883, espClient);
PubSubClientTools mqtt(client);

ThreadController threadControl = ThreadController();
Thread thread_send_measurements = Thread();
Thread thread_pidThermostat = Thread();
Thread thread_setup_MQTT_connection = Thread();

// PIDController node(20, 30, 0.1, 10, 1, 0, 0.1, 1000, true, false);

#define INTERVAL_SEND 10000
#define INTERVAL_PID 30000
#define INTERVAL_MQTT_CHECK 30000

int send_interval = INTERVAL_SEND;
int pid_interval = INTERVAL_PID;

unsigned long delayTime = 10000;
// Set the interval for executing the function
const unsigned long interval = 30000; // 30 seconds in milliseconds
// Set the next time the function should be executed
unsigned long nextExecution = millis() + interval;

// various variables
int value = 0;
int value2 = 100;
const String s = "";
char msg_out[20] = "";

// Global Variables for PID control
float previousError = 0.0;
float integral = 0.0;
float Kproportional = 1.0;
float Kintegral = 0.1;
float Kderivative = 0.01;
float setpoint = 19;
float temperature = setpoint;
float pressure;
float humidity;
bool enable;
unsigned long last_sample_time = millis();

void setup() {
  setup_serial_connection();
  setup_wifimanager();
  setup_MQTT_connection();
  setup_bme_280();

  // Enable Threads
  thread_send_measurements.onRun(send_measurements);
  thread_send_measurements.setInterval(INTERVAL_SEND);
  threadControl.add(&thread_send_measurements);

  thread_pidThermostat.onRun(pidThermostat);
  thread_pidThermostat.setInterval(INTERVAL_PID);
  threadControl.add(&thread_pidThermostat);

  thread_setup_MQTT_connection.onRun(setup_MQTT_connection);
  thread_setup_MQTT_connection.setInterval(INTERVAL_MQTT_CHECK);
  threadControl.add(&thread_setup_MQTT_connection);
}

void loop() {
  client.loop();
  threadControl.run();
  //send_measurements();
  //pidThermostat();
}


