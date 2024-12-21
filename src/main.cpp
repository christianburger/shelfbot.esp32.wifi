#include <Arduino.h>
#include <IPAddress.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <time.h>
#include <TZ.h>
#include <coredecls.h>

#include "ESP8266Logger.h"
#include "I2CSlave.h"

// SSID and password of Wifi connection:
const char* ssid = "dlink-30C0";
const char* password = "ypics98298";

//Webserver listening on port 80
ESP8266WebServer server(80);

//I2C device configured as slave
I2CSlave i2c; 

// simple function to decipher the encryption type of a network
String translateEncryptionType(uint8_t encryptionType) {
  switch (encryptionType) {
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_TKIP:
      return "WPA";
    case ENC_TYPE_CCMP:
      return "WPA2";
    case ENC_TYPE_NONE:
      return "Open";
    case ENC_TYPE_AUTO:
      return "WPA/WPA2";
    default:
      return "Unknown";
  }
}

// Function to scan and print all networks that can be detected by the ESP32 
void scanNetworks() {
  int numberOfNetworks = WiFi.scanNetworks();
 
  esp8266_logging::ESP8266Logger::logWiFi("Number of networks found: " + String(numberOfNetworks));
 
  for (int i = 0; i < numberOfNetworks; i++) {
    esp8266_logging::ESP8266Logger::logWiFi("Network name: " + WiFi.SSID(i));
    esp8266_logging::ESP8266Logger::logWiFi("Signal strength: " + String(WiFi.RSSI(i)));
    esp8266_logging::ESP8266Logger::logWiFi("MAC address: " + WiFi.BSSIDstr(i));
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    esp8266_logging::ESP8266Logger::logWiFi("Encryption type: " + encryptionTypeDescription);
    esp8266_logging::ESP8266Logger::logWiFi("-----------------------");
  }
}

// function to connect to a Wifi network -> note that there is no time-out function on this
void connectToNetwork() {
  WiFi.begin(ssid, password);
  esp8266_logging::ESP8266Logger::logWiFi("Establishing connection to WiFi.");
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    esp8266_logging::ESP8266Logger::logWiFi(".");
  }
  esp8266_logging::ESP8266Logger::logWiFi("Connected to network");
}

void handleRoot() {
    time_t now = time(nullptr);
    String timeString = ctime(&now);
    timeString.trim();  
    
    unsigned long rawTime = millis();
    unsigned long ms = rawTime % 1000;
    unsigned long seconds = (rawTime / 1000) % 60;
    unsigned long minutes = (rawTime / 60000) % 60;
    unsigned long hours = (rawTime / 3600000);
    
    String html = "<html><body>";
    html += "<h1>Shelfbot Status</h1>";
    html += "<div>Current Time: " + timeString + "</div>";
    html += "<div>Raw Uptime: " + String(rawTime) + " ms</div>";
    html += "<div>Uptime: " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s " + String(ms) + "ms</div>";
    html += "<div>WiFi SSID: " + WiFi.SSID() + "</div>";
    html += "<div>Signal Strength: " + String(WiFi.RSSI()) + " dBm</div>";
    html += "<div>IP Address: " + WiFi.localIP().toString() + "</div>";
    html += "<div>MAC Address: " + WiFi.macAddress() + "</div>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

void handleLog() {
    if (server.hasArg("clear")) {
        esp8266_logging::ESP8266Logger::clearLog();
        server.send(200, "text/plain", "Log cleared");
        return;
    }
    server.send(200, "text/html", esp8266_logging::ESP8266Logger::getLogHTML());
}

void handleMessage() {
    if (server.hasArg("text")) {
        String message = server.arg("text");
        esp8266_logging::ESP8266Logger::log(message);
        server.send(200, "text/plain", "Message logged: " + message);
    } else {
        server.send(400, "text/plain", "Missing text parameter");
    }
}

void setupMessageEndpoint() {
    server.on("/log", handleLog);
    server.on("/message", handleMessage);
}

void setupWebServer() {
    server.on("/", handleRoot);
    setupMessageEndpoint();
    server.begin();
    esp8266_logging::ESP8266Logger::logSystem("HTTP server started at http://" + WiFi.localIP().toString());
}

void handleWebServer() {
    server.handleClient();
}

void setupTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);
    tzset();
}

void initLogging() {
    esp8266_logging::ESP8266Logger::init();
    esp8266_logging::ESP8266Logger::logSystem("ESP8266 System startup");
}

void initI2C() {
    i2c.begin();
    esp8266_logging::ESP8266Logger::logI2C("I2C Slave initialized on SDA=" + String(2) + " SCL=" + String(4));
}

void initWiFi() {
    scanNetworks();
    connectToNetwork();
    
    esp8266_logging::ESP8266Logger::logWiFi("MAC Address: " + WiFi.macAddress());
    esp8266_logging::ESP8266Logger::logWiFi("IP Address: " + WiFi.localIP().toString());
}

void initWebServer() {
    setupTime();
    setupWebServer();
    esp8266_logging::ESP8266Logger::logSystem("Web server started at http://" + WiFi.localIP().toString());
}

void setup() {
    Serial.begin(115200);
    setupTime();
    initLogging();
    //i2c.testPins();
    initI2C();
    initWiFi();
    initWebServer();
}

void loop() {
    static unsigned long lastStatusPrint = 0;
    const unsigned long STATUS_INTERVAL = 5000;
    
    handleWebServer();
    
    if (i2c.hasNewData()) {
        byte receivedData = i2c.getLastReceivedData();
        esp8266_logging::ESP8266Logger::logI2C("Received: 0x" + String(receivedData, HEX) + " Wire status: 0x" + String(Wire.status(), HEX));
    }
    
    // Add periodic I2C status check
    if (millis() - lastStatusPrint >= STATUS_INTERVAL) {
        lastStatusPrint = millis();
        esp8266_logging::ESP8266Logger::logI2C("Bus status - Available: " + String(Wire.available()) + " Status: 0x" + String(Wire.status(), HEX));
        esp8266_logging::ESP8266Logger::logSystem("Status - RSSI: " + String(WiFi.RSSI()) + "dBm, I2C Status: " + String(Wire.status()));
    }
}