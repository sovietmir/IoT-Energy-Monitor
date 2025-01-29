#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "common.h"
#include "Logger.h"

enum LEDMode {
    NONE,
    BLINKONCE,
    BLINK2TIMES,
    BLINK3TIMES,
    BLINKFAST,
    BLINK,
    PERMANENT,
};


class LEDHandler {
public:
    LEDHandler(String label, Logger& logger);
    void begin(int pin);
    void loop();

    void setMode(LEDMode mode);

    void on();
    void off();
private:
    const String _label;
    Logger& _logger;
    int _pin = -1;
    
    LEDMode _mode=NONE;
    int times = 0;
    unsigned long lastOnTime; 
    unsigned long periodicity = 10000; // Default periodicity in milliseconds
    unsigned long activeTime = 500;    // Default active time in milliseconds

    bool isOn = false;
    
};

#endif