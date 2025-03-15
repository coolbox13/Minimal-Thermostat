#include "knx_manager.h"
#include "mqtt_manager.h"
#include "config.h"
#include "web_server.h"

// Static tag for logging
static const char* TAG = "KNX";

KNXManager::KNXManager(ESPKNXIP& knx)
    : _knx(knx), _mqttManager(nullptr), _valvePosition(0) {
}

void KNXManager::begin() {
    LOG_I(TAG, "Setting up KNX...");

    // Set log level based on KNX_DEBUG_ENABLED
    #if KNX_DEBUG_ENABLED
    esp_log_level_set("KNXIP", ESP_LOG_DEBUG);
    #else
    esp_log_level_set("KNXIP", ESP_LOG_NONE);  // Completely disable logs
    #endif

    // Get WebServerManager instance
    WebServerManager* webServerManager = WebServerManager::getInstance();
    if (webServerManager) {
        // Start KNX with web server
        _knx.start(webServerManager->getServer());
    } else {
        // Start KNX without web server
        _knx.start();
    }

    // Set physical address
    _knx.physical_address_set(_knx.PA_to_address(KNX_AREA, KNX_LINE, KNX_MEMBER));

    // Setup addresses
    setupAddresses();

    // Register callback for KNX events
    _knx.callback_register("valve_control", knxCallback, this);

    LOG_I(TAG, "KNX initialized");
}

void KNXManager::loop() {
    _knx.loop();
    
    // Process queued messages
    processQueue();
}

void KNXManager::processQueue() {
    KnxMessage message;
    bool hasMessage = false;
    
    // Get a message from the queue (thread-safe)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_messageQueue.empty()) {
            message = _messageQueue.front();
            _messageQueue.pop();
            hasMessage = true;
        }
    }
    
    // Process the message if we got one
    if (hasMessage) {
        switch (message.type) {
            case KnxMessage::VALVE_POSITION:
                _setValvePosition(static_cast<int>(message.value1));
                break;
            case KnxMessage::SEND_SENSOR_DATA:
                _sendSensorData(message.value1, message.value2, message.value3);
                break;
            default:
                LOG_W(TAG, "Unknown message type in queue: %d", message.type);
                break;
        }
    }
}

void KNXManager::setMQTTManager(MQTTManager* mqttManager) {
    _mqttManager = mqttManager;
}

void KNXManager::sendSensorData(float temperature, float humidity, float pressure) {
    // Queue the message for processing in the main loop
    KnxMessage message;
    message.type = KnxMessage::SEND_SENSOR_DATA;
    message.value1 = temperature;
    message.value2 = humidity;
    message.value3 = pressure;
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _messageQueue.push(message);
    }
}

void KNXManager::_sendSensorData(float temperature, float humidity, float pressure) {
    // Send temperature to KNX (DPT 9.001 - 2-byte float)
    _knx.write_2byte_float(_temperatureAddress, temperature);
    
    // Send humidity to KNX (DPT 9.007 - 2-byte float)
    _knx.write_2byte_float(_humidityAddress, humidity);
    
    // Send pressure to KNX (DPT 9.006 - 2-byte float)
    _knx.write_2byte_float(_pressureAddress, pressure);
    
    LOG_I(TAG, "Sensor data sent to KNX: temp=%.2f, humidity=%.2f, pressure=%.2f", 
          temperature, humidity, pressure);
}

void KNXManager::setValvePosition(int position) {
    // Queue the message for processing in the main loop
    KnxMessage message;
    message.type = KnxMessage::VALVE_POSITION;
    message.value1 = static_cast<float>(position);
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _messageQueue.push(message);
    }
}

void KNXManager::_setValvePosition(int position) {
    // Constrain position to 0-100%
    position = constrain(position, 0, 100);
    
    if (position != _valvePosition) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _valvePosition = position;
        }
        
        LOG_I(TAG, "Setting valve position to: %d%%", _valvePosition);
        
        // Send to test KNX address only
        _knx.write_1byte_int(_testValveAddress, _valvePosition);
        
        // Update MQTT if available
        if (_mqttManager) {
            _mqttManager->setValvePosition(_valvePosition);
        }
    }
}

int KNXManager::getValvePosition() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _valvePosition;
}

void KNXManager::setupAddresses() {
    #if USE_KNX_TEST_ADDRESSES
        // Use test addresses
        LOG_I(TAG, "Using KNX TEST addresses");
        _valveAddress = _knx.GA_to_address(KNX_GA_TEST_VALVE_MAIN, KNX_GA_TEST_VALVE_MID, KNX_GA_TEST_VALVE_SUB);
        _temperatureAddress = _knx.GA_to_address(KNX_GA_TEMPERATURE_MAIN, KNX_GA_TEMPERATURE_MID, KNX_GA_TEMPERATURE_SUB);
        _humidityAddress = _knx.GA_to_address(KNX_GA_HUMIDITY_MAIN, KNX_GA_HUMIDITY_MID, KNX_GA_HUMIDITY_SUB);
        _pressureAddress = _knx.GA_to_address(KNX_GA_PRESSURE_MAIN, KNX_GA_PRESSURE_MID, KNX_GA_PRESSURE_SUB);
        
        // Test valve address
        _testValveAddress = _knx.GA_to_address(KNX_GA_TEST_VALVE_MAIN, KNX_GA_TEST_VALVE_MID, KNX_GA_TEST_VALVE_SUB);
    #else
        // Use production addresses
        LOG_I(TAG, "Using KNX PRODUCTION addresses");
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
            
            LOG_I(TAG, "KNX valve position received: %d%%", position);
        }
    }
}