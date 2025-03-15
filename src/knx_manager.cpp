#include "knx_manager.h"
#include "mqtt_manager.h"
#include "config.h"

KNXManager::KNXManager(ESPKNXIP& knx)
    : _knx(knx), _mqttManager(nullptr), _valvePosition(0) {
}

void KNXManager::begin() {
    Serial.println("Setting up KNX...");
    
    // Set log level based on KNX_DEBUG_ENABLED
#if KNX_DEBUG_ENABLED
    esp_log_level_set("KNXIP", ESP_LOG_NONE);
#else
    esp_log_level_set("KNXIP", ESP_LOG_NONE);  // Completely disable logs
#endif
    
    // The library might be using multiple log tags, so disable them all
    esp_log_level_set("esp-knx-ip", ESP_LOG_NONE);
    esp_log_level_set("esp-knx-ip-send", ESP_LOG_NONE);
    esp_log_level_set("esp-knx-ip-receive", ESP_LOG_NONE);
    
    // Start KNX with web server
    // In case no webserver needed use: _knx.start(nullptr);
    _knx.start();
    
    // Set physical address (area, line, member)
    _knx.physical_address_set(_knx.PA_to_address(KNX_AREA, KNX_LINE, KNX_MEMBER));
    
    // Setup group addresses
    setupAddresses();
    
    // Register callback for KNX events
    _knx.callback_register("valve_control", knxCallback, this);
    
    Serial.println("KNX initialized");
}

void KNXManager::loop() {
    _knx.loop();
}

void KNXManager::setMQTTManager(MQTTManager* mqttManager) {
    _mqttManager = mqttManager;
}

void KNXManager::sendSensorData(float temperature, float humidity, float pressure) {
    // Send temperature to KNX (DPT 9.001 - 2-byte float)
    _knx.write_2byte_float(_temperatureAddress, temperature);
    
    // Send humidity to KNX (DPT 9.007 - 2-byte float)
    _knx.write_2byte_float(_humidityAddress, humidity);
    
    // Send pressure to KNX (DPT 9.006 - 2-byte float)
    _knx.write_2byte_float(_pressureAddress, pressure);
    
    Serial.println("Sensor data sent to KNX");
}

void KNXManager::setValvePosition(int position) {
    // Constrain position to 0-100%
    position = constrain(position, 0, 100);
    
    if (position != _valvePosition) {
        _valvePosition = position;
        Serial.print("Setting valve position to: ");
        Serial.print(_valvePosition);
        Serial.println("%");
        
        // Send to test KNX address only
        _knx.write_1byte_int(_testValveAddress, _valvePosition);
        
        // Update MQTT if available
        if (_mqttManager) {
            _mqttManager->setValvePosition(_valvePosition);
        }
    }
}

int KNXManager::getValvePosition() const {
    return _valvePosition;
}

void KNXManager::setupAddresses() {
    #if USE_KNX_TEST_ADDRESSES
        // Use test addresses
        Serial.println("Using KNX TEST addresses");
        _valveAddress = _knx.GA_to_address(KNX_GA_TEST_VALVE_MAIN, KNX_GA_TEST_VALVE_MID, KNX_GA_TEST_VALVE_SUB);
        _temperatureAddress = _knx.GA_to_address(KNX_GA_TEMPERATURE_MAIN, KNX_GA_TEMPERATURE_MID, KNX_GA_TEMPERATURE_SUB);
        _humidityAddress = _knx.GA_to_address(KNX_GA_HUMIDITY_MAIN, KNX_GA_HUMIDITY_MID, KNX_GA_HUMIDITY_SUB);
        _pressureAddress = _knx.GA_to_address(KNX_GA_PRESSURE_MAIN, KNX_GA_PRESSURE_MID, KNX_GA_PRESSURE_SUB);
        
        // Test valve address
        _testValveAddress = _knx.GA_to_address(KNX_GA_TEST_VALVE_MAIN, KNX_GA_TEST_VALVE_MID, KNX_GA_TEST_VALVE_SUB);
    #else
        // Use production addresses
        Serial.println("Using KNX PRODUCTION addresses");
        _valveAddress = _knx.GA_to_address(KNX_GA_VALVE_MAIN, KNX_GA_VALVE_MID, KNX_GA_VALVE_SUB);
        _temperatureAddress = _knx.GA_to_address(KNX_GA_TEMPERATURE_MAIN, KNX_GA_TEMPERATURE_MID, KNX_GA_TEMPERATURE_SUB);
        _humidityAddress = _knx.GA_to_address(KNX_GA_HUMIDITY_MAIN, KNX_GA_HUMIDITY_MID, KNX_GA_HUMIDITY_SUB);
        _pressureAddress = _knx.GA_to_address(KNX_GA_PRESSURE_MAIN, KNX_GA_PRESSURE_MID, KNX_GA_PRESSURE_SUB);
        
        // In production environment, valve and test valve addresses can be different
        _testValveAddress = _knx.GA_to_address(KNX_GA_TEST_VALVE_MAIN, KNX_GA_TEST_VALVE_MID, KNX_GA_TEST_VALVE_SUB);
    #endif
    }

void KNXManager::knxCallback(message_t const &msg, void *arg) {
    // Get the KNXManager instance from the argument
    KNXManager* instance = static_cast<KNXManager*>(arg);
    if (!instance) return;
    
    // Get the destination address from the message
    address_t dest = msg.received_on;
    
    // Check if this is a message for our valve control GA
    if (dest.value == instance->_valveAddress.value) {
        // Check if we have data and it's a write command
        if (msg.data_len > 0 && msg.ct == KNX_CT_WRITE) {
            // Extract valve position value (assuming it's a scaling value 0-100%)
            int position = (int)msg.data[0];
            instance->setValvePosition(position);
            
            Serial.print("KNX valve position received: ");
            Serial.println(position);
        }
    }
}