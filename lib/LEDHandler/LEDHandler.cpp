#include "LEDHandler.h"

LEDHandler::LEDHandler(String label, Logger* logger) 
    : _label(label), 
      _logger(logger) 
      {}

void LEDHandler::begin(int pin) {
    _pin  = pin;
    pinMode(_pin, OUTPUT);    
}

void LEDHandler::setMode(LEDMode mode) {
    _mode = mode;
    _times = 0;
    // Set periodicity and active time based on mode
    if (_mode == BLINKONCE) {
        _periodicity = 5000;
        _activeTime = 1000;
    } 
    else if (_mode == BLINK2TIMES || _mode == BLINK3TIMES || _mode == BLINKFAST) {
        _periodicity = 400;
        _activeTime = 200;
    }
    else if (_mode == BLINK) {
        _periodicity = 10000;
        _activeTime = 500;
    } else if (_mode == PERMANENT) {
        _periodicity = 0;
        _activeTime = 10;
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
    if (currentTime - _lastOnTime > _periodicity) {
        on();
        _lastOnTime = currentTime;
        _times++;      
    }

    if (currentTime - _lastOnTime > _activeTime && _isOn==true) {
        off();      
        if ((_mode == BLINK3TIMES && _times>=3) || (_mode == BLINK2TIMES && _times>=2) ||  _mode == BLINKONCE) {
            setMode(NONE);
        }
    }
}

void LEDHandler::on(){
    if(_pin==-1 || _isOn) return;
    digitalWrite(_pin, HIGH);// Turn on
    if (_logger != nullptr) _logger->logf("%ld\t %s: Turn on\n", millis(), _label.c_str());
    _isOn = true;
}

void LEDHandler::off(){
    if(_pin==-1 || !_isOn) return;
    digitalWrite(_pin, LOW); // Turn off
    if (_logger != nullptr) _logger->logf("%ld\t %s: Turn off\n", millis(), _label.c_str());
    _isOn = false;
}