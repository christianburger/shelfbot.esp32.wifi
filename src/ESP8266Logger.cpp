#include "ESP8266Logger.h"

namespace esp8266_logging {

std::vector<String> ESP8266Logger::logEntries(MAX_LOG_ENTRIES);
int ESP8266Logger::logIndex = 0;

void ESP8266Logger::init() {
    logEntries.clear();
    logIndex = 0;
    log("ESP8266 Logger initialized");
}

String ESP8266Logger::getTimeStamp() {
    time_t now = time(nullptr);
    String timeString = ctime(&now);
    timeString.trim();
    return timeString;
}

void ESP8266Logger::log(const String& message) {
    String entry = getTimeStamp() + " - " + message;
    logEntries[logIndex] = entry;
    logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
    Serial.println(entry);
}

void ESP8266Logger::logI2C(const String& message) {
    log("I2C: " + message);
}

void ESP8266Logger::logWiFi(const String& message) {
    log("WiFi: " + message);
}

void ESP8266Logger::logSystem(const String& message) {
    log("System: " + message);
}

String ESP8266Logger::getLogHTML() {
    String html = "<html><body><h1>ESP8266 System Log</h1><pre>\n";
    
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        int idx = (logIndex + i) % MAX_LOG_ENTRIES;
        if (logEntries[idx].length() > 0) {
            html += logEntries[idx] + "\n";
        }
    }
    
    html += "</pre></body></html>";
    return html;
}

/*
String ESP8266Logger::getTimeStamp() {
    time_t now = time(nullptr);
    String timeStr = ctime(&now);
    timeStr.trim();
    return timeStr;
}
*/

void ESP8266Logger::clearLog() {
    logEntries.clear();
    logIndex = 0;
    log("Log cleared");
}

}