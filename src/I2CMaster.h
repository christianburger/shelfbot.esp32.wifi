#pragma once
#include <Arduino.h>
#include <Wire.h>

class I2CMaster {
public:
    I2CMaster(uint8_t slaveAddress);
    void begin();
    void sendData(byte data);
    byte receiveData();
    
    // New methods
    void sendMotorCommand(int position);
    bool checkMotorResponse();
    
    void scanBus();
    
private:
    uint8_t _slaveAddress;
};