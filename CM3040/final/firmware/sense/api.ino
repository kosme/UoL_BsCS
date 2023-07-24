#include <ESP8266WiFi.h>

#include "ESPAsyncWebServer.h"

void configureApi(void) {
  server.on("/co2", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"co2\":";
    answer.concat(eCO2);
    answer.concat("}");
    request->send(200, "application/json", answer);
  });

  server.on("/voc", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"voc\":";
    answer.concat(VOC);
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

  server.on("/values", HTTP_GET, [](AsyncWebServerRequest* request) {
    String answer = "{\"alias\":\"";
    answer.concat(tempConfig.alias);
    answer.concat("\",\"co2\":");
    answer.concat(eCO2);
    answer.concat(",\"voc\":");
    answer.concat(VOC);
    answer.concat(",\"temperature\":");
    answer.concat(dht11.temperature);
    answer.concat(",\"humidity\":");
    answer.concat(dht11.humidity);
    answer.concat("}");
    request->send(200, "application/json", answer);
  });

  server.on("/clearconfig", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (clearConfig()) {
      request->send(200);
      shouldReboot = true;
    } else {
      request->send(400);
    }
  });
}
