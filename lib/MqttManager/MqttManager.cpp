#include "MqttManager.h"

MqttManager::MqttManager(ConfigurationManager& configManager, Logger& logger): 
    _configManager(configManager), 
    _logger(logger),
    espClient(),  // Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
    client(espClient)
    {}

void MqttManager::setLEDs(LEDHandler* normalLED, LEDHandler* errorLED) {
  normalLED = normalLED;
  errorLED = errorLED;
}

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
    client.publish((topicPrefix +"/"+ topic).c_str(), value.c_str());
    return true;
  }
  return false;
} 



















// Helper functions
void MqttManager::normalLEDon() {
  if (normalLED != nullptr) {
    normalLED->on(); 
  }
}
void MqttManager::normalLEDoff() {
  if (normalLED != nullptr) {
    normalLED->off(); 
  }
}
void MqttManager::errorLEDMode(LEDMode mode) {
  if (errorLED != nullptr) {
    errorLED->setMode(mode); 
  }
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