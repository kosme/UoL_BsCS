#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <StreamUtils.h>
#include <WiFiClient.h>

#ifndef MAX_NODES_NUMBER
#define MAX_NODES_NUMBER 3
#endif

bool connectToNetwork(networkConfig* network, uint8_t attempts) {
  if (strlen(network->pass) > 0) {
    WiFi.begin(network->ssid, network->pass);
  } else {
    WiFi.begin(network->ssid);
  }
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED &&
         --attempts > 0) {
    delay(1000);
    Serial.print(" . ");
  }
  Serial.println();
  // Connection failed, probably due to wrong network credentials
  if (WiFi.status() == WL_CONNECT_FAILED || attempts == 0) {
    WiFi.disconnect();
    return false;
  } else {
    Serial.println("Connected");
    Serial.print("Station IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.hostname(ArduinoOTA.getHostname());
    return true;
  }
}

// Send to the command to a sensor node so that it clears its network
// credentials
bool clearNodeNetwork(String remote) {
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    String name = "http://";
    name.concat(remote);
    name.concat(".local/clearconfig");
    if (http.begin(client, name)) {
      // start connection and send HTTP header
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          http.end();
          client.stop();
          return true;
        } else {
          Serial.println("Code");
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\r\n",
                      http.errorToString(httpCode).c_str());
      }
    } else {
      Serial.println("Client");
    }
    http.end();
    client.stop();
  }
  return false;
}

// Get the reading of one sensor node
bool getRemoteReading(String remote, remoteSensorData* data) {
  DynamicJsonDocument doc(400);
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    String name = "http://";
    name.concat(remote);
    name.concat(".local/values");
    if (http.begin(client, name)) {
      // start connection and send HTTP header
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          DeserializationError error = deserializeJson(doc, http.getStream());

          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            http.end();
            client.stop();
            return false;
          }

          strlcpy(data->alias, doc["alias"], sizeof(data->alias));
          data->co2 = (uint16_t)doc["co2"];
          data->voc = (uint16_t)doc["voc"];
          data->temperature = (float)doc["temperature"];
          data->humidity = (float)doc["humidity"];
        }
      } else {
        // Return http error code
        Serial.printf("[HTTP] GET... failed, error: %s\r\n",
                      http.errorToString(httpCode).c_str());
        http.end();
        client.stop();
        return false;
      }
      http.end();
      client.stop();
      return true;
    }
  }
  return false;
}

bool getAllNodesData(void) {
  for (uint8_t i = 0; i < MAX_NODES_NUMBER; i++) {
    remoteSensorData data;
    if (strlen(nodeNames[i])) {
      if (!getRemoteReading(nodeNames[i], &data)) {
        return false;
      }
      nodesData[i] = data;
    }
  }
  return true;
}
