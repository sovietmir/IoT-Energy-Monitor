#ifndef WQTT_MANAGER_H
#define WQTT_MANAGER_H

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Stream.h>
#include <vector>
#include "common.h"
#include "ConfigurationManager.h"
#include "Logger.h"
#include "LEDHandler.h"


class MqttManager {
public:
    MqttManager(ConfigurationManager& configManager, Logger& logger);
   
    void begin();
    void loop();
    bool publish(String topic, String value);
    
    // Add a hook to the list of hooks
    void addReportStepHook(std::function<void(int)> func);
    void reportStep(int step);

private:
    ConfigurationManager& _configManager;
    Logger& _logger;

    WiFiClient espClient;
    PubSubClient client;

    String topicPrefix = "EnergyMonitor";

    std::vector<std::function<void(int)>> reportStepsHooks; // List of hooks
};

#endif