#ifndef MAINS_MONITOR_H
#define MAINS_MONITOR_H

#include <PZEM004Tv30.h>
#include "common.h"
#include "HTTPServerManager.h"
#include "Logger.h"
#include <ArduinoJson.h>
#include <vector>

#if !defined(RX_PIN)
  #define RX_PIN 12 // Connect to PZEM's TX
#endif
#if !defined(TX_PIN)
  #define TX_PIN 13 // Connect to PZEM's RX
#endif

/**
 * @struct mainsMetrics
 * @brief Structure to hold mains metrics such as voltage, current, power, etc.
 */
struct mainsMetrics {
    String time;        ///< Time of the measurement (format: YYYY-mm-dd HH:ii:ss).
    float voltage;      ///< Voltage in volts.
    float current;      ///< Current in amperes.
    float power;        ///< Power in watts.
    float energy;       ///< Energy in kilowatt-hours.
    float frequency;    ///< Frequency in hertz.
    float powerfactor;  ///< Power factor.
};

/**
 * @class MainsMonitor
 * @brief Class to monitor mains electricity metrics using PZEM004Tv30 sensor.
 */
class MainsMonitor {
public:
    /**
     * @brief Constructor for MainsMonitor.
     * @param serverManager Reference to the HTTPServerManager instance.
     * @param logger Pointer to the Logger instance.
     */
    MainsMonitor(HTTPServerManager& serverManager, Logger* logger);

    /**
     * @brief Initializes the MainsMonitor with the given URL and device name.
     * @param URL The URL to send metrics to.
     * @param deviceName The name of the device that will be associated with the sent metrics.
     */
    void begin(const char* URL, const char* deviceName, int periodicity=10000);

    /**
     * @brief Main loop to read and report metrics periodically.
     */
    void loop();

    /**
     * @brief Reads the current metrics from the PZEM sensor.
     * @return The read metrics.
     */
    mainsMetrics read();

    /**
     * @brief Checks if the metrics are valid.
     * @param metric The metrics to check.
     * @return True if the metrics are valid, false otherwise.
     */
    bool checkMetrics(mainsMetrics& metric);

    /**
     * @brief Logs the metrics to the logger.
     * @param metric The metrics to log.
     */
    void logMetrics(mainsMetrics& metric);

    /**
     * @brief Sends the metrics via an HTTP GET request.
     * @param metrics The metrics to send.
     * @return True if the request was successful, false otherwise.
     */
    bool makeHTTPGETRequest(mainsMetrics& metrics);

    /**
     * @brief Adds a hook to be called during the report step.
     * @param func The function to be called.
     */
    void addReportStepHook(std::function<void(int)> func);

    /**
     * @brief Reports the current step to all registered hooks.
     * @param step The step to report.
     */
    void reportStep(int step);

    /**
     * @brief Adds a hook to be called when metrics are reported.
     * @param func The function to be called.
     */
    void addReportMetricsHook(std::function<void(mainsMetrics&)> func);

    /**
     * @brief Reports the metrics to all registered hooks.
     * @param metric The metrics to report.
     */
    void reportMetrics(mainsMetrics& metric);

    /**
     * @brief Registers endpoints for the HTTP server.
     */
    void registerEndpoints();
    
    /**
     * @brief Broadcasts the metrics to all connected WebSocket clients.
     * @param metrics The metrics to broadcast.
     */
    void broadcastMetrics(mainsMetrics& metrics);

    /**
     * @brief Handles the set periodicity request.
     * This sets the periodicity (in milliseconds) for reading metrics during 
     * the current run of the microcontroller.
     * Note that this value is lost when the microcontroller reboots.
     * 
     * @param server Reference to the ESP8266WebServer instance.
     */
    void handleSetPeriodicityRequest(ESP8266WebServer& server);

private:
    HTTPServerManager& _serverManager; ///< Reference to the HTTPServerManager instance.
    Logger* _logger; ///< Pointer to the Logger instance.
    SoftwareSerial _pzemSWSerial; ///< SoftwareSerial instance for PZEM communication.
    PZEM004Tv30 _pzem; ///< PZEM004Tv30 instance for reading metrics.

    const char* _URL=""; ///< URL to send metrics to.
    const char* _deviceName=""; ///< Name of the device that will be associated with the sent metrics.

    long _lastTime = 0; ///< Last time metrics were read.
    int _periodicity = 10000; ///< Periodicity of reading metrics in milliseconds.

    mainsMetrics _lastMetrics; ///< Last read metrics.

    std::vector<std::function<void(int)>> _reportStepsHooks; ///< List of report step hooks.
    std::vector<std::function<void(mainsMetrics&)>> _reportMetricsHooks; ///< List of report metrics hooks.

    /**
     * @brief Serializes the metrics to a JSON string.
     * @param metrics The metrics to serialize.
     * @return The serialized JSON string.
     */
    String serializeMetrics(const mainsMetrics& metrics);

    /**
     * @brief URL encodes the given string.
     * @param str The string to encode.
     * @return The encoded string.
     */
    String urlencode(const String& str);
};

#endif