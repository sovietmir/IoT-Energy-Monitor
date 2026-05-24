#ifndef MAINS_MONITOR_H
#define MAINS_MONITOR_H

#include <PZEM004Tv30.h>
#include "common.h"
#include <IoTesp8266Framework.h> //< HTTPServerManager/HTTPServerManager.h, Logger/Logger.h and Sensor/Sensor.h are used 
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
    String device;      ///< Device Name (_deviceName)
    String time;        ///< Time of the measurement (format: YYYY-mm-dd HH:ii:ss).
    float voltage;      ///< Voltage in volts.
    float current;      ///< Current in amperes.
    float power;        ///< Power in watts.
    float energy;       ///< Energy in kilowatt-hours.
    float frequency;    ///< Frequency in hertz.
    float powerfactor;  ///< Power factor.

    void toJson(JsonDocument& doc) const {
        doc["device"] = device;
        doc["time"] = time;
        doc["voltage"] = voltage;
        doc["current"] = current;
        doc["power"] = power;
        doc["energy"] = energy;
        doc["frequency"] = frequency;
        doc["powerfactor"] = powerfactor;
    }
    bool isValid() const {
        if(isnan(voltage)){
            return false;
        }
        if (isnan(current)) {
            return false;
        } 
        if (isnan(power)) {
            return false;
        }
        if (isnan(energy)) {
            return false;
        }
        if (isnan(frequency)) {
            return false;
        }
        if (isnan(powerfactor)) {
            return false;
        }
        return true;
    }
};

/**
 * @class MainsMonitor
 * @brief Class to monitor mains electricity metrics using PZEM004Tv30 sensor.
 */
class MainsMonitor : public Sensor<mainsMetrics>{
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
    void begin(String URL, String deviceName, int periodicity=10000);

    /**
     * @brief Main loop to read and report metrics periodically.
     */
    void loop();

    /**
     * @brief Reads the current metrics from the PZEM sensor.
     * @return The read metrics.
     */
    //mainsMetrics read();
    mainsMetrics read() override;

    /**
     * @brief Logs the metrics to the logger.
     * @param metric The metrics to log.
     */
    //void logMetrics(mainsMetrics& metric);
    void logMetrics(const mainsMetrics& metric) override;

    /**
     * @brief Sends the metrics via an HTTP GET request.
     * @param metrics The metrics to send.
     * @return True if the request was successful, false otherwise.
     */
    bool makeHTTPGETRequest(mainsMetrics& metrics);


private:
    //HTTPServerManager& _serverManager; ///< Reference to the HTTPServerManager instance.
    //Logger* _logger; ///< Pointer to the Logger instance.
    SoftwareSerial _pzemSWSerial; ///< SoftwareSerial instance for PZEM communication.
    PZEM004Tv30 _pzem; ///< PZEM004Tv30 instance for reading metrics.

    String _URL=""; ///< URL to send metrics to.
    String _deviceName=""; ///< Name of the device that will be associated with the sent metrics.

    //long _lastTime = 0; ///< Last time metrics were read.
    //int _periodicity = 10000; ///< Periodicity of reading metrics in milliseconds.

    //mainsMetrics _lastMetrics; ///< Last read metrics.

    //std::vector<std::function<void(int)>> _reportStepsHooks; ///< List of report step hooks.
    //std::vector<std::function<void(mainsMetrics&)>> _reportMetricsHooks; ///< List of report metrics hooks.

    /**
     * @brief Serializes the metrics to a JSON string.
     * @param metrics The metrics to serialize.
     * @return The serialized JSON string.
     */
    //String serializeMetrics(const mainsMetrics& metrics);

    /**
     * @brief URL encodes the given string.
     * @param str The string to encode.
     * @return The encoded string.
     */
    String urlencode(const String& str);
};

#endif