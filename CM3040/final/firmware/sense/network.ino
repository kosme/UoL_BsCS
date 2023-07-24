#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

bool connectToNetwork(networkConfig *network, uint8_t attempts) {
  if (strlen(network->pass) > 0) {
    WiFi.begin(network->ssid, network->pass);
  } else {
    WiFi.begin(network->ssid);
  }
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED &&
         --attempts > 0) {
    for (uint8_t i = 0; i < 10; i++) {
      ts.execute();
      delay(100);
    }
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
    WiFi.hostname(hostname);
    return true;
  }
}

// Function for adding the sensor as a new system node.
// The device sends it unique hostname for it to be registered and
// receives the network credentials in needs for connecting to the system
bool fetchCredentials(networkConfig *network) {
  WiFiClient client;
  HTTPClient http;
  String body = "hostname=";
  body.concat(hostname);
  http.begin(client, "http://192.168.4.1/netconfig");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(body);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      const String &payload = http.getString();
      StaticJsonDocument<192> doc;

      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        client.stop();
        http.end();
        return false;
      }

      // Disconnect
      client.stop();
      // Extract values and store answer as a networkConfig structure
      strlcpy(network->ssid, doc["name"].as<const char *>(),
              sizeof(network->ssid));
      strlcpy(network->pass, doc["pass"].as<const char *>(),
              sizeof(network->pass));
      http.end();
      return saveConfig(network);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n",
                  http.errorToString(httpCode).c_str());
  }

  http.end();
  return false;
}
