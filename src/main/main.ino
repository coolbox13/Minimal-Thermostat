void setup() {
    // Common initialization
    setupSerial();
    setupFileSystem();
    setupWiFi();
    
    // Subsystem initialization
    setupSensors();
    setupThermostatControl();
    setupKNXCommunication();
    setupMQTT();
    setupWebServer();
  }