#include <ESP8266WiFi.h>

#include "ESPAsyncWebServer.h"

#ifndef MAX_NODES_NUMBER
#define MAX_NODES_NUMBER 3
#endif

// Configure endpoints for serving the dashboard and interacting with it
void configureStation(void) {
  // Serve assets (images, js, and css)
  server.serveStatic("/assets/", LittleFS, "/assets/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/config.html", String(), false, processor);
  });

  // Endpoint to change the device alias
  server.on("/alias", HTTP_POST, [](AsyncWebServerRequest* request) {
    loadConfig(&tempConfig);
    if (request->hasParam("alias", true)) {
      Serial.println("Changing alias");
      strlcpy(tempConfig.alias,
              request->getParam("alias", true)->value().c_str(),
              sizeof(tempConfig.alias));
      saveConfig(&tempConfig);
      request->send(200, "text/plain", "Device name changed.");
    } else {
      request->send(400);
    }
  });

  server.on("/addsensor", HTTP_GET, [](AsyncWebServerRequest* request) {
    startMicroAP();
    request->send(200, "text/plain", "Starting sync process.");
  });

  server.on("/deletenode", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->hasParam("position", true)) {
      Serial.println("Deleting node");
      uint8_t pos = request->getParam("position", true)->value().toInt();
      if (pos < MAX_NODES_NUMBER) {
        nodeToDelete = pos;
        task_deleteNode.restart();
        request->send(200);
      }
    }
    request->send(400);
  });
}
