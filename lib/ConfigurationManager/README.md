# ConfigurationManager Class

This class handles reading, writing, validating, and providing configuration data 
to other parts of the system. It uses JSON for configuration storage and provides 
an HTTP API for remote configuration management.

## Features

- **Load and Save Configuration**: Load configuration from a JSON file and save changes back to the file.
- **Configuration Retrieval**: Retrieve configuration values using a path-based approach (e.g., `wifi.ssid`).
- **HTTP API**: Register HTTP endpoints to allow remote reading and updating of the configuration.
- **Logging**: Integrates with a logging system to log configuration changes and errors.

## Usage Example

```cpp
#include "ConfigurationManager.h"

ConsoleLogger logger;
HTTPServerManager httpServerManager(&logger);

ConfigurationManager configManager("config", httpServerManager, &logger);

void setup() {
    Serial.begin(115200);

    if (!configManager.loadConfig()) {
        Serial.println("Failed to load configuration.");
        configManager.resetConfig();
    }

    // Retrieve configuration values
    Serial.println(configManager.getValue("wifi.ssid", ""));
    Serial.println(configManager.getValue("device.name", ""));
    Serial.println(configManager.getValue("mqtt.port", 1883));

    // Example: Update configuration
    StaticJsonDocument<1024> newConfig;
    newConfig["wifi"]["ssid"] = "NewSSID";
    newConfig["wifi"]["password"] = "NewPassword";
    configManager.setConfig(newConfig.as<JsonObject>());
    configManager.saveConfig();
}

void loop() {
    // Main logic here
}
```

## API Endpoints

The `ConfigurationManager` class registers the following HTTP endpoints:

- **GET `/api/<name>/read`**: Retrieves the current configuration as a JSON object.
- **POST `/api/<name>/save`**: Updates the configuration with the provided JSON object.

Where `<name>` is the file name passed when instantiating the `ConfigurationManager` object. For example, if the `ConfigurationManager` is instantiated with `"config"`, the endpoints will be:

- **GET `/api/config/read`**
- **POST `/api/config/save`**

## Methods

- **loadConfig()**: Loads the configuration from the file system.
- **saveConfig()**: Saves the current configuration to the file system.
- **resetConfig()**: Resets the configuration to an empty state.
- **getValue(path, defaultValue)**: Retrieves a configuration value using a path-based approach.
- **setConfig(newConfig)**: Sets the current configuration to the provided JSON object.
- **getConfig()**: Retrieves the current configuration as a JSON document.
- **begin()**: Initializes the ConfigurationManager and registers HTTP endpoints.
- **registerEndpoints()**: Registers HTTP endpoints for configuration management.

## Dependencies

- **ArduinoJson**: For handling JSON data.
- **LittleFS**: For file system operations.
- **HTTPServerManager**: For managing HTTP server endpoints.
- **Logger**: For logging messages.

## Notes

- The configuration file should be in JSON format and stored in the root directory of the file system.
- The `loadConfig()` method initializes the LittleFS file system internally, so there is no need to initialize it manually before calling `loadConfig()`.