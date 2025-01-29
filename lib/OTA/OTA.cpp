#include "OTA.h"

OTA::OTA(HTTPServerManager& serverManager, Logger& logger) : _serverManager(serverManager),_logger(logger) {}

void OTA::begin(){
    registerEndpoints();
}
void OTA::registerEndpoints() {
    // Register firmware upload endpoint
    _serverManager.registerPage(
        "/api/firmware", 
        HTTP_POST, 
        [this](ESP8266WebServer& server) {
          handleFirmwareUpload(server);
        }, 
        [this](ESP8266WebServer& server) {
          handleFirmwareUpload(server); // Proper file upload handler
        }
    );

    // Register file upload endpoint
    _serverManager.registerPage(
        "/api/upload", 
        HTTP_POST, 
        [this](ESP8266WebServer& server) {
          handleFileUpload(server);
        },
        [this](ESP8266WebServer& server) {
          handleFileUpload(server); // Proper file upload handler
        }
    );
    
    _serverManager.registerPage("/api/reboot", HTTP_GET, [this](ESP8266WebServer& server) {
        _logger.log("URI: /api/");
        server.send(200, "application/json", "{\"message\":\"The microcontroller shall reboot in half a second...\"}");
        delay(500);
        ESP.restart();
    });

    // Register directory listing
    _serverManager.registerPage("/api/directories", HTTP_GET, [this](ESP8266WebServer& server) {
        _logger.log("URI: /api/directories");
        handleDirectoryList(server);
    });


    // Register API endpoints
    _serverManager.registerPage("/api/files", HTTP_GET, [this](ESP8266WebServer& server) {
        handleFileSystemRequest(server);
    });

    _serverManager.registerPage("/api/download", HTTP_GET, [this](ESP8266WebServer& server) {
        handleDownloadRequest(server);
    });

    _serverManager.registerPage("/api/delete", HTTP_DELETE, [this](ESP8266WebServer& server) {
        handleDeleteRequest(server);
    });

    _serverManager.registerPage("/api/addDirectory", HTTP_POST, [this](ESP8266WebServer& server) {
        handleAddDirectoryRequest(server);
    });
}

