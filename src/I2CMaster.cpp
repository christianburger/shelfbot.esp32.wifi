#include "I2CMaster.h"

I2CMaster::I2CMaster(uint8_t slaveAddress) : _slaveAddress(slaveAddress) {}

void I2CMaster::begin() {
    Wire.begin(0x42); 
    void begin(); 
    Wire.setClock(10000);  // Set lower clock speed for reliabilitySerial.println("I2C Master initialized");
    
    Wire.beginTransmission(_slaveAddress);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.print("\nI2C Slave device found at address 0x");
        Serial.println(_slaveAddress, HEX);
    } else {
        Serial.print("I2C Slave device not found at address 0x");
        Serial.print(_slaveAddress, HEX);
        Serial.print(", error: ");
        Serial.println(error);
    }
}

void I2CMaster::sendData(byte data) {
    Serial.print("Sending data: 0x");
    Serial.print(data, HEX);
    Serial.print(" to slave: 0x");
    Serial.println(_slaveAddress, HEX);
    
    Wire.beginTransmission(_slaveAddress);
    Wire.write(data);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("Data sent successfully");
    } else {
        Serial.print("Failed to send data, error: ");
        Serial.println(error);
    }
}

byte I2CMaster::receiveData() {
    Serial.print("Requesting 1 byte from slave: 0x");
    Serial.println(_slaveAddress, HEX);
    
    byte bytesReceived = Wire.requestFrom(_slaveAddress, (uint8_t)1);
    if (bytesReceived != 1) {
        Serial.print("Expected 1 byte, received: ");
        Serial.print(bytesReceived);
        Serial.println(" bytes");
        return 0;
    }
    
    if (Wire.available()) {
        byte data = Wire.read();
        Serial.print("Received data: 0x");
        Serial.println(data, HEX);
        return data;
    }
    
    Serial.println("No data available from slave");
    return 0;
}

void I2CMaster::sendMotorCommand(int position) {
    Serial.print("I2C Master: Starting transmission to address 0x");
    Serial.println(_slaveAddress, HEX);
    
    Wire.beginTransmission(_slaveAddress);
    Serial.println("I2C Master: Begin transmission complete");
    
    byte data = position & 0xFF;
    Wire.write(data);
    Serial.print("I2C Master: Wrote byte 0x");
    Serial.println(data, HEX);
    
    byte result = Wire.endTransmission();
    Serial.print("I2C Master: End transmission result: ");
    Serial.println(result);
}

bool I2CMaster::checkMotorResponse() {
    Serial.println("I2C Master: Requesting response");
    Wire.requestFrom(_slaveAddress, (uint8_t)1);
    
    Serial.print("I2C Master: Available bytes: ");
    Serial.println(Wire.available());
    
    if (Wire.available()) {
        byte response = Wire.read();
        Serial.print("I2C Master: Received response: 0x");
        Serial.println(response, HEX);
        return response == 0x01;
    }
    Serial.println("I2C Master: No response received");
    return false;
}

void I2CMaster::scanBus() {
    byte error, address;
    int devicesFound = 0;
    
    Serial.println("Scanning I2C bus...");
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        Serial.print("checking address 0x");
        Serial.println(address, HEX);
        
        if (error == 0) {
            Serial.print("Device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            devicesFound++;
        }
    }
    
    Serial.print("Scanning complete. Found ");
    Serial.print(devicesFound);
    Serial.println(" device(s)");
}