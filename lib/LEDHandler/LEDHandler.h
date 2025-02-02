/**
 * @file LEDHandler.h
 * @brief LED control handler for different blinking modes.
 * 
 * This class manages an LED's state and behavior, including various blinking modes.
 */

#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "common.h"
#include "Logger.h"

/**
 * @enum LEDMode
 * @brief Defines different LED operation modes.
 */
enum LEDMode {
    NONE,        ///< No blinking, LED remains off
    BLINKONCE,   ///< LED blinks once
    BLINK2TIMES, ///< LED blinks twice
    BLINK3TIMES, ///< LED blinks three times
    BLINKFAST,   ///< LED blinks fast
    BLINK,       ///< LED blinks periodically
    PERMANENT    ///< LED remains on permanently
};

/**
 * @class LEDHandler
 * @brief Handles LED operations and blinking modes.
 */
class LEDHandler {
public:
    /**
     * @brief Constructs an LEDHandler object.
     * @param label A label identifying the LED instance.
     * @param logger Pointer to a Logger instance for debugging.
     */
    LEDHandler(String label, Logger* logger);

    /**
     * @brief Initializes the LED on a specified GPIO pin.
     * @param pin The GPIO pin to which the LED is connected.
     */
    void begin(int pin);

    /**
     * @brief Updates the LED state, should be called periodically.
     */
    void loop();

    /**
     * @brief Sets the blinking mode of the LED.
     * @param mode The desired LEDMode.
     */
    void setMode(LEDMode mode);

    /**
     * @brief Turns the LED on.
     */
    void on();

    /**
     * @brief Turns the LED off.
     */
    void off();

private:
    const String _label; ///< Label identifying the LED instance.
    Logger* _logger;     ///< Pointer to the logger instance.
    int _pin = -1;       ///< GPIO pin number for LED.
    
    LEDMode _mode = NONE; ///< Current LED mode.
    int _times = 0;       ///< Counter for blinking modes.
    unsigned long _lastOnTime;  ///< Timestamp of the last LED activation.
    unsigned long _periodicity = 10000; ///< Blinking periodicity in milliseconds.
    unsigned long _activeTime = 500;    ///< Active LED duration in milliseconds.

    bool _isOn = false; ///< LED state flag.
};

#endif // LED_HANDLER_H
