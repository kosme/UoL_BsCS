#include <ESP8266WiFi.h>

#include "ESPAsyncWebServer.h"

void configureApi(void) {
  server.on("/networks", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", scanAvailableNetworks());
  });

  server.on("/waqi", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"waqi\":";
    answer.concat(waqi);
    answer.concat("}");
    request->send(200, "application/json", answer);
  });

  server.on("/aqi", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"aqi\":";
    answer.concat(aqi_value);
    answer.concat("}");
    request->send(200, "application/json", answer);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"temperature\":";
    answer.concat(dht11.temperature);
    answer.concat("}");
    request->send(200, "application/json", answer);
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"humidity\":";
    answer.concat(dht11.humidity);
    answer.concat("}");
    request->send(200, "application/json", answer);
  });
}
