#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <TaskScheduler.h>

#include "ESPAsyncWebServer.h"
#include "FS.h"

/***
 * Scans all the networks on the 2.4GHz band that are available
 * [generateHTML] optional boolean parameter that defaults to false.
 * If true, it returns the detected networks as a string of <option>
 * elements. If false, it returns the list of detected networks as a JSON object
 */
String scanAvailableNetworks(bool generateHTML = false) {
  int n = WiFi.scanComplete();
  if (n == -2) {
    WiFi.scanNetworks(true);
    Serial.println("No networks found");
    if (generateHTML) {
      return (String("<option value='none'>No networks found</option>"));
    } else {
      return (String("{\"names\":[]}"));
    }
  } else if (!n) {
    Serial.println("No networks found");
    if (generateHTML) {
      return (String("<option value='none'>No networks found</option>"));
    } else {
      return (String("{\"names\":[]}"));
    }
  } else if (n) {
    String options;
    if (generateHTML) {
      // Generate an HTML object
      options = "<option value='' selected></option>";
    } else {
      // Generate a JSON object
      options = "{\"names\":[";
    }
    for (int i = 0; i < n; ++i) {
      if (generateHTML) {
        options += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) +
                   "</option>";
      } else {
        if (i) {
          options += ",";
        }
      }
      options += "\"" + WiFi.SSID(i) + "\"";
    }
    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2) {
      WiFi.scanNetworks(true);
    }
    if (!generateHTML) {
      options += "]}";
    }
    return options;
  }
}

void configureAP(void) {
  char tmp[23];
  sprintf(tmp, "aq-monitor-main-%06x", ESP.getChipId());
  WiFi.softAP(tmp);
  // WiFi.softAPConfig(IPAddress(192, 168, 1, 100), IPAddress(192, 168, 1, 100),
  // IPAddress(255, 255, 255, 0));
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // First scan usually fails to detect anything
  scanAvailableNetworks();

  // Route to load ap.css file
  server.on("/ap.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/ap.css", "text/css");
  });

  // Route to load script.js file
  server.on("/ap.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/ap.js", "application/javascript");
  });

  // Serve images
  server.serveStatic("/img/", LittleFS, "/img/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/ap.html", String(), false, processor);
  });

  server.on("/networks", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", scanAvailableNetworks());
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
    loadConfig(&tempConfig);
    // Using c_str() instead of toCharArray() for copying the credentials to the
    // networkConfig structure reduces the code footprint
    if (request->hasParam("network", true)) {
      strlcpy(tempConfig.ssid,
              request->getParam("network", true)->value().c_str(),
              sizeof(tempConfig.ssid));
    } else {
      request->send(400, "text/plain", "Bad POST");
    }
    if (request->hasParam("password", true)) {
      strlcpy(tempConfig.pass,
              request->getParam("password", true)->value().c_str(),
              sizeof(tempConfig.pass));
    } else {
      strlcpy(tempConfig.pass, "", sizeof(tempConfig.pass));
    }
    attemptConnection = true;
    request->send(200, "text/plain",
                  "Credentials received. Attempting connection.");
  });
}

// This is an access point whose only purpose is simplifying the configuration
// of sensor nodes
void startMicroAP(void) {
  task_blinkLinking.enable();
  /**
   * 1. Serializar las credenciales y darles padding ajustandolo a una longitud
   * multiplo de 16
   * 2. Generar el set de llaves
   * 3. En Get, enviar llave publica
   * 4. Al recibir la llave del destinatario en Post, hacer dh2
   * 5. Cifrar credenciales y enviar en respuesta al Post
   */

  // generateKeyPair(keys.pub, keys.pk);
  // Not ideal to hardcode the password. Anyway, a WPA network with a strong
  // password is better that an open network for sendind the credentials
  WiFi.softAP("IoT-Main-AP", "BwpX56FMI$#!IgGq", 1, 1);
  // Timeout after one minute
  task_APOff.restartDelayed(1 * TASK_MINUTE);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Micro AP IP address: ");
  Serial.println(myIP);

  server.on("/netconfig", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->hasParam("hostname", true)) {
      char hostname[64];
      strlcpy(hostname, request->getParam("hostname", true)->value().c_str(),
              sizeof(hostname));
      String answer;
      Serial.println("Requesting network");
      networkConfig network;
      loadConfig(&network);
      StaticJsonDocument<200> doc;
      doc["name"] = network.ssid;
      doc["pass"] = network.pass;
      serializeJson(doc, answer);
      for (uint8_t i = 0; i < MAX_NODES_NUMBER; i++) {
        if (!strlen(nodeNames[i])) {
          strlcpy(nodeNames[i], hostname, sizeof(nodeNames[i]));
          saveNodes();
          request->send(200, "application/json", answer);
          task_APOff.restartDelayed(1 * TASK_SECOND);
          return;
        }
      }
      request->send(409, "text/plain", "Sensor quota reached");
      // Schedule AP for being turned off AP. Its job is completed
      task_APOff.restartDelayed(1 * TASK_SECOND);

      // if (answer.length() % 16) {
      //   for (uint8_t i = answer.length(); i < 16; i++) {
      //     // Add empty spaces to the end of the string to make its length a
      //     // multiple of 16
      //     answer.concat(" ");
      //   }
      // }
      // serializeJson(doc, &ans, sizeof(ans));
      // String answer((char*)keys.pub);
      // request->send(200, "text/plain", answer);
    } else {
      request->send(400, "text/plain", "Bad POST");
    }
  });
}
