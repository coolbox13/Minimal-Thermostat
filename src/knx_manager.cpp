#include "knx_manager.h"
#include "mqtt_manager.h"
#include "config.h"
#include "web_server.h"

// Static tag for logging
static const char* TAG = "KNX";

KNXManager::KNXManager(ESPKNXIP& knx)
    : _knx(knx), _mqttManager(nullptr), _valvePosition(0), _configManager(nullptr) {
}

void KNXManager::begin() {
    LOG_I(TAG, "Setting up KNX...");

    // Get config manager instance
    _configManager = ConfigManager::getInstance();
    
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

    // Setup addresses based on runtime configuration
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
        
        // Send to appropriate KNX address based on configuration
        if (isUsingTestAddresses()) {
            _knx.write_1byte_int(_testValveAddress, _valvePosition);
            LOG_D(TAG, "Using test address: %d/%d/%d", 
                  _testValveAddress.ga.area, _testValveAddress.ga.line, _testValveAddress.ga.member);
        } else {
            _knx.write_1byte_int(_valveAddress, _valvePosition);
            LOG_D(TAG, "Using production address: %d/%d/%d", 
                 _valveAddress.ga.area, _valveAddress.ga.line, _valveAddress.ga.member);
        }
        
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

bool KNXManager::isUsingTestAddresses() const {
    if (_configManager) {
        return _configManager->getUseTestAddresses();
    }
    
    // Default to test addresses if config manager not available
    LOG_W(TAG, "ConfigManager not available, defaulting to test addresses");
    return true;
}

void KNXManager::reloadAddresses() {
    LOG_I(TAG, "Reloading KNX addresses from configuration");
    setupAddresses();
}

void KNXManager::setupAddresses() {
    bool useTestAddresses = isUsingTestAddresses();
    
    LOG_I(TAG, "Using KNX %s addresses", useTestAddresses ? "TEST" : "PRODUCTION");
    
    // Always set up both address sets for quick switching
    _valveAddress = _knx.GA_to_address(KNX_GA_VALVE_MAIN, KNX_GA_VALVE_MID, KNX_GA_VALVE_SUB);
    _temperatureAddress = _knx.GA_to_address(KNX_GA_TEMPERATURE_MAIN, KNX_GA_TEMPERATURE_MID, KNX_GA_TEMPERATURE_SUB);
    _humidityAddress = _knx.GA_to_address(KNX_GA_HUMIDITY_MAIN, KNX_GA_HUMIDITY_MID, KNX_GA_HUMIDITY_SUB);
    _pressureAddress = _knx.GA_to_address(KNX_GA_PRESSURE_MAIN, KNX_GA_PRESSURE_MID, KNX_GA_PRESSURE_SUB);
    
    // Test valve address is always set
    _testValveAddress = _knx.GA_to_address(KNX_GA_TEST_VALVE_MAIN, KNX_GA_TEST_VALVE_MID, KNX_GA_TEST_VALVE_SUB);
    
    // Debug print the current addresses
    LOG_D(TAG, "KNX addresses configured:");
    LOG_D(TAG, "Production valve address: %d/%d/%d", 
         _valveAddress.ga.area, _valveAddress.ga.line, _valveAddress.ga.member);
    LOG_D(TAG, "Test valve address: %d/%d/%d", 
         _testValveAddress.ga.area, _testValveAddress.ga.line, _testValveAddress.ga.member);
    LOG_D(TAG, "Temperature sensor address: %d/%d/%d", 
         _temperatureAddress.ga.area, _temperatureAddress.ga.line, _temperatureAddress.ga.member);
}

void KNXManager::knxCallback(message_t const &msg, void *arg) {
    // Get the KNXManager instance from the argument
    KNXManager* instance = static_cast<KNXManager*>(arg);
    if (!instance) return;
    
    // Get the destination address from the message
    address_t dest = msg.received_on;
    
    // Check if this is a message for our valve control GA
    // We need to check both possible addresses
    bool isValveMsg = false;
    if (dest.value == instance->_valveAddress.value) {
        isValveMsg = true;
    } else if (dest.value == instance->_testValveAddress.value) {
        isValveMsg = true;
    }
    
    if (isValveMsg) {
        // Check if we have data and it's a write command
        if (msg.data_len > 0 && msg.ct == KNX_CT_WRITE) {
            // Extract valve position value (assuming it's a scaling value 0-100%)
            int position = (int)msg.data[0];
            instance->setValvePosition(position);
            
            LOG_I(TAG, "KNX valve position received: %d%%", position);
        }
    }
}