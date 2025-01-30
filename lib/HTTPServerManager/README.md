## HTTPServerManager Class

The HTTPServerManager class starts an HTTP server on the ESP8266. It serves static 
files from the filesystem (using LittleFS) for GET requests, provides a websocket 
broadcast method and provides an easy integration of additional endpoint, that can 
handle GET/POST API request and file uploads.


### Usage Example
The below simple example illustrates how to setup and use HTTPServerManager object.
In order to show the integration of soome additional endpoint an object of class 
ConfigurationManager is used. The endpoint "/custom" is registered as a GET method. 
The endpoint "/broadcast" is registers also as a GET method, but it demostrates the 
use of "broadcastWebSocketMessage" method.

```c++

#include "HTTPServerManager.h"
#include "ConfigurationManager.h"
#include "ConsoleLogger.h"

ConfigurationManager configManager("/configurations.json");
ConsoleLogger consoleLogger;
HTTPServerManager httpServerManager(configManager, consoleLogger);

void setup() {
    Serial.begin(115200);

    if (!configManager.loadConfig()) {
        consoleLogger.log("Failed to load configuration.");
    }

    httpServerManager.begin();

    // Register a custom page
    httpServerManager.registerPage("/custom", HTTP_GET, [](ESP8266WebServer& server) {
        server.send(200, "text/plain", "This is a custom page!");
    });

    // WebSocket broadcast example
    httpServerManager.registerPage("/broadcast", HTTP_GET, [&httpServerManager](ESP8266WebServer& server) {
        httpServerManager.broadcastWebSocketMessage("Hello WebSocket Clients!");
        server.send(200, "text/plain", "Broadcast message sent");
    });
}

void loop() {
    httpServerManager.loop();
}

```

### Features

1. Serves static files (e.g., HTML, CSS, JS) from LittleFS.
2. Allows easy integration of additional endpoints.
3. Exposes method to broadcast a message to all WebSocket clients
4. Abstract logging using Logger for debugging.
