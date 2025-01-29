#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <vector>
#include "common.h"
#include "ConfigurationManager.h"
#include "HTTPServerManager.h"
#include "Logger.h"
#include "LEDHandler.h"


class WiFiManager {
public:
    WiFiManager(ConfigurationManager& configManager, HTTPServerManager& serverManager, Logger& logger);

    void begin();
    void registerEndpoints();

    bool firstConnectToAP();
    bool reConnectoAP();
    bool connectToAP();    
    void setAPcredentials(const char* ssid, const char* password);
    void createAP();
    void reboot();
    
    // Add a hook to the list of hooks
    void addReportStepHook(std::function<void(int)> func);
    void reportStep(int step);

    void handleScanAPs(ESP8266WebServer& server);

private:
    ConfigurationManager& _configManager;
    HTTPServerManager& _serverManager;
    Logger& _logger;

    // Hardcoded AP credentials
    const char* _apSSID;
    const char* _apPassword;

    bool APstarted = false;

    std::vector<std::function<void(int)>> reportStepsHooks; // List of hooks
};

#endif