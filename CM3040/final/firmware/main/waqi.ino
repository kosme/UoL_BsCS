#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <StreamUtils.h>
#include <WiFiClient.h>

StaticJsonDocument<48> filter;
DynamicJsonDocument doc(100);

void createWaqiFilter(void) {
  filter["status"] = true;
  filter["data"]["aqi"] = true;
}

int getAirQualityReading(void) {
  int reading = -14;
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;

    if (http.begin(client,
                   F("http://api.waqi.info/feed/here/"
                     "?token=b5d184a1c6e4d63c73a61c2059ee4cb95be8853a"))) {
      // Serial.println("[HTTP] GET...");
      // start connection and send HTTP header
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        // Serial.print("[HTTP] GET... code: ");
        // Serial.println(httpCode, DEC);

        // file found at server
        if (httpCode == HTTP_CODE_OK ||
            httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          DeserializationError error = deserializeJson(
              doc, http.getStream(), DeserializationOption::Filter(filter));

          if (error) {
            // Serial.print(F("deserializeJson() failed: "));
            // Serial.println(error.f_str());
            http.end();
            client.stop();
            return -12;
          }

          // status is part of the answer sent by the remote server. It can be
          // ok or error
          const char* status = doc["status"];
          // Check response code is "ok"
          if (String(status).equals("ok")) {
            reading = doc["data"]["aqi"];
            // Serial.print(F("Current air quality index is: "));
            // Serial.println(reading);
          } else {
            // Serial.println(status);
            reading = -13;
          }
        }
      } else {
        // Return http error code
        // Serial.printf("[HTTP] GET... failed, error: %s\n",
        //               http.errorToString(httpCode).c_str());
        reading = httpCode;
      }
    }
    http.end();
    client.stop();
  }
  return reading;
}
