#ifndef OTA_H
#define OTA_H

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <Updater.h>
#include "HTTPServerManager.h"
#include "Logger.h"

class OTA {
public:
    OTA(HTTPServerManager& serverManager, Logger& logger);
    void begin();
    void registerEndpoints();

    // Add a hook to the list of hooks
    void addReportStepHook(std::function<void(int)> func);
    void reportStep(int step);

private:
    HTTPServerManager& _serverManager;
    Logger& _logger;

    void handleFirmwareUpload(ESP8266WebServer& server);
    void handleFileUpload(ESP8266WebServer& server);
    void handleDirectoryList(ESP8266WebServer& server);
    void handleFileSystemRequest(ESP8266WebServer& server);
    void handleDownloadRequest(ESP8266WebServer& server);
    void handleDeleteRequest(ESP8266WebServer& server);
    void handleAddDirectoryRequest(ESP8266WebServer& server);

    void listFilesRecursive(const String& dirPath, JsonArray& filesArray);


    std::vector<std::function<void(int)>> reportStepsHooks; // List of hooks
};

#endif
