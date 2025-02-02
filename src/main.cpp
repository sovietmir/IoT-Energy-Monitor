#include <Arduino.h>
#include "common.h"
#include "ConfigurationManager.h"
#include "ConsoleLogger.h"
#include "TelnetLogger.h"
#include "WiFiManager.h"
#include "HTTPServerManager.h"
#include "OTA.h"
#include "MqttManager.h"

#include "LEDHandler.h"
#include "MainsMonitor.h"
#include "AHT10Monitor.h"

//ConsoleLogger      logger;
TelnetLogger         logger;
HTTPServerManager    httpServerManager(&logger);
WiFiManager          wifiManager(httpServerManager, &logger);
OTA                  otaManager(httpServerManager, &logger);
MqttManager          mqttManager(&logger);

ConfigurationManager configManager("/config.json",  "conf", httpServerManager, logger);
ConfigurationManager defaultManager("/default.json","dflt", httpServerManager, logger);


static LEDHandler    normalLED("Normal (green) indicator", &logger);
static LEDHandler    errorLED("Error (red) indicator", &logger);
MainsMonitor         mainsMonitor(httpServerManager, &logger);
AHT10Monitor         aht10Monitor(httpServerManager, &logger);

void setup() {
  // Conditional compilation for pin configuration
  #if defined(INDICATOR)
    normalLED.begin(INDICATOR);  // This will set _pin inside LEDHandler and  pinMode(INDICATOR, OUTPUT); 
    normalLED.on();
  #endif
  #if defined(ERROR_INDICATOR)
    errorLED.begin(ERROR_INDICATOR);// This will set _pin inside LEDHandler and pinMode(ERROR_INDICATOR, OUTPUT); 
    errorLED.off();
  #endif
  // Assign a lambda function to variable "reportStep"
  auto reportStep = [&](int step){
    if(step==0) { normalLED.setMode(NONE); errorLED.setMode(NONE);}
    if(step==1 || step==3)       normalLED.on();
    else if(step==2 || step==4)  normalLED.off();
    else if(step<0) {
      if(step==-2)      errorLED.setMode(BLINK2TIMES);
      else if(step==-3) errorLED.setMode(BLINK3TIMES);
      else if(step==-4) errorLED.setMode(BLINKFAST);
      else errorLED.setMode(BLINKONCE);      
    }             
  };
  // Use the lambda function assinged to variable "reportStep" as a function hook in the following objects:
  wifiManager.addReportStepHook(reportStep);
  mainsMonitor.addReportStepHook(reportStep);
  otaManager.addReportStepHook(reportStep);
  mqttManager.addReportStepHook(reportStep);

  mainsMonitor.addReportMetricsHook([&](mainsMetrics& metrics){
    mqttManager.publish("voltage", String(metrics.voltage, 0).c_str());
    mqttManager.publish("current", String(metrics.current, 2).c_str());
    mqttManager.publish("power",   String(metrics.power, 2).c_str());
    mqttManager.publish("energy",  String(metrics.energy, 3).c_str());
    mqttManager.publish("frequency", String(metrics.frequency, 0).c_str());
    mqttManager.publish("powerfactor", String(metrics.powerfactor, 2).c_str());
  });  
  aht10Monitor.addReportMetricsHook([&](aht10Metrics& metrics){
    mqttManager.publish("temperature", String(metrics.temperature, 2).c_str());
    mqttManager.publish("humidity", String(metrics.humidity, 2).c_str());    
  }); 
 
  Serial.begin(115200);
  logger.begin();
  logger.log("Start\n");


 

  if (!configManager.loadConfig()) {
    logger.log("Failed to load configuration.\n");
    configManager.resetConfig();
    errorLED.setMode(BLINKONCE); 
  }
  wifiManager.setSSID(configManager.getValue("wifi.ssid", ""));
  wifiManager.setPassword(configManager.getValue("wifi.password", ""));
  wifiManager.setHostname(configManager.getValue("wifi.hostname", ""));
  wifiManager.setTimeServer(configManager.getValue("time.server", "pool.ntp.org"));
  wifiManager.setTimeZone(configManager.getValue("time.zone", ""));
  
  defaultManager.loadConfig();
  wifiManager.setAPSSID(configManager.getValue("wifiAP.ssid", "Energy_Monitor_IoT"));
  wifiManager.setAPPassword(configManager.getValue("wifiAP.password", ""));

  mqttManager.setTopic(configManager.getValue("wifi.hostname", ""));
  mqttManager.setServer(configManager.getValue("mqtt.broker", ""));
  mqttManager.setPort(configManager.getValue("mqtt.port", 1883));
  mqttManager.setClientId(configManager.getValue("mqtt.clientId", "Test_1"));
  mqttManager.setUsername(configManager.getValue("mqtt.username", ""));
  mqttManager.setPassword(configManager.getValue("mqtt.password", ""));

  configManager.begin();  // i.e., registerEndpoints()
  defaultManager.begin(); // i.e., registerEndpoints()
  wifiManager.begin();    // i.e.: try to connect to defined (in config.json) AP, if successfull then set current datetime, registerEndpoints()
  otaManager.begin();     // i.e., registerEndpoints()
  httpServerManager.begin();
  mqttManager.begin();

  mainsMonitor.begin(configManager.getValue("http.entryPointUrl", ""), configManager.getValue("wifi.hostname", ""), configManager.getValue("device.periodicity", 10000));
  aht10Monitor.begin();

  normalLED.setMode(BLINKONCE);

}

void loop() {
  // put your main code here, to run repeatedly:
  logger.loop();
  normalLED.loop();
  errorLED.loop();

  wifiManager.loop();

  httpServerManager.loop();

  mqttManager.loop();

  mainsMonitor.loop();

  aht10Monitor.loop();
}

