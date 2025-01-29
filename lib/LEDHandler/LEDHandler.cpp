#include "LEDHandler.h"


LEDHandler::LEDHandler(String label, Logger& logger) : _label(label), _logger(logger) {}

void LEDHandler::begin(int pin) {
    _pin  = pin;
    pinMode(_pin, OUTPUT);    
}

void LEDHandler::setMode(LEDMode mode) {
    _mode = mode;
    times = 0;
    // Set periodicity and active time based on mode
    if (_mode == BLINKONCE) {
        periodicity = 5000;
        activeTime = 1000;
    } 
    else if (_mode == BLINK2TIMES || _mode == BLINK3TIMES || _mode == BLINKFAST) {
        periodicity = 400;
        activeTime = 200;
    }
    else if (_mode == BLINK) {
        periodicity = 10000;
        activeTime = 500;
    } else if (_mode == PERMANENT) {
        periodicity = 0;
        activeTime = 10;
    }
}

void LEDHandler::loop(){
    if(_pin==-1) return;
    
    if (_mode == NONE) {
        off();
        return;
    }

    long unsigned currentTime = millis();

    // Handle LED on/off logic
    if (currentTime - lastOnTime > periodicity) {
        on();
        lastOnTime = currentTime;
        times++;      
    }

    if (currentTime - lastOnTime > activeTime && isOn==true) {
        off();      
        if ((_mode == BLINK3TIMES && times>=3) || (_mode == BLINK2TIMES && times>=2) ||  _mode == BLINKONCE) {
            setMode(NONE);
        }
    }
}


void LEDHandler::on(){
    if(_pin==-1 || isOn) return;
    digitalWrite(_pin, HIGH);// Turn on
    _logger.logf("%ld\t %s: Turn on\n", millis(), _label.c_str());
    isOn = true;
}
void LEDHandler::off(){
    if(_pin==-1 || !isOn) return;
    digitalWrite(_pin, LOW); // Turn off
    _logger.logf("%ld\t %s: Turn off\n", millis(), _label.c_str());
    isOn = false;
}