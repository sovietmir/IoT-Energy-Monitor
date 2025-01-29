#include "AHT10Monitor.h"

AHT10Monitor::AHT10Monitor(HTTPServerManager& serverManager, Logger& logger) 
        : _serverManager(serverManager),
          _logger(logger), 
          myAHT10(AHT10_ADDRESS_0X38)
          {}

void AHT10Monitor::begin() {
    myAHT10.begin(SDA_PIN, SCL_PIN);
}

void AHT10Monitor::loop(){
    currentTime = millis();
    if (currentTime - lastTime > periodicity) {
        lastTime = currentTime;
        reportStep(1); 
        lastMetrics = read();
        logMetrics(lastMetrics);
        reportMetrics(lastMetrics);
        broadcastMetrics(lastMetrics);
        reportStep(2);        
    }    
}

aht10Metrics AHT10Monitor::read(){
    aht10Metrics toret;
    toret.time = String(DateTime.format(DateFormatter::SIMPLE).c_str());

    toret.temperature = myAHT10.readTemperature();
    toret.humidity = myAHT10.readHumidity();
    return toret;
}


void AHT10Monitor::logMetrics(aht10Metrics& metrics){
    _logger.log("Time: "+  metrics.time+"\n");
    _logger.log("\tTemperature: "+  String(metrics.temperature, 2)+" +-0.3C\n");
    _logger.log("\tHumidity   : "+  String(metrics.humidity, 2)+" +-2%\n");
}


// Function to serialize the structure into JSON
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
  reportStepsHooks.push_back(func);
}
void AHT10Monitor::reportStep(int step){
  for (auto& hook : reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}
void AHT10Monitor::addReportMetricsHook(std::function<void(aht10Metrics&)> func) {
  reportMetricsHooks.push_back(func);
}
void AHT10Monitor::reportMetrics(aht10Metrics& metric){
  for (auto& hook : reportMetricsHooks) {
    if (hook) {
        hook(metric);
    }
  }
}



void AHT10Monitor::broadcastMetrics(aht10Metrics& metrics){
    _serverManager.broadcastWebSocketMessage(serializeMetrics(metrics));
};

