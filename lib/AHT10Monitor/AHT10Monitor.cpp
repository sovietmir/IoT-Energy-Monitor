#include "AHT10Monitor.h"

AHT10Monitor::AHT10Monitor(HTTPServerManager& serverManager, Logger* logger) 
        : _serverManager(serverManager),
          _logger(logger), 
          _AHT10(AHT10_ADDRESS_0X38)
          {}

void AHT10Monitor::begin() {
    _AHT10.begin(SDA_PIN, SCL_PIN);
}

void AHT10Monitor::loop(){
    long currentTime = millis();
    if (currentTime - _lastTime > _periodicity) {
        _lastTime = currentTime;
        reportStep(1); 
        _lastMetrics = read();
        logMetrics(_lastMetrics);
        reportMetrics(_lastMetrics);
        broadcastMetrics(_lastMetrics);
        reportStep(2);        
    }    
}

aht10Metrics AHT10Monitor::read(){
    aht10Metrics toret;
    toret.time = Logger::timeToString();

    toret.temperature = _AHT10.readTemperature();
    toret.humidity = _AHT10.readHumidity();
    return toret;
}

void AHT10Monitor::logMetrics(aht10Metrics& metrics){
    if (_logger != nullptr) {
      _logger->log("Time: "+  metrics.time+"\n");
      _logger->log("\tTemperature: "+  String(metrics.temperature, 2)+" +-0.3C\n");
      _logger->log("\tHumidity   : "+  String(metrics.humidity, 2)+" +-2%\n");
    }
}

String  AHT10Monitor::serializeMetrics(const aht10Metrics& metrics) {
    JsonDocument doc; // Create a JSON document to hold the data
    doc["time"] = metrics.time;
    doc["temperature"] = metrics.temperature;
    doc["humidity"] = metrics.humidity;

    String jsonString;
    serializeJson(doc, jsonString);  // Serialize the JSON document to a string
    return jsonString;
}

void AHT10Monitor::addReportStepHook(std::function<void(int)> func) {
  _reportStepsHooks.push_back(func);
}

void AHT10Monitor::reportStep(int step){
  for (auto& hook : _reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}

void AHT10Monitor::addReportMetricsHook(std::function<void(aht10Metrics&)> func) {
  _reportMetricsHooks.push_back(func);
}

void AHT10Monitor::reportMetrics(aht10Metrics& metric){
  for (auto& hook : _reportMetricsHooks) {
    if (hook) {
        hook(metric);
    }
  }
}

void AHT10Monitor::registerEndpoints() {
    _serverManager.registerPage("/api/getLastMetricsAHT10", HTTP_GET, [this](ESP8266WebServer& server) {
        server.send(200, "application/json", serializeMetrics(_lastMetrics));
    });
    
}

void AHT10Monitor::broadcastMetrics(aht10Metrics& metrics){
    _serverManager.broadcastWebSocketMessage(serializeMetrics(metrics));
};

