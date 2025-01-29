#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger {
public:
    virtual void begin() {};
    virtual void loop() {};
    virtual void log(const char* message) = 0;
    virtual void log(String message) {
        log(message.c_str());
    }

    // New logf method. This is variadic template. Usage example: logger.logf("Date Now is %s, Timestamp is %ld", "2025-01-13T12:34:56Z", timestamp);
    template <typename... Args>
    void logf(const char* format, Args... args) {
        char buffer[128]; // Adjust size as needed
        snprintf(buffer, sizeof(buffer), format, args...);
        log(buffer);
    }
};

#endif