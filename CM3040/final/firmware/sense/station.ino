#include <ESP8266WiFi.h>

#include "ESPAsyncWebServer.h"

// Configure endpoints for serving the dashboard
void configureStation(void) {
  // Serve images
  server.serveStatic("/img/", LittleFS, "/img/");

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Route to load script.js file
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/script.js", "application/javascript");
  });

  // Route to load config.js file
  server.on("/config.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/config.js", "application/javascript");
  });

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
}
