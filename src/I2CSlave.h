#pragma once
#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 4  // GPIO04
#define SCL_PIN 2  // GPIO02
#define I2C_FREQ 10000  // 10kHz
#define I2C_SLAVE_ADDR 8 

class I2CSlave {
public:
    I2CSlave();
    void begin();
    bool hasNewData();
    byte getLastReceivedData();
    void setDataToSend(byte data);
    void testPins();
    
private:
    static void handleReceive(int numBytes);
    static void handleRequest();
    static volatile byte _receivedData;
    static volatile bool _newDataReceived;
    static byte _dataToSend;
};