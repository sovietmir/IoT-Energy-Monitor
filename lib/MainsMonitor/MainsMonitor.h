#ifndef MAINS_MONITOR_H
#define MAINS_MONITOR_H

#include <PZEM004Tv30.h>
#include "common.h"
#include "HTTPServerManager.h"
#include "Logger.h"
#include "LEDHandler.h"
#include <ESPDateTime.h>
#include <ArduinoJson.h>
#include <vector>

#if !defined(RX_PIN)
  #define RX_PIN 12 // Connect to PZEM's TX
#endif
#if !defined(TX_PIN)
  #define TX_PIN 13 // Connect to PZEM's RX
#endif

struct mainsMetrics {
    String time;
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float powerfactor;
};


class MainsMonitor {
public:
    MainsMonitor(HTTPServerManager& serverManager, Logger& logger);

    void begin(const char* URL, const char* deviceName);
    void registerEndpoints();

    void loop();
    mainsMetrics read();
    bool checkMetrics(mainsMetrics& metric);
    void logMetrics(mainsMetrics& metric);
    bool makeHTTPGETRequest(mainsMetrics& metrics);
    void broadcastMetrics(mainsMetrics& metrics);

    // Add a hook to the list of hooks
    void addReportStepHook(std::function<void(int)> func);
    void reportStep(int step);
    void addReportMetricsHook(std::function<void(mainsMetrics&)> func);
    void reportMetrics(mainsMetrics& metric);



    void handleSetPeriodicityRequest(ESP8266WebServer& server);
private:
    HTTPServerManager& _serverManager;
    Logger& _logger;
    SoftwareSerial _pzemSWSerial; // Member to hold SoftwareSerial
    PZEM004Tv30 pzem;

    const char* _URL;
    const char* _deviceName;

    long currentTime, lastTime = 0;
    long periodicity = 10000; // In ms

    mainsMetrics lastMetrics;

    String serializeMetrics(const mainsMetrics& metrics);
    String urlencode(const String& str);


    std::vector<std::function<void(int)>> reportStepsHooks; // List of hooks
    std::vector<std::function<void(mainsMetrics&)>> reportMetricsHooks; // List of hooks
};

#endif