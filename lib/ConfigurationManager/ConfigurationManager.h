#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <ArduinoJson.h>
#include <LittleFS.h>
#include "HTTPServerManager.h"
#include "Logger.h"

class ConfigurationManager {
public:
    ConfigurationManager(const char* configFilePath, const char* prefix, HTTPServerManager& serverManager, Logger& logger);

    bool loadConfig();
    bool saveConfig();
    void resetConfig();

    const char* getValue(const char* path, const char* defaultValue);
    int getValue(const char* path, int defaultValue);
    
    void setConfig(const JsonObject& newConfig);
    JsonDocument getConfig();

    void begin();
    void registerEndpoints();
    void handleConfigPost(ESP8266WebServer& server);
    void handleGetCurrentConfig(ESP8266WebServer& server);
private:
    const char* _configFilePath;
    const char* _prefix;
    HTTPServerManager& _serverManager;
    Logger& _logger;
    JsonDocument _config;
};

#endif