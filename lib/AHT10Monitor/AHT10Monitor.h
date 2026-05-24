#ifndef AHT10_MONITOR_H
#define AHT10_MONITOR_H

#include <AHT10.h>
#include "common.h"
#include <IoTesp8266Framework.h> //< HTTPServerManager/HTTPServerManager.h, Logger/Logger.h and Sensor/Sensor.h are used 
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

    void toJson(JsonDocument& doc) const {
        doc["time"] = time;
        if(isValid()){
          doc["temperature"] = temperature;
          doc["humidity"] = humidity;
        }
    }
    bool isValid() const {
        return !isnan(temperature);
    }
};

/**
 * @class AHT10Monitor
 * @brief Class to monitor temperature and humidity using the AHT10 sensor.
 * 
 * This class provides methods to interact with sensor AHT10. It supports periodic 
 * reading data from the sensor, thus monitoring it, as well as handling HTTP API calls 
 * for retriving last data read amd setting the periodicity of readings.  The
 * later is done by registering paths `/api/getLastMetricsAHT10`, 
 * `/api/getPeriodicityAHT10` and `/api/setPeriodicityAHT10`,respectfully. 
 */
class AHT10Monitor : public Sensor<aht10Metrics>  {
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
    void begin() override;


    /**
     * @brief Reads the current metrics from the AHT10 sensor.
     * @return The read metrics.
     */
    aht10Metrics read() override;

    /**
     * @brief Logs the metrics to the logger.
     * @param metric The metrics to log.
     */
    void logMetrics(const aht10Metrics& metric) override;

private:
    AHT10 _AHT10; ///< AHT10 sensor instance.
};

#endif