void OTA::handleFirmwareUpload(ESP8266WebServer& server) {
    HTTPUpload& upload = server.upload();
    uint32_t update_size = ((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
     
    reportStep(1);
    //Update.runAsync(true);
    if (upload.status == UPLOAD_FILE_START) {
        _logger.log("Start firmware update: "+upload.filename+".\n");
       if (!Update.begin(update_size)) { // Begin OTA process
            reportStep(2);
            reportStep(-3);
            Update.printError(Serial);
            server.send(500, "application/json", "{\"error\":\"Firmware update failed to start.\"}");
            _logger.log("Firmware update failed to start.\n");
            return;
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { // End OTA process
            server.send(200, "application/json", "{\"message\":\"Firmware updated successfully. Rebooting...\"}");
            _logger.log("Firmware update successful. Rebooting...\n");
            delay(500);
            ESP.restart();
        } else {
            reportStep(-3);
            Update.printError(Serial);
            server.send(500, "application/json", "{\"error\":\"Firmware update failed.\"}");
            _logger.log("Firmware update failed.\\n");
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        reportStep(2);
        reportStep(-3);
        Update.end();
        server.send(500, "application/json", "{\"error\":\"Firmware update aborted.\"}");
        _logger.log("Firmware update aborted.\\n");
    }
}

void OTA::handleFileUpload(ESP8266WebServer& server) {
    HTTPUpload& upload = server.upload();
    static File file;         // Keeps file open across status changes
    static String directory;  // Keeps directory path across status changes
    
     reportStep(1);
    if (upload.status == UPLOAD_FILE_START) {
        _logger.log("Start file upload: "+upload.filename+";");
        directory = server.arg("directory"); // Get the directory from the form
        if (directory.isEmpty()) directory = "/"; // Default to root if not provided

        String path = directory  + "/" + upload.filename; // Add subdirectory here
        file = LittleFS.open(path, "w");
        if (!file) {
            server.send(500, "application/json", "{\"error\":\"Failed to open file for writing.\"}");
            _logger.log(" Failed to open file '"+path+"' for writing.\n");
            reportStep(2);
            reportStep(-3);
            return;
        }
        _logger.log("\n");
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        _logger.log("File uploading (writing)...\n");        
        if (file) {
            file.write(upload.buf, upload.currentSize);
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (file) {
            file.close();            
            server.send(200, "application/json", "{\"message\":\"File uploaded successfully.\"}");
            _logger.log("File upload successful.\n");
        } else {
            server.send(500, "application/json", "{\"error\":\"Failed to save file.\"}");
            _logger.log("Failed to save file.\n");
            reportStep(-2);
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (file) {
            file.close();
            LittleFS.remove(upload.filename); // Clean up
        }
        server.send(500, "application/json", "{\"error\":\"File upload aborted.\"}");
        _logger.log("File upload aborted.\n");
        reportStep(2);
        reportStep(-1);
    }
}

void OTA::handleDirectoryList(ESP8266WebServer& server) {
    String response = "[";
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        response += "\"" + dir.fileName() + "\",";
    }
    if (response.endsWith(",")) response.remove(response.length() - 1); // Remove trailing comma
    response += "]";
    server.send(200, "application/json", response);
}


/**
 * Handles listing files.
 *
 * @param server Reference to the ESP8266WebServer instance managing the request.
 *               Used to access request parameters and send responses.
 */
void OTA::handleFileSystemRequest(ESP8266WebServer& server) {
    _logger.log("handleFileSystemRequest: ");
    JsonDocument  doc;                     // Create a JSON document to store file data.
    JsonArray files = doc["files"].to<JsonArray>();

    FSInfo fs_info;
    LittleFS.info(fs_info);
    doc["total"] = fs_info.totalBytes;
    doc["used"] = fs_info.usedBytes;
    doc["free"] = fs_info.totalBytes - fs_info.usedBytes;

        // Start recursive listing from the root directory.
    listFilesRecursive("/", files);
        
        // Serialize the JSON data and send it as the response.
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    _logger.log("ok\n");    
}


/**
 * Recursively retrieves all files and directories from the file system,
 * starting from the specified directory.
 *
 * @param dirPath The path to the directory to start the traversal.
 * @param filesArray A JSON array to store file and directory details.
 */
void OTA::listFilesRecursive(const String& dirPath, JsonArray& filesArray) {
    Dir dir = LittleFS.openDir(dirPath); // Open the directory at dirPath.
    while (dir.next()) {
        JsonObject file = filesArray.add<JsonObject>(); // Create a JSON object for the entry.
        file["name"] = dirPath + dir.fileName(); // Full path to the file/directory.
        file["type"] = dir.isDirectory() ? "directory" : "file"; // Type: file or directory.
        file["size"] = dir.isDirectory() ? 0 : dir.fileSize(); // Size (0 for directories).

        // If it's a directory, recurse into it.
        if (dir.isDirectory()) {
            listFilesRecursive(dirPath + dir.fileName() + "/", filesArray);
        }
    }
}




/**
 * Handles  downloading a file
 * 
 * @param server Reference to the ESP8266WebServer instance managing the request.
 *               Used to access request parameters and send responses.
 */
void OTA::handleDownloadRequest(ESP8266WebServer& server) {
    String filePath = server.arg("file"); // Extract the 'file' parameter from the request if provided.
    _logger.log("handleDownloadRequest '"+filePath+"'");

    if (LittleFS.exists(filePath)) {
        File file = LittleFS.open(filePath, "r"); // Attempt to open the requested file in read mode.
        server.streamFile(file, "application/octet-stream");
        file.close();
    } else {
        server.send(404, "application/json", "{\"error\":\"File not found\"}");
    }
    _logger.log("\n");
   
}

/**
 * Handles delete file
 * 
 * @param server Reference to the ESP8266WebServer instance managing the request.
 *               Used to access request parameters and send responses.
 */
void OTA::handleDeleteRequest(ESP8266WebServer& server) {
    String path = server.arg("path");
    _logger.log("handleDeleteRequest: "+path);
    if (LittleFS.exists(path)) { // Check if the file exists.
        if (LittleFS.remove(path)) { // Delete the file.
            server.send(200, "application/json", "{\"status\":\"File deleted successfully\"}");
        } else {
            server.send(500, "application/json", "{\"error\":\"Failed to delete file\"}");
        }            
    } else {
        server.send(404, "application/json", "{\"error\":\"File not found\"}");
    }
    _logger.log("\n");
   
}


/**
 * Handles add director, i.e., creates given direcotry
 * The request to create a direcotry is POST, with plain paylod that contains an object 
 * which is JSON encoded. The object should have fields 'parentPath' and 'dirName'
 * 
 * @param server Reference to the ESP8266WebServer instance managing the request.
 *               Used to access request parameters and send responses.
 */
void OTA::handleAddDirectoryRequest(ESP8266WebServer& server) {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Bad Request\"}");
        return;
    }
    
    String json = server.arg("plain");
    JsonDocument params;
    DeserializationError error = deserializeJson(params, json);
    if(error) {
        server.send(500, "application/json", "{\"error\":\"Failed to deserializeJson the request data\"}");
        return;
    }

    String parentPath = params["parentPath"];//server.arg("parentPath");
    String dirName =  params["dirName"];// server.arg("dirName");
    String fullPath = parentPath + "/" + dirName;
    _logger.log("handleAddDirectoryRequest '"+fullPath+"': ");

    if (LittleFS.mkdir(fullPath)) {
        server.send(200, "application/json", "{\"status\":\"Directory created successfully\"}");
        _logger.log("ok\n");
    } else {
        server.send(500, "application/json", "{\"error\":\"Failed to create directory\"}");
        _logger.log("nok\n");
    }
   
}






void OTA::addReportStepHook(std::function<void(int)> func) {
  reportStepsHooks.push_back(func);
}
void OTA::reportStep(int step){
  for (auto& hook : reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}