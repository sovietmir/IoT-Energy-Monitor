#ifndef AHT10_MONITOR_H
#define AHT10_MONITOR_H

#include <AHT10.h>
#include "common.h"
#include "HTTPServerManager.h"
#include "Logger.h"
#include <ArduinoJson.h>
#include <vector>

#if !defined(SDA_PIN)
  #define SDA_PIN 5  ///< Default SDA pin for I2C communication.
#endif
#if !defined(SCL_PIN)
  #define SCL_PIN 4  ///< Default SCL pin for I2C communication.
#endif

/**
 * @struct aht10Metrics
 * @brief Structure to hold AHT10 sensor metrics such as temperature and humidity.
 */
struct aht10Metrics {
    String time;        ///< Time of the measurement  (format: YYYY-mm-dd HH:ii:ss).
    float temperature;  ///< Temperature in degrees Celsius.
    float humidity;     ///< Humidity in percentage.
};

/**
 * @class AHT10Monitor
 * @brief Class to monitor temperature and humidity using the AHT10 sensor.
 * 
 * This class provides methods to interact with sensor AHT10. It supports periodic 
 * reading data from the sensor, thus monitoring it, as well as handling HTTP API calls 
 * for retriving last data read amd setting the periodicity of readings.
 */
class AHT10Monitor {
public:
    /**
     * @brief Constructor for AHT10Monitor.
     * @param serverManager Reference to the HTTPServerManager instance.
     * @param logger Pointer to the Logger instance.
     */
    AHT10Monitor(HTTPServerManager& serverManager, Logger* logger);

    /**
     * @brief Initializes the AHT10 sensor.
     */
    void begin();

    /**
     * @brief Registers endpoints for the HTTP server.
     */
    void registerEndpoints();

    /**
     * @brief Main loop to read and report metrics periodically.
     */
    void loop();

    /**
     * @brief Reads the current metrics from the AHT10 sensor.
     * @return The read metrics.
     */
    aht10Metrics read();

    /**
     * @brief Logs the metrics to the logger.
     * @param metric The metrics to log.
     */
    void logMetrics(aht10Metrics& metric);

    /**
     * @brief Broadcasts the metrics to all connected WebSocket clients.
     * @param metrics The metrics to broadcast.
     */
    void broadcastMetrics(aht10Metrics& metrics);

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
    void addReportMetricsHook(std::function<void(aht10Metrics&)> func);

    /**
     * @brief Reports the metrics to all registered hooks.
     * @param metric The metrics to report.
     */
    void reportMetrics(aht10Metrics& metric);

    /**
     * @brief Handles the set periodicity request.
     * @param server Reference to the ESP8266WebServer instance.
     */
    void handleSetPeriodicityRequest(ESP8266WebServer& server);

private:
    HTTPServerManager& _serverManager; ///< Reference to the HTTPServerManager instance.
    Logger* _logger; ///< Pointer to the Logger instance.
    AHT10 _AHT10; ///< AHT10 sensor instance.

    long _lastTime = 0; ///< Last time metrics were read.
    long _periodicity = 10000; ///< Periodicity of reading metrics in milliseconds.

    aht10Metrics _lastMetrics; ///< Last read metrics.

    std::vector<std::function<void(int)>> _reportStepsHooks; ///< List of report step hooks.
    std::vector<std::function<void(aht10Metrics&)>> _reportMetricsHooks; ///< List of report metrics hooks.

    /**
     * @brief Serializes the metrics to a JSON string.
     * @param metrics The metrics to serialize.
     * @return The serialized JSON string.
     */
    String serializeMetrics(const aht10Metrics& metrics);
};

#endif