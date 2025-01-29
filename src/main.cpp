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

#include <ESPDateTime.h>


//#include <AHT10.h>
//AHT10 myAHT10(AHT10_ADDRESS_0X38);


enum OperationMode {
    INIT,
    NORMAL,
    SETTINGS
};
OperationMode operationMode = INIT;



//ConsoleLogger logger;
TelnetLogger logger;
HTTPServerManager httpServerManager(logger);
ConfigurationManager configManager("/config.json",  "conf", httpServerManager, logger);
ConfigurationManager defaultManager("/default.json","dflt", httpServerManager, logger);
WiFiManager wifiManager(configManager, httpServerManager, logger);
MqttManager mqttManager(configManager, logger);
OTA otaManager(httpServerManager, logger);
static LEDHandler normalLED("Normal (green) indicator", logger);
static LEDHandler errorLED("Error (red) indicator", logger);
MainsMonitor mainsMonitor(httpServerManager, logger);
AHT10Monitor aht10Monitor(httpServerManager, logger);
 




long unsigned currentTime = millis();
long unsigned lastTimeWifiCheck = 0;
long unsigned lastTime = 0;



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
  // Assign a lambda function to a variable
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

  wifiManager.addReportStepHook(reportStep);
  mainsMonitor.addReportStepHook(reportStep);
  otaManager.addReportStepHook(reportStep);

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
  //wifiManager.setLEDs(&normalLED, &errorLED);
  //mainsMonitor.setLEDs(&normalLED, &errorLED);

  Serial.begin(115200);
  logger.begin();
  logger.log("Start\n");

  defaultManager.loadConfig();
  wifiManager.setAPcredentials(defaultManager.getValue("wifiAP.APssid", "Energy_Monitor_IoT"), defaultManager.getValue("wifiAP.APpassword", ""));
 

  if (!configManager.loadConfig()) {
    logger.log("Failed to load configuration.\n");
    configManager.resetConfig();
    errorLED.setMode(BLINKONCE); 
  }

  operationMode=NORMAL;
  if(!wifiManager.firstConnectToAP()) { // switch mode 
    operationMode=SETTINGS;;   
  }
  else { // Connected to WIFI, do DateTime 
    DateTime.setServer("pool.ntp.org");
    DateTime.setTimeZone("EET-2EEST,M3.5.0/3,M10.5.0/4"); // TZ_Europe_Athens from here https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
    DateTime.begin();
    if (!DateTime.isTimeValid()) {
      logger.log("Failed to get time from server.\n");
    } else {
      logger.logf("Date Now is %s\n", DateTime.toISOString().c_str());
      logger.logf("Date Now is %s\n", "Timestamp is %ld\n", DateTime.now());
    }
  }

  configManager.begin();  // i.e., registerEndpoints()
  defaultManager.begin(); // i.e., registerEndpoints()
  wifiManager.begin();    // i.e., registerEndpoints()
  otaManager.begin();     // i.e., registerEndpoints()
  httpServerManager.begin();
  mqttManager.begin();

  mainsMonitor.begin(configManager.getValue("http.entryPointUrl", ""), configManager.getValue("wifi.hostname", ""));
  aht10Monitor.begin();

  normalLED.setMode(BLINKONCE);


  // Initialize I2C with custom pins
  
  //myAHT10.begin(SDA_PIN, SCL_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  logger.loop();
  normalLED.loop();
  errorLED.loop();

  currentTime = millis();
  
  if(operationMode==SETTINGS && currentTime - lastTimeWifiCheck > 30000) {
    lastTimeWifiCheck = currentTime;
    if(wifiManager.reConnectoAP()){
      wifiManager.reboot();
    }
  }

  if(operationMode==NORMAL && !wifiManager.reConnectoAP()) { // switch mode
    operationMode=SETTINGS;
    errorLED.setMode(BLINK); 
  }
  
  if(operationMode==SETTINGS){
    wifiManager.createAP();
  }

  httpServerManager.handleClient();

  mqttManager.loop();

  mainsMonitor.loop();

  aht10Monitor.loop();


}

