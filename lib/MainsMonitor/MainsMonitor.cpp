#include "MainsMonitor.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

MainsMonitor::MainsMonitor(HTTPServerManager& serverManager, Logger* logger) 
        : _serverManager(serverManager),
          _logger(logger), 
          _pzemSWSerial(RX_PIN, TX_PIN), // Initialize SoftwareSerial
          _pzem(_pzemSWSerial)
          {}

void MainsMonitor::begin(const char* URL, const char* deviceName, int periodicity) {
    _pzem.resetEnergy();
    _URL = URL;
    _deviceName = deviceName;
    _periodicity = periodicity;
    if (_logger != nullptr) {
        _logger->logf("MainsMonitor.begin() URL: %s\n", _URL);
        _logger->logf("MainsMonitor.begin() deviceName: %s\n", _deviceName);
    }
    registerEndpoints();
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

bool MainsMonitor::checkMetrics(mainsMetrics& metrics){
    if(isnan(metrics.voltage)){
        if (_logger != nullptr) _logger->log("Error reading voltage\n");
        return false;
    }
    if (isnan(metrics.current)) {
        if (_logger != nullptr) _logger->log("Error reading current\n");
        return false;
    } 
    if (isnan(metrics.power)) {
        if (_logger != nullptr) _logger->log("Error reading power\n");
        return false;
    }
    if (isnan(metrics.energy)) {
        if (_logger != nullptr) _logger->log("Error reading energy\n");
        return false;
    }
    if (isnan(metrics.frequency)) {
        if (_logger != nullptr) _logger->log("Error reading frequency\n");
        return false;
    }
    if (isnan(metrics.powerfactor)) {
        if (_logger != nullptr) _logger->log("Error reading power factor\n");
        return false;
    }
    return true;
}

void MainsMonitor::logMetrics(mainsMetrics& metrics){
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
    if (_URL[0] == '\0') {
        return false;
    }
    bool toret = true;
    WiFiClient client;
    HTTPClient http;

    String data = serializeMetrics(metrics);    // Serialize structure to JSON
    String encodedData = urlencode(data); // URL encode the JSON string

    String URL = String(_URL) + "?action=monitor&data="+encodedData+"&ip="+WiFi.localIP().toString();

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

String  MainsMonitor::serializeMetrics(const mainsMetrics& metrics) {
    JsonDocument doc; // Create a JSON document to hold the data
    doc["device"] = _deviceName;

    doc["time"] = metrics.time;
    doc["voltage"] = metrics.voltage;
    doc["current"] = metrics.current;
    doc["power"] = metrics.power;
    doc["energy"] = metrics.energy;
    doc["frequency"] = metrics.frequency;
    doc["powerfactor"] = metrics.powerfactor;

    String jsonString;
    serializeJson(doc, jsonString);  // Serialize the JSON document to a string
    return jsonString;
}

String MainsMonitor::urlencode(const String& str) {
    String encoded = "";
    char c;
    for (size_t i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;  // Keep alphanumeric and some other characters unencoded
        } else {
            encoded += "%" + String(c, HEX);  // Encode other characters as hex
        }
    }
    return encoded;
}

void MainsMonitor::addReportStepHook(std::function<void(int)> func) {
  _reportStepsHooks.push_back(func);
}

void MainsMonitor::reportStep(int step){
  for (auto& hook : _reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}

void MainsMonitor::addReportMetricsHook(std::function<void(mainsMetrics&)> func) {
  _reportMetricsHooks.push_back(func);
}

void MainsMonitor::reportMetrics(mainsMetrics& metric){
  for (auto& hook : _reportMetricsHooks) {
    if (hook) {
        hook(metric);
    }
  }
}

void MainsMonitor::registerEndpoints() {
    _serverManager.registerPage("/api/getLastMetrics", HTTP_GET, [this](ESP8266WebServer& server) {
        server.send(200, "application/json", serializeMetrics(_lastMetrics));
    });

    _serverManager.registerPage("/api/getPeriodicity", HTTP_GET, [this](ESP8266WebServer& server) {
        server.send(200, "application/json", "{\"periodicity\":\""+String(_periodicity)+"\"}");
    });

    _serverManager.registerPage("/api/setPeriodicity", HTTP_POST, [this](ESP8266WebServer& server) {
        handleSetPeriodicityRequest(server);
    });
    
}

void MainsMonitor::broadcastMetrics(mainsMetrics& metrics){
    _serverManager.broadcastWebSocketMessage(serializeMetrics(metrics));
};

void MainsMonitor::handleSetPeriodicityRequest(ESP8266WebServer& server) {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"status\": -1, \"error\":\"Bad Request\"}");
        return;
    }
    
    String json = server.arg("plain");
    JsonDocument params;
    DeserializationError error = deserializeJson(params, json);
    if(error) {
        server.send(500, "application/json", "{\"status\": -2, \"error\":\"Failed to deserializeJson the request data\"}");
        return;
    }

    _periodicity = params["periodicity"];
    if (_logger != nullptr) _logger->logf("handleSetPeriodicityRequest %d\n", _periodicity);
    server.send(200, "application/json", "{\"status\": 1, \"message\":\"Periodicity is set\"}");
   
}