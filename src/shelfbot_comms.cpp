#include "shelfbot_comms.h"
#include <AccelStepper.h>

extern AccelStepper steppers[];

void ShelfbotComms::begin() {
    Serial.begin(115200);
    #ifdef I2C_SLAVE_DEVICE
    Serial.println("\nStarting I2C as slave");
    I2CSlave::begin();
    #else
    Serial.println("\nStarting I2C as master");
    I2CMaster::begin();
    #endif
}

void ShelfbotComms::sendCommand(CommandType cmd, uint16_t value) {
    String formattedCmd = formatCommand(cmd, String(value));
    
    #ifndef I2C_SLAVE_DEVICE
    Serial.printf("\n\nSENT COMMAND: 0x%04X value: %d\n", cmd, value);
    String response = I2CMaster::communicateWithSlave(I2C_SLAVE_ADDR, formattedCmd.c_str());
    Serial.printf("\nRECEIVED RESPONSE: %s\n", response.c_str());
    #endif
}

CommandType ShelfbotComms::parseCommand(const String& message) {
    if(message.length() < 4) return CMD_UNKNOWN;
    uint16_t cmdCode = (message[2] << 8) | message[3];
    return static_cast<CommandType>(cmdCode);
}

String ShelfbotComms::parseValue(const String& message) {
    int valueStart = message.indexOf((char)(CMD_VALUE_START >> 8));
    int valueEnd = message.indexOf((char)(CMD_VALUE_END >> 8));
    
    if(valueStart < 0 || valueEnd < 0) return "";
    valueStart += 2;
    return message.substring(valueStart, valueEnd);
}

String ShelfbotComms::formatCommand(CommandType cmd, const String& value) {
    String payload;
    payload.reserve(32);
    payload += (char)(cmd >> 8);
    payload += (char)(cmd & 0xFF);
    payload += (char)(CMD_VALUE_START >> 8);
    payload += (char)(CMD_VALUE_START & 0xFF);
    payload += value;
    payload += (char)(CMD_VALUE_END >> 8);
    payload += (char)(CMD_VALUE_END & 0xFF);
    
    byte checksum = 0;
    for(size_t i = 0; i < payload.length(); i++) {
        checksum ^= payload[i];
    }
    
    String command;
    command.reserve(payload.length() + 16);
    command += (char)(CMD_START_MARKER >> 8);
    command += (char)(CMD_START_MARKER & 0xFF);
    command += payload;
    command += (char)(CMD_CHECKSUM_START >> 8);
    command += (char)(CMD_CHECKSUM_START & 0xFF);
    command += String(checksum, HEX);
    command += (char)(CMD_CHECKSUM_END >> 8);
    command += (char)(CMD_CHECKSUM_END & 0xFF);
    command += (char)(CMD_END_MARKER >> 8);
    command += (char)(CMD_END_MARKER & 0xFF);
    
    return command;
}

bool ShelfbotComms::verifyChecksum(const String& message) {
    int valueStart = message.indexOf((char)(CMD_VALUE_START >> 8));
    int checksumStart = message.indexOf((char)(CMD_CHECKSUM_START >> 8));
    
    if(valueStart < 0 || checksumStart < 0) return false;
    
    byte calculatedChecksum = 0;
    for(int i = 2; i < checksumStart; i++) {
        calculatedChecksum ^= message[i];
    }
    
    String receivedChecksum = message.substring(checksumStart + 2, message.indexOf((char)(CMD_CHECKSUM_END >> 8)));
    return calculatedChecksum == strtol(receivedChecksum.c_str(), NULL, 16);
}

