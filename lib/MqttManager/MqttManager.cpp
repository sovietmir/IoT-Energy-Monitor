#include "MqttManager.h"

MqttManager::MqttManager(ConfigurationManager& configManager, Logger& logger): 
    _configManager(configManager), 
    _logger(logger),
    espClient(),  // Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
    client(espClient)
    {}

void MqttManager::begin(){
    //topicPrefix   = String(IoTclassName)+"/"+String(_configManager.getHostname());
    topicPrefix   = String(IoTclassName)+"/"+String(_configManager.getValue("wifi.hostname", ""));
    

    const char* server = _configManager.getValue("mqtt.broker", "");//_configManager.getMQTTBroker();
    int           port = _configManager.getValue("mqtt.port", 1883);//_configManager.getMQTTPort(); // 1883    
    client.setServer(server, port);  
      
}


void  MqttManager::loop(){ 
    if (!client.loop()) {
      const char* clientId = _configManager.getValue("mqtt.clientId", "Test_1");//_configManager.getMQTTClientId();
      const char* username = _configManager.getValue("mqtt.username", "");//_configManager.getMQTTUsername();
      const char* password = _configManager.getValue("mqtt.password", "");//_configManager.getMQTTPassword();
      client.connect(clientId, username, password);
    }
}


bool MqttManager::publish(String topic, String value) {
  if (client.connected()){
    reportStep(1);
    client.publish((topicPrefix +"/"+ topic).c_str(), value.c_str());
    reportStep(2);
    return true;
  }
  return false;
} 









void MqttManager::addReportStepHook(std::function<void(int)> func) {
  reportStepsHooks.push_back(func);
}
void MqttManager::reportStep(int step){
  for (auto& hook : reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}