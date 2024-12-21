#include "I2CSlave.h"
#include "ESP8266Logger.h"

volatile byte I2CSlave::_receivedData = 0;
volatile bool I2CSlave::_newDataReceived = false;
byte I2CSlave::_dataToSend = 0;

I2CSlave::I2CSlave() {
}

void I2CSlave::begin() {
    esp8266_logging::ESP8266Logger::logI2C("Starting Slave initialization");
    esp8266_logging::ESP8266Logger::logI2C("Wire status before begin: 0x" + String(Wire.status(), HEX));
    
    Wire.begin(I2C_SLAVE_ADDR);
    Wire.setClock(I2C_FREQ);
    
    esp8266_logging::ESP8266Logger::logI2C("Wire.begin() completed - Status: 0x" + String(Wire.status(), HEX));
    esp8266_logging::ESP8266Logger::logI2C("Clock set to " + String(I2C_FREQ) + "Hz");
    
    Wire.onReceive(handleReceive);
    Wire.onRequest(handleRequest);
    
    esp8266_logging::ESP8266Logger::logI2C("Handlers registered");
    esp8266_logging::ESP8266Logger::logI2C("SDA Pin: " + String(SDA_PIN) + " SCL Pin: " + String(SCL_PIN));
    esp8266_logging::ESP8266Logger::logI2C("I2C Slave Ready on address 0x" + String(I2C_SLAVE_ADDR, HEX));
}

void I2CSlave::handleReceive(int numBytes) {
    esp8266_logging::ESP8266Logger::logI2C("=== Receive Event Start ===");
    esp8266_logging::ESP8266Logger::logI2C("Wire status: 0x" + String(Wire.status(), HEX));
    esp8266_logging::ESP8266Logger::logI2C("Receiving " + String(numBytes) + " bytes");
    
    int bytesAvailable = Wire.available();
    esp8266_logging::ESP8266Logger::logI2C("Bytes available: " + String(bytesAvailable));
    
    while(Wire.available()) {
        _receivedData = Wire.read();
        esp8266_logging::ESP8266Logger::logI2C("Received byte: 0x" + String(_receivedData, HEX) + " (" + String(_receivedData) + ")");
        _newDataReceived = true;
    }
    
    esp8266_logging::ESP8266Logger::logI2C("Final Wire status: 0x" + String(Wire.status(), HEX));
    esp8266_logging::ESP8266Logger::logI2C("=== Receive Event End ===");
}

void I2CSlave::handleRequest() {
    esp8266_logging::ESP8266Logger::logI2C("=== Request Event Start ===");
    esp8266_logging::ESP8266Logger::logI2C("Wire status: 0x" + String(Wire.status(), HEX));
    esp8266_logging::ESP8266Logger::logI2C("Sending response byte: 0x" + String(_dataToSend, HEX) + " (" + String(_dataToSend) + ")");
    
    Wire.write("Hello from ESP8266!...");

    //Wire.write(_dataToSend);
    
    esp8266_logging::ESP8266Logger::logI2C("Response sent - Final status: 0x" + String(Wire.status(), HEX));
    esp8266_logging::ESP8266Logger::logI2C("=== Request Event End ===");
}

bool I2CSlave::hasNewData() {
    //esp8266_logging::ESP8266Logger::logI2C("Checking for new data: " + String(_newDataReceived ? "YES" : "NO"));
    if (_newDataReceived) {
        esp8266_logging::ESP8266Logger::logI2C("Checking for new data: " + String(_newDataReceived ? "YES" : "NO"));
    }

    return _newDataReceived;
}

byte I2CSlave::getLastReceivedData() {
    esp8266_logging::ESP8266Logger::logI2C("Retrieving last received data: 0x" + String(_receivedData, HEX) + " (" + String(_receivedData) + ")");
    _newDataReceived = false;
    esp8266_logging::ESP8266Logger::logI2C("Reset new data flag");
    return _receivedData;
}

void I2CSlave::setDataToSend(byte data) {
    _dataToSend = data;
    esp8266_logging::ESP8266Logger::logI2C("Data to send updated: 0x" + String(_dataToSend, HEX) + " (" + String(_dataToSend) + ")");
}

void I2CSlave::testPins() {
    esp8266_logging::ESP8266Logger::logI2C("=== Pin Test Start ===");
    esp8266_logging::ESP8266Logger::logI2C("Setting pins to OUTPUT mode");
    
    pinMode(SDA_PIN, OUTPUT);
    pinMode(SCL_PIN, OUTPUT);
    
    esp8266_logging::ESP8266Logger::logI2C("Starting pin test - SDA=" + String(SDA_PIN) + " SCL=" + String(SCL_PIN));
    
    for(int i = 0; i < 20; i++) {
        esp8266_logging::ESP8266Logger::logI2C("Cycle " + String(i+1) + "/20: HIGH");
        digitalWrite(SDA_PIN, HIGH);
        digitalWrite(SCL_PIN, HIGH);
        delay(200);
        
        esp8266_logging::ESP8266Logger::logI2C("Cycle " + String(i+1) + "/20: LOW");
        digitalWrite(SDA_PIN, LOW);
        digitalWrite(SCL_PIN, LOW);
        delay(200);
    }
    
    esp8266_logging::ESP8266Logger::logI2C("=== Pin Test Complete ===");
}