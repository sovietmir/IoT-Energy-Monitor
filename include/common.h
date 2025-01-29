#ifndef COMMON_H
#define COMMON_H

constexpr const char* IoTclassName = "EnergyMonitor"; // Energy Monitor


#define INDICATOR 14        // Green LED, to inticate modes 
#define ERROR_INDICATOR 16  // Red LED, to inticate erros 

// PZEM's Serial: RX 14, TX 12
#define RX_PIN 12 // Connect to PZEM's TX
#define TX_PIN 13 // Connect to PZEM's RX

// By default I2C: scl D5, sda D4
#define SDA_PIN 5 
#define SCL_PIN 4 

#endif