void ShelfbotComms::handleCommand(char* message) {
    String msg(message);
    if (!verifyChecksum(msg)) {
        #ifdef I2C_SLAVE_DEVICE
        I2CSlave::setResponse(formatCommand(CMD_UNKNOWN, "CHECKSUM_ERR").c_str());
        #endif
        return;
    }

    CommandType cmd = parseCommand(msg);
    String value = parseValue(msg);
    String response;

    switch(cmd) {
        case CMD_GET_TEMP:
            response = formatCommand(CMD_GET_TEMP, String(analogRead(A0) * 0.48876f));
            break;
            
        case CMD_SET_LED:
            digitalWrite(LED_BUILTIN, value.toInt());
            response = formatCommand(CMD_SET_LED, "OK");
            break;
            
        case CMD_READ_ADC:
            response = formatCommand(CMD_READ_ADC, String(analogRead(value.toInt())));
            break;
            
        case CMD_SET_PWM:
            analogWrite(value.toInt(), value.toInt());
            response = formatCommand(CMD_SET_PWM, "OK");
            break;
            
        case CMD_GET_STATUS:
            response = formatCommand(CMD_GET_STATUS, String(millis()));
            break;
            
        case CMD_SET_MOTOR_1:
            steppers[0].moveTo(value.toInt());
            response = formatCommand(CMD_SET_MOTOR_1, String(steppers[0].currentPosition()));
            break;
            
        case CMD_SET_MOTOR_2:
            steppers[1].moveTo(value.toInt());
            response = formatCommand(CMD_SET_MOTOR_2, String(steppers[1].currentPosition()));
            break;
            
        case CMD_SET_MOTOR_3:
            steppers[2].moveTo(value.toInt());
            response = formatCommand(CMD_SET_MOTOR_3, String(steppers[2].currentPosition()));
            break;
            
        case CMD_SET_MOTOR_4:
            steppers[3].moveTo(value.toInt());
            response = formatCommand(CMD_SET_MOTOR_4, String(steppers[3].currentPosition()));
            break;
            
        case CMD_SET_MOTOR_5:
            steppers[4].moveTo(value.toInt());
            response = formatCommand(CMD_SET_MOTOR_5, String(steppers[4].currentPosition()));
            break;
            
        case CMD_SET_MOTOR_6:
            steppers[5].moveTo(value.toInt());
            response = formatCommand(CMD_SET_MOTOR_6, String(steppers[5].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_1_POS:
            response = formatCommand(CMD_GET_MOTOR_1_POS, String(steppers[0].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_2_POS:
            response = formatCommand(CMD_GET_MOTOR_2_POS, String(steppers[1].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_3_POS:
            response = formatCommand(CMD_GET_MOTOR_3_POS, String(steppers[2].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_4_POS:
            response = formatCommand(CMD_GET_MOTOR_4_POS, String(steppers[3].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_5_POS:
            response = formatCommand(CMD_GET_MOTOR_5_POS, String(steppers[4].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_6_POS:
            response = formatCommand(CMD_GET_MOTOR_6_POS, String(steppers[5].currentPosition()));
            break;
            
        case CMD_GET_MOTOR_1_VEL:
            response = formatCommand(CMD_GET_MOTOR_1_VEL, String(steppers[0].speed()));
            break;
            
        case CMD_GET_MOTOR_2_VEL:
            response = formatCommand(CMD_GET_MOTOR_2_VEL, String(steppers[1].speed()));
            break;
            
        case CMD_GET_MOTOR_3_VEL:
            response = formatCommand(CMD_GET_MOTOR_3_VEL, String(steppers[2].speed()));
            break;
            
        case CMD_GET_MOTOR_4_VEL:
            response = formatCommand(CMD_GET_MOTOR_4_VEL, String(steppers[3].speed()));
            break;
            
        case CMD_GET_MOTOR_5_VEL:
            response = formatCommand(CMD_GET_MOTOR_5_VEL, String(steppers[4].speed()));
            break;
            
        case CMD_GET_MOTOR_6_VEL:
            response = formatCommand(CMD_GET_MOTOR_6_VEL, String(steppers[5].speed()));
            break;
            
        case CMD_STOP_MOTOR_1:
            steppers[0].stop();
            response = formatCommand(CMD_STOP_MOTOR_1, "OK");
            break;
            
        case CMD_STOP_MOTOR_2:
            steppers[1].stop();
            response = formatCommand(CMD_STOP_MOTOR_2, "OK");
            break;
            
        case CMD_STOP_MOTOR_3:
            steppers[2].stop();
            response = formatCommand(CMD_STOP_MOTOR_3, "OK");
            break;
            
        case CMD_STOP_MOTOR_4:
            steppers[3].stop();
            response = formatCommand(CMD_STOP_MOTOR_4, "OK");
            break;
            
        case CMD_STOP_MOTOR_5:
            steppers[4].stop();
            response = formatCommand(CMD_STOP_MOTOR_5, "OK");
            break;
            
        case CMD_STOP_MOTOR_6:
            steppers[5].stop();
            response = formatCommand(CMD_STOP_MOTOR_6, "OK");
            break;
            
        case CMD_STOP_ALL:
            for(int i = 0; i < 6; i++) {
                steppers[i].stop();
            }
            response = formatCommand(CMD_STOP_ALL, "OK");
            break;
            
        case CMD_GET_BATTERY:
            response = formatCommand(CMD_GET_BATTERY, String(analogRead(A0)));
            break;
            
        case CMD_GET_SYSTEM:
            response = formatCommand(CMD_GET_SYSTEM, String(millis()));
            break;
            
        default:
            response = formatCommand(CMD_UNKNOWN, "ERR");
            break;
    }
    
    #ifdef I2C_SLAVE_DEVICE
    I2CSlave::setResponse(response.c_str());
    #endif
}

void ShelfbotComms::handleComms() {
    #ifdef I2C_SLAVE_DEVICE
    char* message = I2CSlave::getMessage();
    if(message != nullptr) {
        handleCommand(message);
    }
    #endif
}