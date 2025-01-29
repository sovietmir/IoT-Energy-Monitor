#include "WiFiManager.h"

WiFiManager::WiFiManager(ConfigurationManager& configManager, HTTPServerManager& serverManager, Logger& logger) : _configManager(configManager), _serverManager(serverManager), _logger(logger) {}


bool WiFiManager::firstConnectToAP() {
  const char* hostname = _configManager.getValue("wifi.hostname", ""); 
  if (strlen(hostname) > 0) {
    WiFi.hostname(hostname);
  }
  return this->connectToAP();
}
bool WiFiManager::reConnectoAP() {
  if (WiFi.status() != WL_CONNECTED) {  // Check if Wi-Fi is disconnected
    _logger.log("WiFi disconnected.\n");
    
    WiFi.disconnect();  // Ensure a clean start for reconnection
    return this->connectToAP();
  }
  return true;
}
bool WiFiManager::connectToAP() {
    const char* ssid     = _configManager.getValue("wifi.ssid", "");
    const char* password = _configManager.getValue("wifi.password", "");
    

    if (strlen(ssid) == 0 || strlen(password) == 0) {
        _logger.log("WiFi credentials are missing.\n");
        reportStep(-1);
        return false;
    }

    _logger.log("Connecting to WiFi.\n");
    _logger.logf("SSID: %s\n", ssid);


    reportStep(1);
    WiFi.begin(ssid, password);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        reportStep(2);
        if (millis() - startTime > 10000) {            
            _logger.log("Failed to connect to WiFi.\n");
            reportStep(-2);
            return false;
        }
        delay(200);
        reportStep(3);
        delay(100);
        _logger.log(".");        
    }
    _logger.log("\n");
    reportStep(4); 

    _logger.logf("Connected to WiFi! IP Address: %s\n", WiFi.localIP().toString().c_str());
    return true;
}


void WiFiManager::setAPcredentials(const char* ssid, const char* password){
  _apSSID = ssid;
  _apPassword = password;
};
void WiFiManager::createAP() {
  if(!APstarted){
    _logger.logf("Creating AP with SSID: %s\n", _apSSID);

    APstarted = WiFi.softAP(_apSSID, _apPassword);

    _logger.logf("AP IP Address: : %s\n", WiFi.softAPIP().toString().c_str());
  }
}


void WiFiManager::reboot() {
    // Reboot the ESP8266
    _logger.log("Rebooting...\n");
    ESP.restart();  // This will reset the ESP8266
}



void WiFiManager::addReportStepHook(std::function<void(int)> func) {
  reportStepsHooks.push_back(func);
}
void WiFiManager::reportStep(int step){
  for (auto& hook : reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}





void WiFiManager::begin() {
  registerEndpoints();
}
void WiFiManager::registerEndpoints() {
    // Register configuration backend calls handling
    _serverManager.registerPage("/api/nearby-ap", HTTP_GET, [this](ESP8266WebServer& server) { handleScanAPs(server); });
}

void WiFiManager::handleScanAPs(ESP8266WebServer& server) {
    int n = WiFi.scanNetworks();  // Perform Wi-Fi scan
    String response = "[";
    for (int i = 0; i < n; ++i) {
        response += "\"" + WiFi.SSID(i) + "\"";
        if (i < n - 1) response += ",";
    }
    response += "]";

    server.send(200, "application/json", response);
}