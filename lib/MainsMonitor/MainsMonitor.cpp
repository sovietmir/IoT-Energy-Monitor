#include "MainsMonitor.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

MainsMonitor::MainsMonitor(HTTPServerManager& serverManager, Logger* logger) 
        : Sensor<mainsMetrics>("PZEM004Tv30", serverManager, logger),//_serverManager(serverManager),
          //_logger(logger), 
          _pzemSWSerial(RX_PIN, TX_PIN), // Initialize SoftwareSerial
          _pzem(_pzemSWSerial)
          {}

void MainsMonitor::begin(String URL, String deviceName, int periodicity) {
    _pzem.resetEnergy();
    _URL = URL;
    _deviceName = deviceName;
    _periodicity = periodicity;
    if (_logger != nullptr) {
        _logger->log("Informer.begin() URL: "+_URL+"\n");
        _logger->log("Informer.begin() deviceName: "+_deviceName+"\n");
    }
    Sensor::begin(); // call parent to register endpoints
    //registerEndpoints();
}

void MainsMonitor::loop(){
    long currentTime = millis();
    if (currentTime - _lastTime > _periodicity) {
        _lastTime = currentTime;
        reportStep(1); // digitalWrite(INDICATOR, HIGH);
        _lastMetrics = read();
        //logMetrics(_lastMetrics);
        reportMetrics(_lastMetrics);
        if(checkMetrics(_lastMetrics)){
          if(!makeHTTPGETRequest(_lastMetrics)){
            // TODO: write to file
            reportStep(-1);             
          }
          broadcastMetrics(_lastMetrics);
          reportStep(0);
        } else {
            reportStep(-4);             
        }
        reportStep(2);        
    }    
}
mainsMetrics MainsMonitor::read(){
    mainsMetrics toret;
    toret.device = String(_deviceName);
    toret.time = Logger::timeToString();
    
    int i=0;
    while(_pzem.readAddress()==0 && i++<5){
        if (_logger != nullptr) _logger->logf("MainsMonitor.read() waiting for PZEM's address...\n");
        delay(50);
    }
    //if (_logger != nullptr) _logger->logf("Custom Address: %s\n", String(_pzem.readAddress(), HEX));
    //delay(10);

    toret.voltage =     _pzem.voltage();
    toret.current =     _pzem.current();
    toret.power =       _pzem.power();
    toret.energy =      _pzem.energy();
    toret.frequency =   _pzem.frequency();
    toret.powerfactor = _pzem.pf();
    return toret;
}
void MainsMonitor::logMetrics(const mainsMetrics& metrics){
    if(checkMetrics(metrics) && _logger != nullptr){
        _logger->log("Time: "+          metrics.time+"\n");
        _logger->log("\tVoltage: "+     String(metrics.voltage, 2)+" V\n");
        _logger->log("\tCurrent: "+     String(metrics.current, 2)+" A\n");
        _logger->log("\tPower: "+       String(metrics.power, 2)+" W\n");
        _logger->log("\tEnergy: "+      String(metrics.energy, 3)+" kWh\n");
        _logger->log("\tFrequency: "+   String(metrics.frequency, 1)+" Hz\n");
        _logger->log("\tPowerfactor: "+ String(metrics.powerfactor, 2)+"\n");
    }
}
bool MainsMonitor::makeHTTPGETRequest(mainsMetrics& metrics){
    if (_URL=="") {
        return false;
    }
    bool toret = true;
    WiFiClient client;
    HTTPClient http;

    String data = this->serializeMetrics(metrics);    // Serialize structure to JSON
    String encodedData = urlencode(data); // URL encode the JSON string

    String URL = _URL + "?action=monitor&data="+encodedData+"&ip="+WiFi.localIP().toString();

    //if (_logger != nullptr) _logger->logf("Request URL: %s\n", URL.c_str());
    http.begin(client, URL); // Initialize HTTP client
    int httpCode = http.GET(); // Send HTTP GET request

    if (httpCode > 0) { // Check the returning code
        String payload = http.getString(); // Get response payload
        if (_logger != nullptr){
            _logger->logf("HTTP GET request response code: %d\n", httpCode);
            _logger->log("Response: ");
            _logger->log(payload+"\n");
        }
    } else {
        if (_logger != nullptr) _logger->logf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        toret = false;
    }

    http.end(); // Free resources
    return toret;
}
String MainsMonitor::urlencode(const String& str) {
    String encoded = "";
    char c;
    for (size_t i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;  // Keep alphanumeric and some other characters unencoded
        } else {
            //encoded += "%" + String(c, HEX);  // Encode other characters as hex
            char buf[4];
            sprintf(buf, "%%%02X", c); // always 2-digit uppercase hex
            encoded += buf;
        }
    }
    return encoded;
}
