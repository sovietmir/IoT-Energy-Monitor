#include "AHT10Monitor.h"

AHT10Monitor::AHT10Monitor(HTTPServerManager& serverManager, Logger* logger) 
        : Sensor<aht10Metrics>("AHT10", serverManager, logger),
          _AHT10(AHT10_ADDRESS_0X38)
          {}

void AHT10Monitor::begin() {
    _AHT10.begin(SDA_PIN, SCL_PIN);
    Sensor::begin(); // call parent to register endpoints
    this->_lastTime = -1000000;
}

const int NUM_READINGS = 5;
const unsigned long READ_DELAY_MS = 20;

aht10Metrics AHT10Monitor::read(){
    _logger->log("AHT10Monitor::read() \n");
    aht10Metrics toret;


    float temperatureSum = 0;
    float humiditySum = 0;
    int validCount = 0;
    
    for (int i = 0; i < NUM_READINGS; i++) {
      float temperature = _AHT10.readTemperature();
      float humidity = _AHT10.readHumidity();
      // Validate readings
      bool temperatureValid = !isnan(temperature) && temperature!=AHT10_ERROR && temperature > -40 && temperature < 100;
      bool humidityValid    = !isnan(humidity)  && humidity!=AHT10_ERROR      && humidity >= 0     && humidity <= 100;
      if (temperatureValid && humidityValid) {
        temperatureSum += temperature;
        humiditySum    += humidity;
        validCount++;
      }
      delay(READ_DELAY_MS);
    }

    if(validCount>0){
      toret.temperature = temperatureSum / validCount;
      toret.humidity    = humiditySum / validCount;
    }
    else {
      toret.temperature = _AHT10.readTemperature();
      toret.humidity = _AHT10.readHumidity();
      if(toret.temperature==AHT10_ERROR) toret.temperature = NAN;
      if(toret.humidity==AHT10_ERROR)    toret.humidity = NAN;
    }
    
    toret.time = Logger::timeToString();
    
    return toret;
}

void AHT10Monitor::logMetrics(const aht10Metrics& metrics){
    if (_logger != nullptr) {
      _logger->log("Sensor: "+_sensorName+". Time: "+  metrics.time+"\n");
      _logger->log("\tTemperature: "+  String(metrics.temperature, 2)+" +-0.3C\n");
      _logger->log("\tHumidity   : "+  String(metrics.humidity, 2)+" +-2%\n");
    }
}
