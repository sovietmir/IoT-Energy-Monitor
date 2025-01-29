#ifndef AHT10_MONITOR_H
#define AHT10_MONITOR_H

#include <AHT10.h>
#include "common.h"
#include "HTTPServerManager.h"
#include "Logger.h"
#include "LEDHandler.h"
#include <ESPDateTime.h>
#include <ArduinoJson.h>
#include <vector>

#if !defined(SDA_PIN)
  #define SDA_PIN 5 
#endif
#if !defined(SCL_PIN)
  #define SCL_PIN 4 
#endif

struct aht10Metrics {
    String time;
    float temperature;
    float humidity;
};


class AHT10Monitor {
public:
    AHT10Monitor(HTTPServerManager& serverManager, Logger& logger);

    void begin();

    void loop();
    aht10Metrics read();
    void logMetrics(aht10Metrics& metric);
    void broadcastMetrics(aht10Metrics& metrics);

    // Add a hook to the list of hooks
    void addReportStepHook(std::function<void(int)> func);
    void reportStep(int step);
    void addReportMetricsHook(std::function<void(aht10Metrics&)> func);
    void reportMetrics(aht10Metrics& metric);



    void handleSetPeriodicityRequest(ESP8266WebServer& server);
private:
    HTTPServerManager& _serverManager;
    Logger& _logger;
    AHT10 myAHT10;

    long currentTime, lastTime = 0;
    long periodicity = 10000; // In ms

    aht10Metrics lastMetrics;

    String serializeMetrics(const aht10Metrics& metrics);


    std::vector<std::function<void(int)>> reportStepsHooks; // List of hooks
    std::vector<std::function<void(aht10Metrics&)>> reportMetricsHooks; // List of hooks
};

#endif